#pragma once

#include <vector>
#include <framework/vk/syncs.h>
#include <framework/scene/rpass.h>

namespace vk_engine {
    class CommandBuffer;
    class Scene;
    class Render {
    public:
        Render(std::vector<RenderOutputSync> &render_output_syncs, VkFormat color_format, VkFormat ds_format) 
            : render_output_syncs_(render_output_syncs), rpass_(color_format, ds_format) {}
        
        void beginFrame(const float time_elapse, const uint32_t frame_index, const uint32_t rt_index);

        void render(Scene *scene);

        void endFrame();

    private:
        uint32_t cur_frame_index_{0};
        uint32_t cur_rt_index_{0};
        float cur_time_{0.0};
        std::vector<RenderOutputSync> render_output_syncs_;
        std::shared_ptr<CommandBuffer> cmd_buf_;
        RPass rpass_;
    };
}