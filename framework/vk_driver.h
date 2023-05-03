#pragma once

#include <string>
#include <vulkan/vulkan.h>

namespace vk_engine
{
    class VkDriver
    {
    public:
        VkDriver() = default;
        ~VkDriver();
        
        VkDriver(const VkDriver &) = delete;
        VkDriver &operator=(const VkDriver &) = delete;
        bool init(const std::string &app_name, const bool enable_validation);
    private:
        VkInstance instance_{VK_NULL_HANDLE};
        VkPhysicalDevice physical_device_{VK_NULL_HANDLE};
        VkDevice device_{VK_NULL_HANDLE};     
    };
}