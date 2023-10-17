#pragma once

#include <memory>
#include <list>
#include <Eigen/Dense>

namespace vk_engine {

class GraphicsPipeline;
class Material;
struct StaticMesh;
class CommandBuffer;

struct UboItem
{
    std::shared_ptr<Buffer> buffer_;
    mutable uint64_t lastAccessed;
};

class TempoaryBufferPool
{
public:
    std::shared_ptr<Buffer> requestMeshUbo();
    void gc();
private:
    std::list<UboItem*> free_mesh_ubos_;
    std::list<UboItem*> used_mesh_ubos_;
    uint32_t frame_cnt_{0};
};

class RPass {
public:
    virtual ~RPass() = default;
    void draw(const std::shared_ptr<Material> &mat, const Eigen::Matrix4f &rt, 
        const std::shared_ptr<StaticMesh> &mesh, const std::shared_ptr<CommandBuffer> &cmd_buf);
private:
    // pipeline cache?
    std::shared_ptr<GraphicsPipeline> pipeline_;
    TempoaryBufferPool buffer_pool_;
};
} // namespace vk_engine
