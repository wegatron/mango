#include <framework/vk/commands.h>
#include <framework/utils/error.h>

namespace vk_engine
{
    CommandPool::CommandPool(const std::shared_ptr<VkDriver> &driver, uint32_t queue_family_index, CmbResetMode mode)
        : driver_(driver), mode_(mode)
    {
        VkCommandPoolCreateFlags flags[] = {
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
        };

        VkCommandPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.queueFamilyIndex = queue_family_index;
        pool_info.flags = flags[static_cast<int>(mode)];
        
        if (vkCreateCommandPool(driver->getDevice(), &pool_info, nullptr, &command_pool_) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    CommandPool::~CommandPool()
    {
        for(auto &command_buffer : command_buffers_)
        {
            if(command_buffer.use_count() != 1)
            {
                throw VulkanException(VK_RESULT_MAX_ENUM, "command pool destroy with command buffer is still in use!");
            }
        }
        vkDestroyCommandPool(driver_->getDevice(), command_pool_, nullptr);
    }

    void CommandPool::reset(bool memory2system)
    {
        VkCommandPoolResetFlags flag = memory2system ? VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT : 0;
        vkResetCommandPool(driver_->getDevice(), command_pool_, flag);
    }

    std::shared_ptr<CommandBuffer> CommandPool::requestCommandBuffer(VkCommandBufferLevel level)
    {
        auto cmdb = std::make_shared<CommandBuffer>(driver_, *this, level);
        command_buffers_.emplace_back(cmdb);

        return cmdb;
    }

    CommandBuffer::CommandBuffer(const std::shared_ptr<VkDriver> &driver, CommandPool &command_pool, VkCommandBufferLevel level)
        : driver_(driver), command_pool_(command_pool.getHandle())
    {
        #if !defined(NDEBUG)
        resetable_ = (command_pool.getResetMode() != CommandPool::CmbResetMode::ResetPool);
        #endif
        
        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = command_pool.getHandle();
        alloc_info.level = level;
        alloc_info.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(driver_->getDevice(), &alloc_info, &command_buffer_) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    CommandBuffer::~CommandBuffer()
    {
        if(command_buffer_ != VK_NULL_HANDLE)
        {
            vkFreeCommandBuffers(driver_->getDevice(), command_pool_, 1, &command_buffer_);
        }
    }

    void CommandBuffer::reset(bool memory2pool)
    {
        #if !defined(NDEBUG)
        if(!resetable_)
        {
            throw std::runtime_error("command buffer is not resetable!");
        }
        #endif

        VkCommandBufferResetFlags flag = memory2pool ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0;
        vkResetCommandBuffer(command_buffer_, flag);
    }
}