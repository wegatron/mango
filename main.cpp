#include <iostream>

#include <volk.h>
#include <vulkan/vulkan.h>
#include <cassert>

#include "triangle_app.h"
#include "framework/window_app.h"

int main(int argc, char const *argv[])
{
    auto app = std::make_shared<vk_engine::TriangleApp>("triangle");
    auto window_app = std::make_shared<vk_engine::WindowApp>(
        "triangle", 800, 600);

    window_app->setApp(app);
    if(!window_app->init())
    {
        std::cerr << "Failed to init window app" << std::endl;
        return -1;
    }
    window_app->run();
    return 0;
}
