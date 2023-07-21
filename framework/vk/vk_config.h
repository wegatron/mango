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
    KHR_VALIDATION_LAYER,
    LAYER_END_PIVOT,

    //// instance extension
    INSTANCE_EXTENSION_BEGIN_PIVOT,
    GLFW_EXTENSION,
    INSTANCE_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2,
    INSTANCE_EXTENSION_END_PIVOT,

    //// device extension
    DEVICE_EXTENSION_BEGIN_PIVOT,
    KHR_SWAPCHAIN,

    // VMA support these extensions
    KHR_GET_MEMORY_REQUIREMENTS_2,
    KHR_DEDICATED_ALLOCATION,
    KHR_BUFFER_DEVICE_ADDRESS,
    KHR_DEVICE_GROUP_CREATION,
    KHR_DEVICE_GROUP,
    DEVICE_EXTENSION_END_PIVOT,

    //// Device features
    MAX_FEATURE_EXTENSION_COUNT
  };

  const char *const kFeatureExtensionNames[static_cast<uint32_t>(
      FeatureExtension::MAX_FEATURE_EXTENSION_COUNT)] = {
      "LAYER_BEGIN_PIVOT",
      "VK_LAYER_KHRONOS_validation",
      "LAYER_END_PIVOT",

      "INSTANCE_EXTENSION_BEGIN_PIVOT",
      "GLFW_EXTENSION",
      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
      "INSTANCE_EXTENSION_END_PIVOT",

      "DEVICE_EXTENSION_BEGIN_PIVOT",
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
      VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
      VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME,
      VK_KHR_DEVICE_GROUP_EXTENSION_NAME,
      "DEVICE_EXTENSION_END_PIVOT",
  };

  VkConfig()
      : enableds_(static_cast<uint32_t>(
                      FeatureExtension::MAX_FEATURE_EXTENSION_COUNT),
                  EnableState::DISABLED), device_type_(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)  {}

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
  VkPhysicalDeviceType device_type_;
  
  std::vector<std::pair<const char *, EnableState>> request_layers_;
  std::vector<std::pair<const char *, EnableState>> request_device_extensions_;
    
  std::vector<const char *> enabled_layers_;
  std::vector<const char *> enabled_instance_extensions_;
  std::vector<const char *> enabled_device_extensions_;
  std::map<VkStructureType, std::shared_ptr<void>> extension_features_;
  void * extension_features_list_; //!< wrapper not own memory
};

class Vk11Config : public VkConfig {
public:
  Vk11Config() : VkConfig() {
    // for vma
    for (auto i = static_cast<uint32_t>(
             FeatureExtension::DEVICE_EXTENSION_BEGIN_PIVOT);
         i < static_cast<uint32_t>(FeatureExtension::DEVICE_EXTENSION_END_PIVOT); ++i) {
      enableds_[i] = EnableState::REQUIRED;
    }
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