#pragma once

#include <memory>
#include <framework/vk/vk_driver.h>

namespace vk_engine
{
    class CommandBuffer;
    class CommandPool final
    {
    public:
        // refer to: https://arm-software.github.io/vulkan_best_practice_for_mobile_developers/samples/performance/command_buffer_usage/command_buffer_usage_tutorial.html#allocate-and-free
        enum class CmbResetMode
        {
            AlwaysAllocate = 0,
            ResetIndividually = 1, // reset command buffers individually
            ResetPool = 2
        };

        CommandPool(const std::shared_ptr<VkDriver> &driver, uint32_t queue_family_index, CmbResetMode mode);
        
        CommandPool(const CommandPool &) = delete;
        CommandPool &operator=(const CommandPool &) = delete;
        CommandPool(CommandPool &&) = delete;

        ~CommandPool();

        void reset(bool memory2system);

        std::shared_ptr<CommandBuffer> requestCommandBuffer(VkCommandBufferLevel level);
    
        VkCommandPool getHandle() const { return command_pool_; }

        CmbResetMode getResetMode() const { return mode_; }
    
    private:
        std::shared_ptr<VkDriver> driver_;
        VkCommandPool command_pool_{VK_NULL_HANDLE};
        CmbResetMode mode_;
        std::vector<std::shared_ptr<CommandBuffer>> command_buffers_;
    };

    class CommandBuffer final
    {
    public:
        CommandBuffer(const std::shared_ptr<VkDriver> &driver, CommandPool &command_pool, VkCommandBufferLevel level);

        CommandBuffer(const CommandBuffer &) = delete;
        CommandBuffer &operator=(const CommandBuffer &) = delete;
        CommandBuffer(CommandBuffer &&) = delete;

        ~CommandBuffer(); // free command buffer

        void reset(bool memory2pool);

        VkCommandBuffer getHandle() const { return command_buffer_; }

    private:
        std::shared_ptr<VkDriver> driver_;
        VkCommandPool command_pool_;
        VkCommandBuffer command_buffer_{VK_NULL_HANDLE};
        #if !defined(NDEBUG)
        bool resetable_;
        #endif
    };
}