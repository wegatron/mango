#pragma once

#include <string>
#include <vector>
#include <vk_mem_alloc.h>
#include <volk.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <framework/vk/vk_config.h>

namespace vk_engine {

struct RequestedDeviceExtension {
  const char *name;
  bool required;
};
  
class VkDriver {
public:
  VkDriver() = default;
  ~VkDriver();

  VkDriver(const VkDriver &) = delete;
  VkDriver &operator=(const VkDriver &) = delete;
  void init(const std::string &app_name, const std::shared_ptr<VkConfig> &config,
            GLFWwindow *window);

  VkPhysicalDevice getPhysicalDevice() const { return physical_device_; }
  
  VkDevice getDevice() const { return device_; }

  VmaAllocator getAllocator() const { return allocator_; }

  VkSurfaceKHR getSurface() const { return surface_; }

  uint32_t getGraphicsQueueFamilyIndex() const { return graphics_queue_family_index_; }

  VkQueue getGraphicsQueue() const { return graphics_queue_; }

  VkResult waitIdle() const { return vkDeviceWaitIdle(device_); }

private:
  void initInstance();

  void initDevice();

  void initAllocator();

  std::pair<bool, uint32_t>
  selectPhysicalDevice(const std::vector<RequestedDeviceExtension> &request_extensions);

  void checkSwapchainAbility();

  void initSwapchain(GLFWwindow *window);

  void setupDebugMessenger();

  bool isDeviceExtensionEnabled(const char *extension_name);

  std::shared_ptr<VkConfig> config_;
  VkInstance instance_{VK_NULL_HANDLE};
  VkPhysicalDevice physical_device_{VK_NULL_HANDLE};
  VkDevice device_{VK_NULL_HANDLE};

  std::vector<const char *> enabled_device_extensions_;

  VkQueue graphics_queue_{VK_NULL_HANDLE};
  uint32_t graphics_queue_family_index_{0};

  VkSurfaceKHR surface_{VK_NULL_HANDLE};

  VmaAllocator allocator_{VK_NULL_HANDLE};

  VkDebugUtilsMessengerEXT debug_messenger_;
};
} // namespace vk_engine