# vk_engine

doc use mermaid. 
[comparation between graphviz and mermaid](https://www.devtoolsdaily.com/diagrams/graphviz_vs_mermaidjs/)
[class diagram](https://mermaid.js.org/syntax/classDiagram.html)

vulkan quick start: [vulkan-in-30-minutes](https://renderdoc.org/vulkan-in-30-minutes.html)

## framework Modules
| 模块 |  描述 |
| --- | --- |
| framework/platform | 与系统/平台相关的类: 窗口、UI事件的抽象以及实现等|
| framework/scene | 场景相关的数据类和操作类, 包括场景组件(相机、材质、mesh), 场景的加载、渲染. |
| framework/utils | 一些通用的功能函数、类, 例如: log、copiler、marcos、memory allocator、imageUI based UI element render和用来构建应用的AppManager. |
| framework/vk | vulkan 的功能封装, 给上层提供更简单的接口, 让上层以对象的方式管理vulkan资源, 并提供了一些功能类(例如Resource cache、stagepool), 以提升系统性能. |

数据绑定规约:

| descriptor set |  用途 |
| --- | --- |
| GLOBAL_SET_INDEX = 0 | 用来存一些全局的属性/参数(对场景中的所有物体有效的参数), 例如: 相机的参数. |
| MATERIAL_SET_INDEX = 1 | 用来存可能对一个或多个物体的属性/参数, 例如: 材质. |
| OBJECT_SET_INDEX = 2 | 用来存只对一个物体有效的参数. 例如: 物体的Rt. |

### Vulkan
#### Initialization
通过VkConfig类来设置Vulkan的配置, 高版本的Vulkan会将一些原来的扩展作为默认的特性, VkConfig被设计为一个虚类(接口).

| 函数 | 描述 |
| --- | --- |
| setFeatureEnabled | 设置启用的特性: instance extension, device extension |
| setDeviceType | 设置设备类型(独显/集显) |
| checkAndUpdate |  根据设置, 更新/设置VkInstanceCreateInfo |
| checkSelectAndUpdate | 根据设置+surface(可以是null), 选择合适的硬件设备, 并更新/设置VkDeviceCreateInfo |

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

初始化流程图(VkDriver::init):
```mermaid
flowchart
    id1(set VkConfig)
    id2(vkCreateInstance)
    id3{window != null}
    id4(create surface)
    id5(select physical device
        create vkDevice
        create CommandQueue)
    id1 --> id2
    id2 --> id3
    id3 --> |Yes| id4
    id3 --> |No| id5
    id4 --> id5
    id6(create swapchain)
    id7(create render target)
    id6 --> id7
    id5 --> id6
```

#### ShaderModule

利用glslang将shader+shader_variant编译成spirv, 利用spirv-cross解析shader中用到的资源, 以被构建DescriptorSetLayout --> PipelineLayout.

```mermaid
classDiagram
    class ShaderResource {
        VkShaderStageFlags stages

        ShaderResourceType type

        ShaderResourceMode mode

        uint32_t set

        uint32_t binding

        uint32_t location

        uint32_t input_attachment_index

        uint32_t vec_size

        uint32_t columns

        uint32_t array_size

        uint32_t offset

        uint32_t size

        uint32_t constant_id

        uint32_t qualifiers

        std::string name

        static size_t hash(const ShaderResource &resource) noexcept
    }
    class ShaderModule {
        ShaderModule(const ShaderVariant &variant)
        void load(const std::string &file_path)
        void setGlsl(const std::string &glsl_code, VkShaderStageFlagBits stage)
        const std::string &getGlsl() const noexcept
        const std::vector< uint32_ > &getSpirv() const noexcept
        size_t getHash() const noexcept
        VkShaderStageFlagBits getStage()
        const std::vector< ShaderResource > &getResources() const noexcept
        static size_t hash(const std::string &glsl_code VkShaderStageFlagBits stage) noexcept
        static void compile2spirv(const std::string &glsl_code, const std::string &preamble,  VkShaderStageFlagBits stage, std::vector< uint32_t > &spirv_code)
        static void readGlsl(const std::string &file_path, VkShaderStageFlagBits &stage, std::string &glsl_code)

        size_t hash_code_
        VkShaderStageFlagBits stage_
        std::shared_ptr< VkDriver > driver_
        std::string glsl_code_
        std::vector< uint32_t > spirv_code_
        std::vector< ShaderResource > resources_
        ShaderVariant variant_
    }
    class Shader {
        Shader(const std::shared_ptr<VkDriver> &driver, const std::shared_ptr< ShaderModule > &shader_module)
        VkShaderModule getHandle() const noexcept
        std::shared_ptr< VkDriver > driver_
        VkShaderModule handle_       
    }
    class ShaderVariant {
        ShaderVariant()
        ShaderVariant(std::string &&preamble)
        void addDefinitions(const std::vector< std::string > &definitions)
        void addDefine(const std::string &def)
        void addUndefine(const std::string &undef)
        const std::string &getPreamble() const
        std::string preamble_
    }

    ShaderVariant --o ShaderModule : 1
    ShaderResource --o ShaderModule : n
    Shader --> ShaderModule
```

#### Pipeline
| 类 | 描述 |
| --- | --- |
| Pipeline | Pipeline接口, 通过其获取vkPipeline 和 PipelineLayout. |
| GPipelineState | 封装了Graphics PipelineState的数据属性. |
| GraphicsPipeline | 接受GPipelineState, 创建vulkan graphics pipeline. |

```mermaid
classDiagram
    class GPipelineState {
        VertexInputState vertex_input_state_
        InputAssemblyState input_assembly_state_
        RasterizationState rasterization_state_
        ViewPortState viewport_state_
        MultisampleState multisample_state_
        DepthStencilState depth_stencil_state_
        ColorBlendState color_blend_state_
        uint32_t subpass_index_
        std::vector< std::shared_ptr< ShaderModule >> shader_modules_
        std::vector< VkDynamicState > dynamic_states_

        void setShaders(const std::vector< std::shared_ptr< ShaderModule >> &shader_modules)
        void setVertexInputState(const VertexInputState &state)
        void setInputAssemblyState(const InputAssemblyState &state)
        void setRasterizationState(const RasterizationState &state)
        void setViewportState(const ViewPortState &state)
        void setMultisampleState(const MultisampleState &state)
        void setDepthStencilState(const DepthStencilState &state)
        void setColorBlendState(const ColorBlendState &state)
        void setSubpassIndex(uint32_t subpass_index)
        const std::vector< std::shared_ptr< ShaderModule >> &getShaderModules()
        const XXX & getXXX()
        const uint32_t getSubpassIndex()
    }
    class Pipeline {
        VkPipeline getHandle() const
        std::shared_ptr< PipelineLayout > getPipelineLayout() const
        Type getType() const
        Type type_
        VkPipeline pipeline_
        std::shared_ptr< VkDriver > driver_
        std::shared_ptr< PipelineLayout > pipeline_layout_      
    }
    class GraphicsPipeline {
        GraphicsPipeline(const std::shared_ptr<VkDriver> &driver, const std::shared_ptr<ResourceCache> &cache, const  std::shared_ptr<RenderPass> &render_pass, std::unique_ptr<PipelineState> &&pipeline_state)
        GPipelineState & getPipelineState()
        std::unique_ptr< GPipelineState > pipeline_state_   
    }
    Pipeline <|-- GraphicsPipeline
    GPipelineState --o GraphicsPipeline : 1
```

![](pipeline_state.png)

![](pipeline_overview.png)

#### RenderPass & Framebuffer

| 类 | 描述 |
| --- | --- |
| RenderPass (framework\vk\render_pass.h) | 封装了vkRenderPass, 表示了这个渲染Pass中的输出设置: attachment的load/store, subpass以及subpass之间的依赖关系, ttachment与subpass之间的联系. |
| RenderTarget (framework\vk\frame_buffer.h)| 封装了渲染的输出目标(color output imageview + depth output imageview) |
| FrameBuffer (framework\vk\frame_buffer.h)| 封装了VkFrameBuffer, 是RenderPass和RenderTarget的组合. |

在GraphicsPipeline创建时就需要设置VkRenderPass, 但后续在真正渲染时, 只要渲染输出的FrameBuffer与GraphicsPipeline中的VkRenderPass兼容即可.
兼容性要求:
* attachment 数量相同(允许有VK_ATTACHMENT_UNUSED)
* 对应attachment的format、sample count一致
* 允许attachment有不同的extend, load/store, layout

![](diff_vkframebuffer_vkrenderpass.png)

#### DescriptorSet & DescriptorPool
* 创建
    应该是为了更好地配合多线程, 提高性能, 在Vulkan中DescriptorPool像是一个内存池, 需要先申请一个大的pool(定义好能从中申请的DescriptorSet的规格和容量). 后续DescriptorSet从pool中进行申请.
    在很多引擎中是直接申请一个最大的DescriptorPool, 在这里, 我针对材质设置了一个定制的DescriptorPool.
    ![](descriptor_set.png)

* 更新和复用
    参考[writing-an-efficient-vulkan-renderer](https://zeux.io/2020/02/27/writing-an-efficient-vulkan-renderer/), 为了更好的性能, 避免不必要的DescriptorSet创建开销. 对OBJECT_SET_INDEX(MeshParamsPool), MATERIAL_SET_INDEX(MatGpuResourcePool), GLOBAL_SET_INDEX(AppContext.descriptor_pool)分别设置了不同规格的DescriptorSetPool, 并对使用完的DescriptorSet进行缓存复用. 

#### CommandPool & CommandBuffer
与DescriptorSet类似, CommandBuffer同Pool中去申请以提升性能. 在这里每一个FrameBuffer绑定一个CommandPool, 在每帧开始时将CommandPool进行Reset(所有从中创建的CommandBuffer返回到initial state), 然后取一个已经创建或新建(若不足)一个CommandBuffer进行命令录制.
![](cmd_buf_state.png)

#### Synchronize

Vulkan中的三种同步对象:
![](sync_objs.png)

使用场景:
* 渲染和呈现循环
    ![](render_present_sync.png)
* RenderPass之间, 使用Barrier进行同步. 例如: 加载图片纹理后使用ImageBarrier将ImageLayout转换为OPTIMAL_LAYOUT. 在这里普通的场景渲染之后加了一个ImageBarrier, 然后再进行Imgui的渲染.

#### 功能类/函数

| 类 | 描述 |
| --- | --- |
| ResourceCache (framework/vk/resource_cache.h) | 参考Vulkan-Samples, 将一些可以服用的Vulkan 资源进行缓存复用, 例如: shader, DescriptorSetLayout, PipelineLayout, Sampler |
| StagePool (framework/vk/stage_pool.h) | 参考Filament, 一个GPU-CPU均能访问的buffer/image cache, 用来上传/下载数据. |

#### 渲染过程
![](render_process.png)

### App
| 类 |  描述 |
| --- | --- |
| Window (framework/platform/window.h) | 窗口的抽象基类. 由其负责窗口的创建、关闭、移动、缩放, imgUI的初始化接入, 接收用户的UI操作产生输入事件, 创建surface. |
| AppBase (framework/utils/app_base.h) | 应用程序的抽象基类. 实现了应用的真正逻辑, 包括初始化、每一帧的tick, 以及事件的处理. |
| AppManager (framework/utils/window_app.h) | 将Window和AppBase组装成一个完整的功能类 |
| AppContext (framework/utils/app_context.h) | 全局单例, 保存了整个系统的一些全局对象: driver, descriptor_pool(GLOBAL_SET_INDEX), stage_pool ...|

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
    
    class AppContext {
        std::shared_ptr< VkDriver > driver
        std::shared_ptr< DescriptorPool > descriptor_pool
        std::shared_ptr< StagePool > stage_pool
        std::shared_ptr< GPUAssetManager > gpu_asset_manager
        std::shared_ptr< ResourceCache > resource_cache
        std::vector< FrameData> frames_data
        std::unique_ptr< GlobalParamSet > global_param_set
        std::vector< RenderOutputSync > render_output_syncs        
    }

    Window <|-- GlfwWindow
    AppBase <|-- ViewerApp
    AppManager --o AppBase : 1
    AppManager --o Window : 1
    
```

### scene
这里利用entt, 以ECS的架构来组建场景. 这里使用TransformRelationship, 来表示entity的父子关系(这里处于性能的考虑, 每个节点设置三个指针parent, child, sibling).

```mermaid
---
title: Scene(ECS)
---
classDiagram
    class Material
    class Mesh {
        VertexBuffer vertices
        VertexBuffer normals
        VertexBuffer texture_coords
        IndexBuffer faces
        Eigen::AlignedBox3f aabb      
    }
    class TransformRelationship{
        std::shared_ptr< TransformRelationship > parent
        std::shared_ptr< TransformRelationship > child
        std::shared_ptr< TransformRelationship > sibling
        Eigen::Matrix4f ltransform
        Eigen::Matrix4f gtransform
        Eigen::AlignedBox3f aabb       
    }
    class Camera {
        void setLookAt(const Eigen::Vector3f &eye, const Eigen::Vector3f &up, const Eigen::Vector3f &center)
        void setRotationEuraXYZ(const Eigen::Vector3f &eura_xyz)
        void setRotationQuat(const Eigen::Quaternionf &quat)
        void setRotation(const Eigen::Matrix3f &r)

        const Eigen::Matrix4f &getViewMatrix()
        const Eigen::Matrix4f &getProjMatrix()
        std::string name_
        bool dirty_proj_
        float near_
        float far_
        float fovy_
        float aspect_
        float dis_
        Eigen::Matrix4f proj_mat_;
        Eigen::Matrix4f view_mat_        
    }
    class Light
    class Entity    
    Material --o Entity
    Mesh --o Entity
    TransformRelationship --o Entity
    Camera --o Entity
    Light --o Entity
```

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

| 类 | 描述 |
| --- | --- |
| GPUAssetManager (framework/scene/asset_manager.hpp) | 用来加载数据/资产到GPU, 例如image |
| AssimpLoader (framework/scene/loader.h) | 用来加载3d场景 |

数据加载的过程:

```mermaid
flowchart LR
	id1(Assimp::ReadFile)
	id2(processMesh)
	id3(processMaterial)
	id4(processCamera)
	id5(processNode)
	id1 --> id2 --> id3 --> id4 --> id5
```

| 过程 | 描述 |
| --- | --- |
| Assimp::ReadFile | 使用assimp读取3d场景文件 |
| processMesh | 获取assimp中读取到的所有mesh, 转换成Mesh Component数组 |
| processMaterial | 获取assimp中读取到的所有material, 转换成Material Component数组 |
| processCamera | 获取assimp中读取到的所有camera, 转换成camera component |
| processNode | 根据assimp中的node信息, 创建Renderable entity, 构建场景树. |




#### Tick One Frame

## Reference
https://zeux.io/2020/02/27/writing-an-efficient-vulkan-renderer/