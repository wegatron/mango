#pragma once

#include <volk.h>
#include <memory>
#include <vector>

namespace vk_engine
{
class VkDriver;
class CommandBuffer;

class CommandQueue final
{
public:
    ~CommandQueue();

	VkQueue getHandle() const;

	uint32_t getFamilyIndex() const;

	uint32_t getIndex() const;

	const VkQueueFamilyProperties &get_properties() const;

	VkBool32 supportPresent() const;

	VkResult submit(const std::vector<VkSubmitInfo> &submit_infos, VkFence fence) const;

	VkResult submit(const std::shared_ptr<CommandBuffer> &cmd_buf, VkFence fence) const;

	VkResult present(const VkPresentInfoKHR &present_infos) const;

	VkResult waitIdle() const;

private:

    CommandQueue(VkDevice device, uint32_t family_index, VkQueueFlags flags, VkBool32 can_present, uint32_t index):
        family_index_(family_index), flags_(flags), index_(index), can_present_(can_present)
    {
        vkGetDeviceQueue(device, family_index, 0, &handle_);
    }

	VkQueue handle_{VK_NULL_HANDLE};

	uint32_t family_index_;

	uint32_t index_;

	VkBool32 can_present_;

	VkQueueFlags flags_;

friend class VkDriver;
};
}