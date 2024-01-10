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

#### Light

基于物理的度量, 灯光的亮度度量 $\to$ 反射亮度度量 $\to$ 曝光 $\to$ 图片.

refer to filament: https://google.github.io/filament/Filament.md.html#lighting/directlighting

| Photometric term | Notation | Unit |


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
