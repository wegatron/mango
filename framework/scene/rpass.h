#pragma once

#include <memory>

namespace vk_engine {

class GraphicsPipeline;
class Material;
class StaticMesh;
class CommandBuffer;

class RPass {
public:
    virtual ~RPass() = default;
    void draw(const std::shared_ptr<Material> &mat, const Eigen::Matrix4f rt, const std::shared_ptr<StaticMesh> &mesh, const std::shared_ptr<CommandBuffer> &cmd_buffer);
private:
    // pipeline cache?
    std::shared_ptr<GraphicsPipeline> pipeline_;
};

} // namespace vk_engine
