#include <framework/utils/vk/queue.h>
#include <framework/utils/vk/commands.h>

namespace vk_engine
{
    VkResult CommandQueue::submit(const std::vector<VkSubmitInfo> &submit_infos, VkFence fence) const
    {
        return vkQueueSubmit(handle_, submit_infos.size(), submit_infos.data(), fence);
    }

    VkResult CommandQueue::submit(const std::shared_ptr<CommandBuffer> &cmd_buf, VkFence fence) const
    {
        // submit
        auto cmd_buf_handle = cmd_buf->getHandle();
        VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmd_buf_handle;
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores = NULL;
        submit_info.pWaitDstStageMask = NULL;
        submit_info.signalSemaphoreCount = 0;
        submit_info.pSignalSemaphores = NULL;
        return vkQueueSubmit(handle_, 1, &submit_info, fence);
    }

    VkResult CommandQueue::present(const VkPresentInfoKHR &present_info) const
    {
        if (!can_present_)
        {
            return VK_ERROR_INCOMPATIBLE_DISPLAY_KHR;
        }

        return vkQueuePresentKHR(handle_, &present_info);        
    }

	VkResult CommandQueue::waitIdle() const
    {
        return vkQueueWaitIdle(handle_);
    }
}