#include "framework/physical_device.h"

#include <volk.h>

namespace vk_engine
{

    std::vector<PhysicalDevice> PhysicalDevice::getPhysicalDevices(VkInstance instance)
    {
        // physical device, discrete gpu and support geometry shader
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
        if(device_count == 0) return {};

        std::vector<PhysicalDevice> physical_devices(device_count);
        std::vector<VkPhysicalDevice> device_handles(device_count);
        vkEnumeratePhysicalDevices(instance, &device_count, device_handles.data());
        for(int i=0; i<device_count; ++i)
        {
            auto &handle = device_handles[i];
            physical_devices[i].physical_device_ = handle;

            vkGetPhysicalDeviceProperties(handle, &(physical_devices[i].properties_));
            vkGetPhysicalDeviceFeatures(handle, &(physical_devices[i].features_));

            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(handle, nullptr, &extensionCount, nullptr);                
            physical_devices[i].extensions_.resize(extensionCount);
            vkEnumerateDeviceExtensionProperties(handle, nullptr, &extensionCount, physical_devices[i].extensions_.data());

            // queue family
            uint32_t queue_family_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(handle, &queue_family_count, nullptr);
            std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
            vkGetPhysicalDeviceQueueFamilyProperties(handle, &queue_family_count, queue_family_properties.data());
            uint32_t selected_queue_family_index = -1;
            for(uint32_t i = 0; i<queue_family_count; ++i)        
            {
                const auto &p = queue_family_properties[i];
                if((p.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
                    && (p.queueFlags & VK_QUEUE_TRANSFER_BIT)
                    && (p.queueFlags & VK_QUEUE_COMPUTE_BIT))
                {
                    physical_devices[i].graphics_queue_family_index_ = i;
                }
            }        
        }
        return physical_devices;
    }
} // namespace vk_engine