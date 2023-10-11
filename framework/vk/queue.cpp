#include <framework/vk/queue.h>

namespace vk_engine
{
    VkResult CommandQueue::submit(const std::shared_ptr<CommandBuffer> &cmd_buffer, VkFence fence) const
    {
        // submit
        auto cmd_buf_handle = cmd_buf->getHandle();
        VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        info.commandBufferCount = 1;
        info.pCommandBuffers = &cmd_buf_handle;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = NULL;
        info.pWaitDstStageMask = NULL;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = NULL;
        return vkQueueSubmit(handle_, 1, &submit_info, fence);
    }


}