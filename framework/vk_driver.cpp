#include "framework/vk_driver.h"
#include <cassert>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <volk.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "framework/physical_device.h"

const std::vector<const char*> request_validation_layers = {
    "VK_LAYER_KHRONOS_validation",
};

const std::vector<const char*> request_device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
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

    std::pair<bool, uint32_t> VkDriver::selectPhysicalDevice(
        std::vector<const char *> request_extensions)
    {
        auto physical_devices = PhysicalDevice::getPhysicalDevices(instance_);
        for(const auto &pd : physical_devices)
        {
            physical_device_ = pd.getHandle();
            uint32_t graphics_queue_family_index = pd.getGraphicsQueueFamilyIndex();
            if(graphics_queue_family_index == 0XFFFFFFFF)
                continue;
            VkBool32 surface_support = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device_, graphics_queue_family_index, surface_, &surface_support);
            if(!surface_support
                || (pd.getProperties().deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                || !(pd.getFeatures().geometryShader))
                continue;

            const auto &device_extensions = pd.getExtensionProperties();
            bool extension_support = true;
            for(const auto &req_ext : request_extensions)
            {
                bool is_find = false;
                for(const auto &ext : device_extensions)
                {
                    if(strcmp(ext.extensionName, req_ext) == 0)
                    {
                        is_find = true;
                        break;
                    }
                }
                if(!is_find) {
                    extension_support = false;
                    break;
                }
            }
            if(!extension_support) continue; 
            return std::make_pair(true, graphics_queue_family_index);            
        }

        return std::make_pair(false, -1);
    }

    bool VkDriver::init(const std::string &app_name, const bool enable_validation, GLFWwindow * window)
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

        assert(window != nullptr);
        if(VK_SUCCESS != glfwCreateWindowSurface(instance_, window, nullptr, &surface_))
        {
            throw std::runtime_error("failed to create window surface!");
            return false;
        }
        
        auto select_ret = selectPhysicalDevice(request_device_extensions);
        if(!select_ret.first)
        {
            throw std::runtime_error("failed to select suitable physical device!");
            return false;
        }

        // queue info
        VkDeviceQueueCreateInfo queue_info;
        float queue_priority = 1.0f;
        queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info.queueFamilyIndex = select_ret.second;
        queue_info.queueCount = 1;        
        queue_info.pQueuePriorities = &queue_priority;

        // logical device
        VkDeviceCreateInfo device_info{};
        VkPhysicalDeviceFeatures device_features{}; // no need to set, all false, when use features to enable all supported features may crash
        device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_info.pQueueCreateInfos = &queue_info;
        device_info.queueCreateInfoCount = 1;
        device_info.pEnabledFeatures = &device_features;
        device_info.enabledExtensionCount = request_device_extensions.size();        
        device_info.ppEnabledExtensionNames = request_device_extensions.data(); 

        device_features.geometryShader = VK_TRUE;
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

        vkGetDeviceQueue(device_, select_ret.second, 0, &graphics_queue_);

        return createSwapchain();        
    }

    bool VkDriver::createSwapchain()
    {
        VkSurfaceCapabilitiesKHR surface_capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_, &surface_capabilities);
        uint32_t format_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_, &format_count, nullptr);
        std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_, &format_count, surface_formats.data());

        uint32_t present_mode_count = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_, surface_, &present_mode_count, nullptr);
        std::vector<VkPresentModeKHR> present_modes(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_, surface_, &present_mode_count, present_modes.data());

        // select format and present mode
        return true;
    }

    VkDriver::~VkDriver()
    {
        vkDestroySwapchainKHR(device_, swapchain_, nullptr);
        vkDestroyDevice(device_, nullptr);
        vkDestroySurfaceKHR(instance_, surface_, nullptr);
        vkDestroyInstance(instance_, nullptr);        
    }
}