#include "framework/vk_driver.h"
#include "framework/error.h"
#include <cassert>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <algorithm>
#include <iostream>
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

    void VkDriver::initInstance()
    {
        if(VK_SUCCESS != volkInitialize())
        {
            throw std::runtime_error("failed to initialize volk!");
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

        if(enable_vk_validation_)
        {
            if(!checkValidationLayerSupport())
                return;
            instance_info.enabledLayerCount = static_cast<uint32_t>(request_validation_layers.size());
            instance_info.ppEnabledLayerNames = request_validation_layers.data();
        }
                
        if(VK_SUCCESS != vkCreateInstance(&instance_info, nullptr, &instance_))
            throw std::runtime_error("failed to create instance");

        volkLoadInstanceOnly(instance_);
    }

    void VkDriver::initDevice()
    {
        auto select_ret = selectPhysicalDevice(request_device_extensions);
        if(!select_ret.first)
        {
            throw std::runtime_error("failed to select suitable physical device!");
        }

        // queue info
        VkDeviceQueueCreateInfo queue_info{};
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
        if(enable_vk_validation_)
        {
            device_info.enabledLayerCount = request_validation_layers.size();
            device_info.ppEnabledLayerNames = request_validation_layers.data();
        } else device_info.enabledLayerCount = 0;
        device_info.enabledExtensionCount = 0;

        if(VK_SUCCESS != vkCreateDevice(physical_device_, &device_info, nullptr, &device_))
        {
            throw std::runtime_error("failed to create vulkan device!");
        }

        volkLoadDevice(device_);

        vkGetDeviceQueue(device_, select_ret.second, 0, &graphics_queue_);
    }

    bool VkDriver::init(const std::string &app_name, const bool enable_validation, GLFWwindow * window)
    {
        enable_vk_validation_ = enable_validation;        
        initInstance();

        assert(window != nullptr);
        if(VK_SUCCESS != glfwCreateWindowSurface(instance_, window, nullptr, &surface_))
        {
            throw std::runtime_error("failed to create window surface!");
            return false;
        }

        initDevice();
        
        checkSwapchainAbility();

        initSwapchain(window);
        return true;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }    

    void VkDriver::setupDebugMessenger() {

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;

        if (CreateDebugUtilsMessengerEXT(instance_, &createInfo, nullptr, &debug_messenger_) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }    

    bool VkDriver::checkSwapchainAbility()
    {
        VkSurfaceFormatKHR surface_format{VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        uint32_t format_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_, &format_count, nullptr);
        std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_, &format_count, surface_formats.data());
        auto format_itr = std::find_if(surface_formats.begin(),
                                       surface_formats.end(),
                                       [&surface_format](const VkSurfaceFormatKHR sf){
                                           return sf.format == surface_format.format && sf.colorSpace==surface_format.colorSpace; });
        if(format_itr == surface_formats.end())
        {
            throw std::runtime_error("B8G8R8_SNORM format, COLOR_SPACE_SRGB_NONLINEAR_KHR is not supported!");
            return false;
        }

        uint32_t present_mode_count = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_, surface_, &present_mode_count, nullptr);
        std::vector<VkPresentModeKHR> present_modes(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_, surface_, &present_mode_count, present_modes.data());
        auto present_itr = std::find_if(
                                        present_modes.begin(), present_modes.end(),
                                        [](const VkPresentModeKHR present_mode){
                                            return present_mode==VK_PRESENT_MODE_FIFO_KHR; });
        if(present_itr == present_modes.end())
        {
            throw std::runtime_error("FIFO present mode is not supported!");
            return false;
        }
        return true;
    }

    bool VkDriver::initSwapchain(GLFWwindow * window)
    {
        VkSurfaceCapabilitiesKHR surface_capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_, &surface_capabilities);        
        VkSurfaceFormatKHR surface_format{VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

        // real resolution for highdpi
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        VkExtent2D extent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };
        extent.width = std::clamp(extent.width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);

        uint32_t image_count = surface_capabilities.minImageCount + 1;
        if(surface_capabilities.maxImageCount > 0 && image_count > surface_capabilities.maxImageCount)
            image_count = surface_capabilities.maxImageCount;

        // create swapchain        
        VkSwapchainCreateInfoKHR swapchain_info{};
        swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchain_info.surface = surface_;
        swapchain_info.imageExtent = extent;
        swapchain_info.minImageCount = image_count;
        
        swapchain_info.imageFormat = surface_format.format; 
        swapchain_info.imageColorSpace = surface_format.colorSpace;
        swapchain_info.imageArrayLayers = 1;
        swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_info.preTransform = surface_capabilities.currentTransform;
        swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchain_info.clipped = VK_TRUE;
        swapchain_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        
        if(VK_SUCCESS != vkCreateSwapchainKHR(device_, &swapchain_info, nullptr, &swapchain_))
        {
            throw std::runtime_error("failed to create swapchain!");
        }
        std::cout << "!!!!!!!" << std::endl;
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
