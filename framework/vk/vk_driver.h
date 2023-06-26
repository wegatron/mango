#pragma once

#include <string>
#include <vector>
#include <vk_mem_alloc.h>
#include <volk.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vk_engine {
class VkDriver {
public:
  VkDriver() = default;
  ~VkDriver();

  VkDriver(const VkDriver &) = delete;
  VkDriver &operator=(const VkDriver &) = delete;
  void init(const std::string &app_name, const bool enable_validation,
            GLFWwindow *window);

  VkDevice getDevice() const { return device_; }

  VmaAllocator getAllocator() const { return allocator_; }

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

  VkInstance instance_{VK_NULL_HANDLE};
  VkPhysicalDevice physical_device_{VK_NULL_HANDLE};
  VkDevice device_{VK_NULL_HANDLE};

  std::vector<const char *> enabled_device_extensions_;

  VkQueue graphics_queue_{VK_NULL_HANDLE};
  VkSurfaceKHR surface_{VK_NULL_HANDLE};

  VkSwapchainKHR swapchain_{VK_NULL_HANDLE};
  std::vector<VkImage> swapchain_images_;
  std::vector<VkImageView> swapchain_image_views_;
  VkExtent2D swapchain_extent_;
  VkFormat swapchain_image_format_;

  VmaAllocator allocator_{VK_NULL_HANDLE};

  bool enable_vk_validation_{true};

  VkDebugUtilsMessengerEXT debug_messenger_;
};
} // namespace vk_engine