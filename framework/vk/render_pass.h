#pragma once

#include <vector>
#include <volk.h>

namespace vk_engine {

struct Attachment {
  VkFormat format{VK_FORMAT_UNDEFINED};

  VkSampleCountFlagBits samples{VK_SAMPLE_COUNT_1_BIT};

  VkImageUsageFlags usage{VK_IMAGE_USAGE_SAMPLED_BIT};

  VkImageLayout initial_layout{VK_IMAGE_LAYOUT_UNDEFINED};

  Attachment() = default;

  Attachment(VkFormat format, VkSampleCountFlagBits samples,
             VkImageUsageFlags usage);
};

struct SubpassInfo
{
	std::vector<uint32_t> input_attachments;

	std::vector<uint32_t> output_attachments;

	std::vector<uint32_t> color_resolve_attachments;

	bool disable_depth_stencil_attachment;

	uint32_t depth_stencil_resolve_attachment;

	VkResolveModeFlagBits depth_stencil_resolve_mode;

	//std::string debug_name;
};

class RenderPass final {
public:
  RenderPass(const RenderPass &) = delete;
  RenderPass &operator=(const RenderPass &) = delete;

  RenderPass() = default;
  ~RenderPass() = default;

  VkRenderPass getHandle() const { return handle_; }

private:
  VkRenderPass handle_{VK_NULL_HANDLE};
};
} // namespace vk_engine