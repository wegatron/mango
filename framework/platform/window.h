#pragma once

#include <string>
#include <functional>
#include <volk.h>
#include <framework/platform/input_events.h>

namespace vk_engine {

class AppBase;

class Window {
public:
  Window(const std::string &window_title, const int width, const int height) {}

  virtual ~Window() = default;

  Window(const Window &) = delete;

  Window &operator=(const Window &) = delete;

  /**
   * @brief Gets a handle from the platform's Vulkan surface
   * @param instance A Vulkan instance
   * @returns A VkSurfaceKHR handle, for use by the application
   */
  virtual VkSurfaceKHR createSurface(VkInstance instance) = 0;

  virtual bool shouldClose() = 0;

  virtual void getExtent(uint32_t &width, uint32_t &height) const = 0;

  /**
   * \brief event process one loop, generate user input event to callback.
  */
  virtual void processEvents() = 0;

  virtual void initImgui() = 0;

  virtual void shutdownImgui() = 0;

  /**
   * \brief 
  */
  virtual void imguiNewFrame() = 0;

  // setup callback function
  virtual void setupCallback(AppBase * app) = 0;
};
} // namespace vk_engine