#include "framework/vk_driver.h"
#include <cassert>
#include <stdexcept>
#include <vector>
#include <volk.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

const std::vector<const char*> request_validation_layers = {
    "VK_LAYER_KHRONOS_validation"  
};

namespace vk_engine
{
    bool checkValidationLayerSupport()
    {
        uint32_t layer_count = 0;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
        std::vector<VkLayerProperties> available_layers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

        for(const auto &name : request_validation_layers)
        {
            bool is_find = false;
            for(const auto &l : available_layers)
            {
                if(strcmp(l.layerName, name) == 0)
                {
                    is_find = true;
                    break;
                }
            }

            if(!is_find)
            {
                std::string error_msg = std::string("validation layer ") + std::string(name) + " requested, but not available!";
                throw std::runtime_error(error_msg);
                return false;
            }
        }
        return true;
    }

    bool VkDriver::init(const std::string &app_name, const bool enable_validation)
    {
        if(VK_SUCCESS != volkInitialize())
        {
            throw std::runtime_error("failed to initialize volk!");
            return false;
        }

        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "vk_engine";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "No Engine";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0); // 以上这些意义不大
        app_info.apiVersion = VK_API_VERSION_1_0; // vulkan api version 

        VkInstanceCreateInfo instance_info{};
        instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_info.pApplicationInfo = &app_info;
        
        // glfw extension
        uint32_t glfw_extension_count = 0;
        const char ** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
        instance_info.enabledExtensionCount = glfw_extension_count;
        instance_info.ppEnabledExtensionNames = glfw_extensions;

        // validation layer
        instance_info.enabledLayerCount = 0;
        if(enable_validation)
        {
            if(!checkValidationLayerSupport())
                return false;
            instance_info.enabledLayerCount = static_cast<uint32_t>(request_validation_layers.size());
            instance_info.ppEnabledLayerNames = request_validation_layers.data();
        }
                
        if(VK_SUCCESS != vkCreateInstance(&instance_info, nullptr, &instance_)) {
            throw std::runtime_error("failed to create instance");
            return false;
        }

        volkLoadInstance(instance_);

        // physical device, descrete gpu and support geometry shader
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(instance_, &device_count, nullptr);
        if(device_count == 0)
        {
            throw std::runtime_error("failed to find gpu with vulkan support!");
            return false;
        }

        std::vector<VkPhysicalDevice> physical_devices(device_count);
        VkPhysicalDeviceFeatures features;
        vkEnumeratePhysicalDevices(instance_, &device_count, physical_devices.data());
        for(const auto &pd : physical_devices)
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(pd, &properties);            
            vkGetPhysicalDeviceFeatures(pd, &features);

            if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && features.geometryShader)
            {
                physical_device_ = pd;
                break;
            }
        }
        if(physical_device_ == VK_NULL_HANDLE)
        {
            throw std::runtime_error("unable to find suitable vulkan device!");
            return false;
        }

        // queue
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &queue_family_count, queue_family_properties.data());
        uint32_t selected_queue_family_index = -1;
        for(uint32_t i = 0; i<queue_family_count; ++i)        
        {
            const auto &p = queue_family_properties[i];
            if((p.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
                && (p.queueFlags & VK_QUEUE_TRANSFER_BIT) 
                && (p.queueFlags & VK_QUEUE_COMPUTE_BIT))
            {
                selected_queue_family_index = i;
            }
        }
        if(selected_queue_family_index == -1)
        {
            throw std::runtime_error("unable to find a queue support graphics, compute and transfer!");
            return false;
        }

        // queue info
        VkDeviceQueueCreateInfo queue_info;
        queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info.queueFamilyIndex = selected_queue_family_index;
        queue_info.queueCount = 1;
        float queue_priority = 1.0f;
        queue_info.pQueuePriorities = &queue_priority;

        // logical device
        VkDeviceCreateInfo device_info{};
        device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_info.pQueueCreateInfos = &queue_info;
        device_info.queueCreateInfoCount = 1;
        device_info.pEnabledFeatures = &features;
        if(enable_validation)
        {
            device_info.enabledLayerCount = request_validation_layers.size();
            device_info.ppEnabledLayerNames = request_validation_layers.data();
        } else device_info.enabledLayerCount = 0;
        device_info.enabledExtensionCount = 0;

        if(VK_SUCCESS != vkCreateDevice(physical_device_, &device_info, nullptr, &device_))
        {
            throw std::runtime_error("failed to create vulkan device!");
            return false;
        }
        return true;
    }

    VkDriver::~VkDriver()
    {        
        vkDestroyInstance(instance_, nullptr);
    }
}