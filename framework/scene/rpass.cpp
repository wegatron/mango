#include <framework/scene/rpass.h>
#include <framework/vk/buffer.h>
#include <framework/scene/component/material.h>

namespace vk_engine
{

    void RPass::draw(const std::shared_ptr<Material> &mat, const Eigen::Matrix4f &rt,
        const std::shared_ptr<StaticMesh> &mesh, const std::shared_ptr<CommandBuffer> &cmd_buf)
    {        
        // bind material set 2
        auto desc_set = mat->getDescriptorSet();
        
        // bind ubo set3
        auto ubo = buffer_pool_.requestMeshUbo();

        // bind mesh vertices indices
        // draw
        // memory barrier
    }
}