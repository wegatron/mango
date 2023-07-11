#include <framework/utils/error.h>
#include <framework/vk/frame_buffer.h>
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
                         const std::shared_ptr<RenderTarget> &render_target) {
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

  auto result = vkCreateFramebuffer(driver_->getDevice(), &framebuffer_info,
                                    nullptr, &framebuffer_);
  if (result != VK_SUCCESS) {
    throw VulkanException(result, "Failed to create framebuffer.");
  }
}

FrameBuffer::~FrameBuffer() {
  vkDestroyFramebuffer(driver_->getDevice(), framebuffer_, nullptr);
}

RenderTarget::RenderTarget(const std::shared_ptr<VkDriver> &driver,
                           std::initializer_list<VkFormat> color_format,
                           VkFormat ds_format, uint32_t width, uint32_t height,
                           uint32_t layers)
    : color_formats_(color_format), ds_format_(ds_format), width_(width),
      height_(height), layers_(layers) {
  driver_ = driver;
  VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                            VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                            VK_IMAGE_USAGE_SAMPLED_BIT;
  for (auto format : color_formats_) {
    auto image = std::make_shared<Image>(driver_, format, width_, height_,
                                         layers_, usage);
    images_.push_back(image);
    auto image_view = std::make_shared<ImageView>(
        driver_, image, VK_IMAGE_VIEW_TYPE_2D, format, 0, 0, 1, 1);
    images_views_.push_back(image_view);
  }

  if (ds_format_ != VK_FORMAT_UNDEFINED) {
    auto image =
        std::make_shared<Image>(driver_, ds_format_, width_, height_, layers_,
                                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    images_.push_back(image);
    auto image_view = std::make_shared<ImageView>(
        driver_, image, VK_IMAGE_VIEW_TYPE_2D, ds_format_);
    images_views_.push_back(image_view);
  }
}

RenderTarget::RenderTarget(
    const std::vector<std::shared_ptr<ImageView>> &image_views,
    std::initializer_list<VkFormat> color_format, VkFormat ds_format,
    uint32_t width, uint32_t height, uint32_t layers)
    : color_formats_(color_format), ds_format_(ds_format), width_(width),
      height_(height), layers_(layers) {}
} // namespace vk_engine