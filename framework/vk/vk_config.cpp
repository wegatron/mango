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
    VkDeviceCreateInfo &create_info) 
{
    for(auto i=static_cast<uint32_t>(FeatureExtension::KHR_SWAPCHAIN);
        i<static_cast<uint32_t>(FeatureExtension::KHR_DEVICE_GROUP); ++i)
    {
        if(enableds_[i] != EnableState::DISABLED)
        {
            request_device_extensions_.emplace_back(kFeatureExtensionNames[i], enableds_[i]);
        }
    }

    for (const auto &pd : physical_devices) {
        auto handle = pd.getHandle();
        uint32_t graphics_queue_family_index = pd.getGraphicsQueueFamilyIndex();
        if (graphics_queue_family_index == 0XFFFFFFFF)
        continue;
        VkBool32 surface_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(handle,
                                            graphics_queue_family_index, surface_,
                                            &surface_support);
        if (!surface_support ||
            (pd.getProperties().deviceType !=
            VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) ||
            !(pd.getFeatures().geometryShader))
        continue;

        const auto &device_extensions = pd.getExtensionProperties();
        #if !defined(NDEBUG)
        for(const auto &ext : device_extensions)
            LOGD("device extension: {}", ext.extensionName);
        #endif
        bool extension_support = true;
        for (const auto &req_ext : request_extensions) {
        bool is_find = false;
        for (const auto &ext : device_extensions) {
            if (strcmp(ext.extensionName, req_ext.name) == 0) {
            is_find = true;
            break;
            }
        }
        if(is_find) {
            enabled_device_extensions_.push_back(req_ext.name);
            continue;
        }
        if (req_ext.required) {
            extension_support = false;
            break;
        }
        }
        if (!extension_support)
        continue;
        return std::make_pair(true, graphics_queue_family_index);
    }

    return 0;
}


void Vk11Config::checkAndUpdateLayers(VkInstanceCreateInfo &create_info)
{
  auto enb_state =
      enableds_[static_cast<uint32_t>(FeatureExtension::KHR_VALIDATION_LAYER)];
  std::string layer_name = "VK_LAYER_KHRONOS_validation";

  request_layers_ = std::vector<std::pair<std::string, EnableState>>{
    {layer_name, EnableState::REQUIRED}
  };

  uint32_t layer_count = 0;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
  std::vector<const char *> enabled_layers; enabled_layers.reserve(request_layers_.size());
  for(const auto layer : request_layers_)
  {
    bool is_find = false;
    for(const auto &l : available_layers)
    {
      if(strcmp(l.layerName, layer.first.c_str()) == 0)
      {
        is_find = true;
        enabled_layers.emplace_back(layer.first);
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

  create_info.enabledLayerCount = enabled_layers.size();
  create_info.ppEnabledLayerNames = enabled_layers.data();
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