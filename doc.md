# vk_engine

doc use mermaid. 
[comparation between graphviz and mermaid](https://www.devtoolsdaily.com/diagrams/graphviz_vs_mermaidjs/)
[class diagram](https://mermaid.js.org/syntax/classDiagram.html)

vulkan quick start: [vulkan-in-30-minutes](https://renderdoc.org/vulkan-in-30-minutes.html)

## framework Modules

framework is the main code of this mini engine. including these modules:

* Platfrom
    include system specific window-system and its input event's abstract interface.
* utils
    include some general reuseable functions and code. like error、log、string、compiler marcos, memory allocator. also include gui interface and window_app/app interface ...
* scene
    Definition of components in the scene, as well as scene loading, rendering and other functions.
* vk
    vulkan objects' c++ wrapper. For easy use and resource managment.


数据绑定规约:

The descriptor set number 0 will be used for engine-global resources, and bound once per frame. // camera

The descriptor set number 1 will be used for material resources, 
and the number 2 will be used for per-object resources. 

This way, the inner render loops will only be binding descriptor sets 1 and 2, and performance will be high.

### Initialization
#### Vulkan
通过VkConfig类来设置Vulkan的配置: 需要启用的特性(instance extension, device extension), 使用的设备类型(独显/集显). 通过设置的信息, 该类帮助创建所需的create info并选择满足条件的physical device. 高版本的Vulkan会将一些原来的扩展作为默认的特性, VkConfig被设计为一个虚类(接口).

Vulkan的基础知识:
Instance创建时可以指定需要添加layer(validation layer), 通过instance来query系统中的device以及device的特性. 基本的流程是: `vkCreateInstance()` → `vkEnumeratePhysicalDevices()` → `vkCreateDevice()`.

```mermaid
---
title: Vulkan Initialization
---
classDiagram
    class VkConfig {     
        void setFeatureEnabled(FeatureExtension feature, EnableState enable_state)
        
        void setDeviceType(VkPhysicalDeviceType device_type)
        
        EnableState isFeatureEnabled(FeatureExtension feature)

        virtual void checkAndUpdate(VkInstanceCreateInfo &create_info)

        virtual uint32_t checkSelectAndUpdate(const std::vector<PhysicalDevice> &physical_devices, VkDeviceCreateInfo &create_info, VkSurfaceKHR surface)
    }

    class Vk11Config

    VkConfig <|-- Vk11Config
```

#### App
* `Window` 是窗口的抽象基类(framework/platform/window.h). 由其负责窗口的创建、关闭、移动、缩放, imgUI的初始化接入, 接收用户的UI操作产生输入事件, 创建surface.
* `AppBase` 是应用程序的抽象基类(framework/utils/app_base.h). 实现了应用的真正逻辑, 包括初始化、每一帧的tick, 以及事件的处理.
* `WindowApp` 包含了`Window`和`AppBase`, 将他们组装成一个完整的功能类(framework/utils/window_app.h).

```mermaid
---
title: App
---
classDiagram
    class Window {
        virtual VkSurfaceKHR createSurface(VkInstance instance)

        virtual bool shouldClose()

        virtual void getExtent(uint32_t &width, uint32_t &height) const

        virtual void processEvents() //接收用户操作, 产生ui event

        virtual void initImgui() // 这里将ui事件与接入imgui, 这里可以全部转移到callback中(待优化)

        virtual void shutdownImgui()

        virtual void imguiNewFrame()

        virtual void setupCallback(AppBase * app) // ui event callback
    }
    
    class GlfwWindow

    class AppBase {
        AppBase(const std::string &name)
        virtual void tick(const float seconds, const uint32_t rt_index, const uint32_t frame_index)
        virtual void init(Window * window, const std::shared_ptr< VkDriver > &driver, const std::vector<std::shared_ptr<RenderTarget>> &rts)
        virtual void updateRts(const std::vector< std::shared_ptr< RenderTarget > > &rts)
        virtual void inputMouseEvent(const std::shared_ptr<MouseInputEvent> &mouse_event)
    }

    class ViewerApp

    class AppManager {
        bool init(VkFormat color_format, VkFormat ds_format)

        void setApp(std::shared_ptr<AppBase> &&app)
        
        void run()
    }

    Window <|-- GlfwWindow
    AppBase <|-- ViewerApp
    WindowApp --o AppBase : 1
    WindowApp --o Window : 1
```

### scene
#### Material
Material类定义了某种材质(类似于Blender中的Material node), 负责对外提供该材质的渲染能力(pipeline state设置+DescriptorSetLayout获取, 参数设置、纹理设置等).

在引擎中材质种类有限, 但使用同一种材质的物体会很多. 因此对材质相关GPU资源构建缓存进行复用: `MatGpuResourcePool`.

```mermaid
---
title: Material
---
classDiagram
    class MaterialUboParam {        
        uint32_t stride
        const std::type_info & tinfo
        const uint8_t ub_offset
        std::string name
    }

    class MaterialUboInfo {
        uint32_t set;
        uint32_t binding;
        uint32_t size;
        bool dirty
        std::vector< std::byte > data;
        std::vector< MaterialUboParam > params;
    }

    class MatParamsSet {
        [要求每个材质只用一个uniform buffer]
        size_t mat_hash_id;
        std::unique_ptr< Buffer > ubo;        
        std::shared_ptr< DescriptorSet > desc_set;
    }

    class MaterialTextureParam {
        uint32_t set;
        uint32_t binding;
        uint32_t index; // for array texture
        std::string name;
        std::shared_ptr< vk_engine::ImageView > img_view;
        bool dirty;       
    }

    class Material {
        MaterialUboInfo ubo_info_;
        std::vector< MaterialTextureParam > texture_params_;

        std::shared_ptr< MatParamsSet > mat_param_set_;
        std::unique_ptr< DescriptorSetLayout > desc_set_layout_;

        uint32_t hash_id_;
        
        void setUboParamValue(const std::string &name, const T value, uint32_t index)

        std::vector< MaterialTextureParam > & textureParams()

        void setPipelineState(PipelineState & pipeline_state)
    }

    class MatGpuResourcePool {
        std::shared_ptr< RenderPass > default_render_pass_;
        std::map< uint32_t, std::shared_ptr< GraphicsPipeline > > mat_pipelines_;
        std::unique_ptr< DescriptorPool > desc_pool_;
        std::list< std::shared_ptr< MatParamsSet > > used_mat_params_set_;
        std::list< std::shared_ptr< MatParamsSet > > free_mat_params_set_;      
    }

    MaterialUboParam --o MaterialUboInfo : n
    MaterialUboInfo --o Material : 1
    MatParamsSet --o Material : 1
    MaterialTextureParam --o Material : n

``` 

#### Asset Loader

### UI
