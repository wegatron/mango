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

constexpr uint32_t MESH_UBO_SIZE=sizeof(float)*16;
struct MeshParamsSet
{
    std::unique_ptr<Buffer> ubo;
    std::shared_ptr<DescriptorSet> desc_set;
    mutable uint64_t lastAccessed;
};

class MeshParamsPool final
{
public:
    MeshParamsPool(const std::shared_ptr<VkDriver> &driver) : driver_(driver) {}
    ~MeshParamsPool();
    MeshParamsSet * requestMeshParamsSet();
    void gc();
private:
    std::shared_ptr<VkDriver> driver_;
    std::list<MeshParamsSet*> free_mesh_params_set_;
    std::list<MeshParamsSet*> used_mesh_params_set_;
    uint32_t frame_cnt_{0};
};

class RPass {
public:
    void gc() { mesh_params_pool_.gc(); }
    virtual ~RPass() = default;
    void draw(const std::shared_ptr<Material> &mat, const Eigen::Matrix4f &rt, 
        const std::shared_ptr<StaticMesh> &mesh, const std::shared_ptr<CommandBuffer> &cmd_buf);
private:
    MatPipelinePool mat_pipeline_pool_;
    MeshParamsPool mesh_params_pool_;
};
} // namespace vk_engine
