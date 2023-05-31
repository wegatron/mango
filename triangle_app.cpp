#include "triangle_app.h"
#include "framework/vk_driver.h"
#include "framework/shader_module.h"
#include <memory>

namespace vk_engine
{
    void TriangleApp::tick(const float seconds)
    {
        // TODO render one frame
    }

    void TriangleApp::init(const std::shared_ptr<VkDriver> &driver)
    {
        /// prepare data
        // load and compile shader
        ShaderSource shader_source;
        shader_source.load("shaders/triangle.vert");
                
        // create pipeline layout

        // create pipeline

        // render data
        // vertex data
        std::vector<float> vertex_data = {
            -0.5, -0.5, 0, 1, 0, 0, 1, 0, 0,
             0.5, -0.5, 0, 1, 0, 0, 1, 1, 0,
             0, 0.5, 0, 1, 0, 0, 1, 0.5, 0.5
        };
        
    }
}