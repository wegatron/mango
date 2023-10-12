#include <framework/scene/render.h>
#include <framework/scene/rpass.h>
#include <framework/scene/scene.h>
#include <utils/app_context.h>
#include <cassert>

namespace vk_engine
{   
    void Render::beginFrame(uint32_t frame_index, uint32_t rt_index)
    {
        // wait sync
        assert(!render_output_syncs_.empty());
        render_output_syncs_[frame_index].render_fence->wait();
        render_output_syncs_[frame_index].render_fence->reset();
        auto &ctx = getDefaultAppContext();
        ctx.frames_data[frame_index].command_pool->reset();
        //        
    }

    void Render::render(Scene *scene)
    {
        assert(scene != nullptr);
        auto & rm = scene->renderableManager();
        auto view = rm.view<std::shared_ptr<TransformRelationship>, std::shared_ptr<Material>, std::shared_ptr<StaticMesh>>();
        RPass rp;
        view.each([&rp, this](const std::shared_ptr<TransformRelationship> &tr, const std::shared_ptr<Material> &mat, const std::shared_ptr<StaticMesh> &mesh)
        {
            // update rt
            rp.draw(mat, tr->transform, mesh, cmd_buffer_);
        });
    }

    void Render::endFrame()
    {
        
    }
}