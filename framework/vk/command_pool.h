#pragma once

#include <framework/vk/vk_driver.h>

namespace vk_engine
{
    class CommandPool final
    {
    public:
        CommandPool(const std::shared_ptr<VkDriver> &driver, uint32_t queue_family_index, VkCommandPoolCreateFlags flags);
        
        CommandPool(const CommandPool &) = delete;
        CommandPool &operator=(const CommandPool &) = delete;
        CommandPool(CommandPool &&) = delete;

        ~CommandPool();
    
        VkCommandPool getHandle() const { return command_pool_; }
    
    private:
        std::shared_ptr<VkDriver> driver_;
        VkCommandPool command_pool_{VK_NULL_HANDLE};
    };

    class CommandBuffer final
    {
    public:
        CommandBuffer(const std::shared_ptr<VkDriver> &driver, const std::shared_ptr<CommandPool> &command_pool, VkCommandBufferLevel level);
        
        CommandBuffer(const CommandBuffer &) = delete;
        CommandBuffer &operator=(const CommandBuffer &) = delete;
        CommandBuffer(CommandBuffer &&) = delete;

        ~CommandBuffer();
    
        VkCommandBuffer getHandle() const { return command_buffer_; }

    private:
        std::shared_ptr<VkDriver> driver_;
        std::shared_ptr<CommandPool> command_pool_;
        VkCommandBuffer command_buffer_{VK_NULL_HANDLE};
    };
}