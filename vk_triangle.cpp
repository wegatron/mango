#include <iostream>

#include <volk.h>
#include <vulkan/vulkan.h>
#include <cassert>
#include "triangle_app.h"
#include "window_app.h"

int main(int argc, char const *argv[])
{
    VkResult ret = volkInitialize();
    assert(ret == VK_SUCCESS);

    auto app = std::make_shared<vk_engine::TriangleApp>("triangle");

    auto window_app = std::make_shared<vk_engine::WindowApp>(
        "triangle", 800, 600);

    window_app->setApp(app);
    window_app->run();
    return 0;
}
