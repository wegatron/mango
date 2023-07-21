#include <framework/vk/vk_config.h>
#include <framework/utils/error.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string.h>

namespace vk_engine {

void Vk11Config::checkAndUpdate(VkInstanceCreateInfo &create_info)
{
    checkAndUpdateLayers(create_info);
    checkAndUpdateExtensions(create_info);
}

uint32_t Vk11Config::checkSelectAndUpdate(
    const std::vector<PhysicalDevice> &physical_devices,
    VkDeviceCreateInfo &create_info, VkSurfaceKHR surface) 
{
    for(auto i=static_cast<uint32_t>(FeatureExtension::DEVICE_EXTENSION_BEGIN_PIVOT)+1;
        i<static_cast<uint32_t>(FeatureExtension::DEVICE_EXTENSION_END_PIVOT); ++i)
    {
        if(enableds_[i] != EnableState::DISABLED)
        {
            request_device_extensions_.emplace_back(kFeatureExtensionNames[i], enableds_[i]);
        }
    }

    uint32_t selected_physical_device_index = -1;
    enabled_device_extensions_.reserve(request_device_extensions_.size());
    for (uint32_t device_index = 0; device_index < physical_devices.size();
         ++device_index)
    {
        enabled_device_extensions_.clear();
        auto &pd = physical_devices[device_index];
        auto handle = pd.getHandle();
        uint32_t graphics_queue_family_index = pd.getGraphicsQueueFamilyIndex();
        if (graphics_queue_family_index == 0XFFFFFFFF)
          continue;
        VkBool32 surface_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(handle,
                                            graphics_queue_family_index, surface,
                                            &surface_support);
        if (!surface_support || (pd.getProperties().deviceType != device_type_))
          continue;

        const auto &device_extensions = pd.getExtensionProperties();
        #if !defined(NDEBUG)
        // TODO output device name
        for(const auto &ext : device_extensions)
            LOGD("device extension: {}", ext.extensionName);
        #endif
        bool extension_support = true;
        for (const auto &req_ext : request_device_extensions_) {
          bool is_find = false;
          for (const auto &ext : device_extensions) {
              if (strcmp(ext.extensionName, req_ext.first) == 0) {
                is_find = true;
                break;
              }
          }
          if(is_find) {
              enabled_device_extensions_.emplace_back(req_ext.first);
              continue;
          }
          if (req_ext.second == EnableState::REQUIRED) {
              extension_support = false;
              break;
          }
        }

        if(extension_support)
        {
            selected_physical_device_index = device_index;
            break;
        }
    }
    return selected_physical_device_index;
}


void Vk11Config::checkAndUpdateLayers(VkInstanceCreateInfo &create_info)
{
    for(auto i=static_cast<uint32_t>(FeatureExtension::LAYER_BEGIN_PIVOT)+1;
        i<static_cast<uint32_t>(FeatureExtension::LAYER_END_PIVOT); ++i)
    {
        if(enableds_[i] != EnableState::DISABLED)
        {
            request_layers_.emplace_back(kFeatureExtensionNames[i], enableds_[i]);
        }
    }  

  uint32_t layer_count = 0;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
  enabled_layers_.reserve(request_layers_.size());
  for(const auto layer : request_layers_)
  {
    bool is_find = false;
    for(const auto &l : available_layers)
    {
      if(strcmp(l.layerName, layer.first) == 0)
      {
        is_find = true;
        enabled_layers_.emplace_back(layer.first);
        break;
      }
    }

    if(!is_find)
    {
      std::string info = std::string("validation layer ") + std::string(layer.first) + " requested, but not available!";
      if(layer.second == EnableState::REQUIRED)
      {
        throw VulkanException(VK_RESULT_MAX_ENUM, info);
      }
      else
      {
        LOGI(info);
        continue;
      }
    }
  }

  create_info.enabledLayerCount = enabled_layers_.size();
  create_info.ppEnabledLayerNames = enabled_layers_.data();
}


void Vk11Config::checkAndUpdateExtensions(VkInstanceCreateInfo &create_info)
{
    if(enableds_[static_cast<uint32_t>(FeatureExtension::GLFW_EXTENSION)] == EnableState::REQUIRED)
    {
        uint32_t glfw_extension_count = 0;
        const char **glfw_extensions =
        glfwGetRequiredInstanceExtensions(&glfw_extension_count);
        create_info.enabledExtensionCount = glfw_extension_count;
        create_info.ppEnabledExtensionNames = glfw_extensions;
    }
}

} // namespace vk_engine