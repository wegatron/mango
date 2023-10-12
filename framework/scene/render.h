#pragma once

#include <vector>
#include <framework/vk/syncs.h>

namespace vk_engine {

    class Render {
    public:
        Render(std::vector<RenderOutputSync> &render_output_syncs) : render_output_syncs_(render_output_syncs) {}
        
        void beginFrame(uint32_t frame_index, uint32_t rt_index);

        void render(Scene *scene);

        void endFrame();

    private:
        std::vector<RenderOutputSync> &render_output_syncs_;
        std::shared_ptr<CommandBuffer> cmd_buffer_;
    };
}