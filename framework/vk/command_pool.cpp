#include <framework/vk/command_pool.h>

namespace vk_engine
{
    CommandPool::CommandPool(const std::shared_ptr<VkDriver> &driver, uint32_t queue_family_index, VkCommandPoolCreateFlags flags)
        : driver_(driver)
    {
        VkCommandPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.queueFamilyIndex = queue_family_index;
        pool_info.flags = flags;
        
        if (vkCreateCommandPool(driver->getDevice(), &pool_info, nullptr, &command_pool_) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    CommandPool::~CommandPool()
    {
        vkDestroyCommandPool(driver_->getDevice(), command_pool_, nullptr);
    }
}