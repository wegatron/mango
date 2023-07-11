#include <framework/vk/frame_buffer.h>
#include <framework/utils/error.h>
namespace vk_engine {

RenderTarget::~RenderTarget() {
    for (auto &image_view : images_views_) {
        image_view.reset();
    }
    for (auto &image : images_) {
        image.reset();
    }
}

FrameBuffer::FrameBuffer(const std::shared_ptr<VkDriver> &driver,
          const std::shared_ptr<RenderPass> &render_pass,
          const std::shared_ptr<RenderTarget> &render_target)
{
    driver_ = driver;
    render_pass_ = render_pass;
    render_target_ = render_target;

    std::vector<VkImageView> attachments;
    for (const auto &image_view : render_target_->getImageViews()) {
        attachments.push_back(image_view->getHandle());
    }

    VkFramebufferCreateInfo framebuffer_info = {};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = render_pass_->getHandle();
    framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebuffer_info.pAttachments = attachments.data();
    framebuffer_info.width = render_target_->getWidth();
    framebuffer_info.height = render_target_->getHeight();
    framebuffer_info.layers = render_target_->getLayers();

    auto result = vkCreateFramebuffer(driver_->getDevice(), &framebuffer_info, nullptr, &framebuffer_);
    if(result != VK_SUCCESS) {
        throw VulkanException(result, "Failed to create framebuffer.");
    }    
}

FrameBuffer::~FrameBuffer()
{
    vkDestroyFramebuffer(driver_->getDevice(), framebuffer_, nullptr);
}
} // namespace vk_engine