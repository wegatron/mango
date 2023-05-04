#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vk_engine
{
    class VkDriver
    {
    public:
        VkDriver() = default;
        ~VkDriver();
        
        VkDriver(const VkDriver &) = delete;
        VkDriver &operator=(const VkDriver &) = delete;
        bool init(const std::string &app_name, const bool enable_validation, GLFWwindow *window);
    private:

        std::pair<bool, uint32_t> selectPhysicalDevice(std::vector<const char*> request_extensions);
        
        bool checkSwapchainAbility();
        
        bool createSwapchain();

        VkInstance instance_{VK_NULL_HANDLE};
        VkPhysicalDevice physical_device_{VK_NULL_HANDLE};
        VkDevice device_{VK_NULL_HANDLE};
        
        VkQueue graphics_queue_{VK_NULL_HANDLE};
        VkSurfaceKHR surface_{VK_NULL_HANDLE};
        VkSwapchainKHR swapchain_{VK_NULL_HANDLE};
    };
}