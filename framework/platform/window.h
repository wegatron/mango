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
   * @param physical_device A Vulkan PhysicalDevice
   * @returns A VkSurfaceKHR handle, for use by the application
   */
  virtual VkSurfaceKHR create_surface(VkInstance instance,
                                      VkPhysicalDevice physical_device) = 0;

  virtual void getExtent(uint32_t &width, uint32_t &height) const = 0;

  virtual void processEvents() = 0;

  // start event process
  // setup callback function
  virtual void init(AppBase * app) = 0;

protected:
  std::function<void (const InputEvent &, AppBase * app)> input_event_callback_;
};
} // namespace vk_engine