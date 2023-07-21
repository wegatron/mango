#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <memory>
#include <volk.h>

#include <framework/vk/physical_device.h>

namespace vk_engine {
class VkConfig {
public:
  enum class EnableState : uint8_t { DISABLED = 0, OPTIONAL, REQUIRED };

  enum class FeatureExtension : uint32_t {
    //// instance layers
    LAYER_BEGIN_PIVOT = 0,
    KHR_VALIDATION_LAYER = 1,
    LAYER_END_PIVOT = 2,

    //// instance extension
    INSTANCE_EXTENSION_BEGIN_PIVOT = 3,
    GLFW_EXTENSION = 4,
    INSTANCE_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2 = 5,
    KHR_DEVICE_GROUP_CREATION = 6,
    INSTANCE_EXTENSION_END_PIVOT = 7,

    //// device extension
    DEVICE_EXTENSION_BEGIN_PIVOT = 8,
    KHR_SWAPCHAIN = 9,

    // VMA support these extensions
    KHR_GET_MEMORY_REQUIREMENTS_2 = 10,
    KHR_DEDICATED_ALLOCATION = 11,
    KHR_BUFFER_DEVICE_ADDRESS = 12,
    KHR_DEVICE_GROUP = 13,
    DEVICE_EXTENSION_END_PIVOT = 14,

    //// Device features
    MAX_FEATURE_EXTENSION_COUNT
  };

  const char *const kFeatureExtensionNames[static_cast<uint32_t>(
      FeatureExtension::MAX_FEATURE_EXTENSION_COUNT)] = {
      "LAYER_BEGIN_PIVOT", // 0
      "VK_LAYER_KHRONOS_validation", // 1
      "LAYER_END_PIVOT", // 2

      "INSTANCE_EXTENSION_BEGIN_PIVOT", // 3
      "GLFW_EXTENSION", // 4
      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, // 5
      VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME, // 6
      "INSTANCE_EXTENSION_END_PIVOT", // 7

      "DEVICE_EXTENSION_BEGIN_PIVOT", // 8
      VK_KHR_SWAPCHAIN_EXTENSION_NAME, // 9
      VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME, // 10
      VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME, // 11
      VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, // 12
      VK_KHR_DEVICE_GROUP_EXTENSION_NAME, // 13
      "DEVICE_EXTENSION_END_PIVOT", // 14
  };

  VkConfig()
      : enableds_(static_cast<uint32_t>(
                      FeatureExtension::MAX_FEATURE_EXTENSION_COUNT),
                  EnableState::DISABLED) {}

  virtual ~VkConfig() = default;

  void setFeatureEnabled(FeatureExtension feature, EnableState enable_state) {
    enableds_[static_cast<uint32_t>(feature)] = enable_state;
  }

  EnableState isFeatureEnabled(FeatureExtension feature) const {
    return enableds_[static_cast<uint32_t>(feature)];
  }

  /**
   * \brief Check and update the instance create info, for enable layers and
   * extensions.
   */
  virtual void checkAndUpdate(VkInstanceCreateInfo &create_info) = 0;

  /**
   * \brief check and select the best physical device, for enable features and
   * extensions.
   *
   * \return the index of the best physical device.
   */
  virtual uint32_t
  checkSelectAndUpdate(const std::vector<PhysicalDevice> &physical_devices,
                       VkDeviceCreateInfo &create_info,
                       VkSurfaceKHR surface) = 0;
  void setDeviceType(VkPhysicalDeviceType device_type) {
    device_type_ = device_type;
  }

protected:
  std::vector<EnableState> enableds_;
  VkPhysicalDeviceType device_type_{VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU};
  
  std::vector<std::pair<const char *, EnableState>> request_layers_;
  std::vector<std::pair<const char *, EnableState>> request_device_extensions_;
    
  std::vector<const char *> enabled_layers_;
  std::vector<const char *> enabled_instance_extensions_;
  std::vector<const char *> enabled_device_extensions_;
  std::map<VkStructureType, std::shared_ptr<void>> extension_features_;
  void * extension_features_list_{nullptr}; //!< wrapper not own memory
  VkPhysicalDeviceFeatures device_features_{VK_FALSE};
};

class Vk11Config : public VkConfig {
public:
  Vk11Config() : VkConfig() {
    // for vma
    enableds_[static_cast<uint32_t>(FeatureExtension::KHR_GET_MEMORY_REQUIREMENTS_2)] = EnableState::REQUIRED;
    enableds_[static_cast<uint32_t>(FeatureExtension::KHR_DEDICATED_ALLOCATION)] = EnableState::REQUIRED;
    enableds_[static_cast<uint32_t>(FeatureExtension::KHR_BUFFER_DEVICE_ADDRESS)] = EnableState::REQUIRED;
    enableds_[static_cast<uint32_t>(FeatureExtension::KHR_DEVICE_GROUP_CREATION)] = EnableState::REQUIRED;
    enableds_[static_cast<uint32_t>(FeatureExtension::KHR_DEVICE_GROUP)] = EnableState::REQUIRED;
  }

  ~Vk11Config() = default;

  void checkAndUpdate(VkInstanceCreateInfo &create_info) override;

  uint32_t
  checkSelectAndUpdate(const std::vector<PhysicalDevice> &physical_devices,
                       VkDeviceCreateInfo &create_info,
                       VkSurfaceKHR surface) override;

private:
  void checkAndUpdateLayers(VkInstanceCreateInfo &create_info);
  void checkAndUpdateExtensions(VkInstanceCreateInfo &create_info);
};
} // namespace vk_engine