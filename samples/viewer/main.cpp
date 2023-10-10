#include <framework/utils/logging.h>
#include <iostream>

#include <cassert>
#include <volk.h>
#include <vulkan/vulkan.h>

#include <framework/utils/window_app.h>
#include "viewer_app.h"

int main(int argc, char const *argv[])
{
  #if !NDEBUG
  spdlog::set_level(spdlog::level::debug);
  #endif
  
  auto app = std::make_shared<vk_engine::ViewerApp>("viewer");
  //app->setScene("data/buster_drone/scene.gltf");
  app->setScene("data/plane/plane0.obj");

  auto window_app =
      std::make_shared<vk_engine::WindowApp>("viewer", 800, 600);
  window_app->setApp(std::move(app));
  if (!window_app->init()) {
    LOGE("Failed to init window app");
    return -1;
  }
  window_app->run();
  return 0;
}