#pragma once
#include <vector>
#include <vulkan/vulkan.h>

namespace vk_engine {
class PhysicalDevice final {
public:
  static std::vector<PhysicalDevice> getPhysicalDevices(VkInstance instance);

  PhysicalDevice() = default;
  
  PhysicalDevice(const PhysicalDevice &) = delete;
  PhysicalDevice & operator=(const PhysicalDevice &) = delete;

  ~PhysicalDevice() = default;

  PhysicalDevice &operator=(const PhysicalDevice &) = delete;
  PhysicalDevice(const PhysicalDevice &) = delete;

  uint32_t getGraphicsQueueFamilyIndex() const {
    return graphics_queue_family_index_;
  }

  VkPhysicalDevice getHandle() const { return physical_device_; }

  VkPhysicalDeviceProperties getProperties() const { return properties_; }

  VkPhysicalDeviceFeatures getFeatures() const { return features_; }

  std::vector<VkExtensionProperties> getExtensionProperties() const {
    return extensions_;
  }

private:
  VkPhysicalDevice physical_device_{VK_NULL_HANDLE};
  VkPhysicalDeviceProperties properties_;
  VkPhysicalDeviceFeatures features_;
  std::vector<VkExtensionProperties> extensions_;
  uint32_t graphics_queue_family_index_{0xFFFFFFFF};
  std::vector<VkQueueFamilyProperties> queue_families_;
};
} // namespace vk_engine