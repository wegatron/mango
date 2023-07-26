#include <framework/utils/logging.h>
#include <iostream>

#include <cassert>
#include <volk.h>
#include <vulkan/vulkan.h>

#include <framework/utils/window_app.h>
#include "triangle_app.h"

int main(int argc, char const *argv[]) {
  #if !NDEBUG
  spdlog::set_level(spdlog::level::debug);
  #endif
  auto app = std::make_shared<vk_engine::TriangleApp>("triangle", nullptr);
  auto window_app =
      std::make_shared<vk_engine::WindowApp>("triangle", 800, 600);

  window_app->setApp(std::move(app));
  if (!window_app->init()) {
    LOGE("Failed to init window app");
    return -1;
  }
  window_app->run();
  return 0;
}
