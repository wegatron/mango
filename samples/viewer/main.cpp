#include <framework/utils/logging.h>
#include <iostream>

#include <cassert>
#include <volk.h>
#include <vulkan/vulkan.h>

#include <framework/utils/window_app.h>
#include <framework/platform/glfw_window.h>
#include <framework/vk/vk_config.h>

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
  app->setScene(argv[1]);
   
  
  auto config = std::make_shared<vk_engine::Vk11Config>();
  config->setDeviceType(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
  config->setFeatureEnabled(vk_engine::VkConfig::FeatureExtension::GLFW_EXTENSION,
                            vk_engine::VkConfig::EnableState::REQUIRED);  
  config->setFeatureEnabled(vk_engine::VkConfig::FeatureExtension::KHR_SWAPCHAIN,
                            vk_engine::VkConfig::EnableState::REQUIRED);
  config->setFeatureEnabled(vk_engine::VkConfig::FeatureExtension::INSTANCE_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2,
                            vk_engine::VkConfig::EnableState::REQUIRED);  
  auto window = std::make_unique<vk_engine::GlfwWindow>("viewer", 800, 600);
  auto window_app =
      std::make_shared<vk_engine::WindowApp>(std::move(window));
  window_app->setApp(std::move(app));
  if (!window_app->init(config, color_format, depth_stencil_format)) {
    LOGE("Failed to init window app");
    return -1;
  }
  window_app->run();
  return 0;
}