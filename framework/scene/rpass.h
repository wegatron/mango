#pragma once

#include <memory>
#include <list>
#include <Eigen/Dense>
#include <framework/scene/component/material.h>
#include <framework/vk/vk_driver.h>

namespace vk_engine {

class GraphicsPipeline;
class Material;
struct StaticMesh;
class CommandBuffer;
class Buffer;
class DescriptorPool;
class DescriptorSetLayout;

constexpr uint32_t MESH_UBO_SIZE=sizeof(float)*16;
constexpr uint32_t MAX_MESH_DESC_SET=90;
struct MeshParamsSet
{
    std::unique_ptr<Buffer> ubo;
    std::shared_ptr<DescriptorSet> desc_set;
    mutable uint32_t last_access{0};
};

class MeshParamsPool final
{
public:
    MeshParamsPool();
    ~MeshParamsPool() { reset(); }
    void gc();
    void reset();
    MeshParamsSet * requestMeshParamsSet();
private:
    std::unique_ptr<DescriptorSetLayout> desc_layout_;
    std::unique_ptr<DescriptorPool> desc_pool_;
    std::list<MeshParamsSet*> free_mesh_params_set_;
    std::list<MeshParamsSet*> used_mesh_params_set_;
    uint32_t cur_frame_{0};
};

class RPass {
public:
    RPass(VkFormat color_format, VkFormat ds_format) : mat_gpu_res_pool_(color_format, ds_format) {}
    virtual ~RPass() = default;
    void gc() { mesh_params_pool_.gc(); mat_gpu_res_pool_.gc(); }
    void draw(const std::shared_ptr<Material> &mat, const Eigen::Matrix4f &rt, 
        const std::shared_ptr<StaticMesh> &mesh, const std::shared_ptr<CommandBuffer> &cmd_buf);
private:    
    MeshParamsPool mesh_params_pool_;
    MatGpuResourcePool mat_gpu_res_pool_;
};
} // namespace vk_engine
