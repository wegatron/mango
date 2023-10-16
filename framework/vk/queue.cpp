#include <framework/vk/queue.h>
#include <framework/vk/commands.h>

namespace vk_engine
{
    VkResult CommandQueue::submit(const std::shared_ptr<CommandBuffer> &cmd_buf, VkFence fence) const
    {
        // submit
        auto cmd_buf_handle = cmd_buf->getHandle();
        VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmd_buf_handle;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = NULL;
        submit_info.pWaitDstStageMask = NULL;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = NULL;
        return vkQueueSubmit(handle_, 1, &submit_info, fence);
    }


}