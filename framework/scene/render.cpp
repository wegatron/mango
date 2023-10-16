#include <framework/scene/render.h>
#include <framework/scene/rpass.h>
#include <framework/scene/scene.h>
#include <framework/utils/app_context.h>
#include <framework/vk/commands.h>
#include <framework/vk/queue.h>
#include <cassert>

namespace vk_engine
{   
    void Render::beginFrame(const float time_elapse, const uint32_t frame_index, const uint32_t rt_index)
    {
        // wait sync
        assert(!render_output_syncs_.empty());
        cur_frame_index_ = frame_index;
        cur_rt_index_ = rt_index;
        cur_time_ = time_elapse; 
        render_output_syncs_[frame_index].render_fence->wait();
        render_output_syncs_[frame_index].render_fence->reset();
        auto &cmd_pool = getDefaultAppContext().frames_data[cur_frame_index_].command_pool;
        cmd_pool->reset();
        auto cmd_buf_ = cmd_pool->requestCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);        
    }

    void Render::render(Scene *scene)
    {
        assert(scene != nullptr);
        scene->update(cur_time_, cmd_buf_);
        auto &render_tgt = getDefaultAppContext().frames_data[cur_rt_index_].render_tgt;
        
        auto & rm = scene->renderableManager();
        auto view = rm.view<std::shared_ptr<TransformRelationship>, std::shared_ptr<Material>, std::shared_ptr<StaticMesh>>();
        RPass rp;
        view.each([&rp, this](const std::shared_ptr<TransformRelationship> &tr, const std::shared_ptr<Material> &mat, const std::shared_ptr<StaticMesh> &mesh)
        {
            // update rt
            rp.draw(mat, tr->transform, mesh, cmd_buf_);
        });
    }

    void Render::endFrame()
    {
        auto cmd_queue = getDefaultAppContext().driver->getGraphicsQueue();
        cmd_queue->submit(cmd_buf_, render_output_syncs_[cur_frame_index_].render_fence->getHandle());
    }
}