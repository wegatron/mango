#pragma once

#include <vector>
#include <framework/vk/syncs.h>

namespace vk_engine {

    class Render {
    public:
        Render(std::vector<RenderOutputSync> &render_output_syncs) : render_output_syncs_(render_output_syncs) {}
        
        void beginFrame();

        void render(Scene *scene);

        void endFrame();

    private:
        std::vector<RenderOutputSync> &render_output_syncs_;
    };
}