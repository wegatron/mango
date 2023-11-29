#include <framework/utils/logging.h>
#include <iostream>

#include <cassert>
#include <volk.h>
#include <vulkan/vulkan.h>

#include <framework/utils/window_app.h>
#include "viewer_app.h"

int main(int argc, char const *argv[])
{
  if(argc < 2) {
    LOGE("Usage: {} <scene_path>", argv[0]);
    return -1;
  }
  
  #if !NDEBUG
  spdlog::set_level(spdlog::level::debug);
  #endif
  
  VkFormat color_format = VK_FORMAT_B8G8R8A8_SRGB;
  VkFormat depth_stencil_format = VK_FORMAT_D24_UNORM_S8_UINT;
  auto app = std::make_shared<vk_engine::ViewerApp>("viewer");

  //app->setScene("data/buster_drone/scene.gltf");  
  app->setScene(argv[1]);

  auto window_app =
      std::make_shared<vk_engine::WindowApp>("viewer", 800, 600);
  window_app->setApp(std::move(app));
  if (!window_app->init(color_format, depth_stencil_format)) {
    LOGE("Failed to init window app");
    return -1;
  }
  window_app->run();
  return 0;
}