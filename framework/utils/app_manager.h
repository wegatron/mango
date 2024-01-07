#pragma once

#include <memory>
#include <string>

#include <GLFW/glfw3.h>

#include <framework/utils/app_base.h>
#include <framework/platform/window.h>
#include <framework/vk/vk_driver.h>
#include <framework/vk/swapchain.h>
#include <framework/vk/frame_buffer.h>

namespace vk_engine {
class AppManager final {
public:
  AppManager(std::unique_ptr<Window> &&window);

  ~AppManager();

  AppManager(const AppManager &) = delete;
  AppManager &operator=(const AppManager &) = delete;

  bool init(const std::shared_ptr<VkConfig> &config, VkFormat color_format, VkFormat ds_format);

  void setApp(std::shared_ptr<AppBase> &&app);
  
  void run();

private:

  void resize(uint32_t width, uint32_t height);
  
  void initSwapchain();

  void initRenderTargets();
  
  std::shared_ptr<VkDriver> driver_;
  std::shared_ptr<Swapchain> swapchain_;
  std::vector<std::shared_ptr<RenderTarget>> render_targets_;

  VkFormat color_format_;
  VkFormat ds_format_;  
  std::vector<std::shared_ptr<Image>> depth_images_;

  std::shared_ptr<AppBase> app_;
  std::unique_ptr<Window> window_;

  uint32_t width_;
  uint32_t height_;
  std::string window_title_;
  uint32_t current_frame_index_{0};
};
} // namespace vk_engine