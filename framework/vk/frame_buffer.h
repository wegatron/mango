#pragma once

#include <framework/vk/vk_driver.h>
#include <framework/vk/render_pass.h>
#include <framework/vk/image.h>
#include <volk.h>

namespace vk_engine {

/**
 * \brief render target is a collection of images used for rendering.
 * use this class for render target's creation and management.
 */
class RenderTarget final {
public:
  ~RenderTarget();

  RenderTarget(const RenderTarget &) = delete;
  RenderTarget &operator=(const RenderTarget &) = delete;

  static RenderTarget create(const std::shared_ptr<VkDriver> &driver,
                             uint32_t color_attachment_count,
                             VkFormat color_format, VkFormat ds_format,
                             uint32_t width, uint32_t height, uint32_t layers);

  static RenderTarget
  create(const std::vector<std::shared_ptr<ImageView>> &image_views);

  const std::vector<std::shared_ptr<ImageView>> &getImageViews() const {
    return images_views_;
  }

  uint32_t getWidth() const { return width_; }

  uint32_t getHeight() const { return height_; }

  uint32_t getLayers() const { return layers_; }

private:
  RenderTarget() = default;

  uint32_t width_{0};
  uint32_t height_{0};
  uint32_t layers_{1}; // should be one, for multiview

  std::shared_ptr<VkDriver> driver_;
  std::vector<std::shared_ptr<ImageView>> images_views_;
  std::vector<std::shared_ptr<Image>> images_;
};

/**
 * \brief framebuffer is a combination of render target and render pass,
 * and used for manage the VkFramebuffer.
 * 
 * render pass defining what render passes the framebuffer will be compatible with.
 */
class FrameBuffer final {
public:
  FrameBuffer(const std::shared_ptr<VkDriver> &driver,
            const std::shared_ptr<RenderPass> &render_pass,
            const std::shared_ptr<RenderTarget> &render_target);

  FrameBuffer(const FrameBuffer &) = delete;
  FrameBuffer &operator=(const FrameBuffer &) = delete;
  FrameBuffer(FrameBuffer &&) = delete;

  ~FrameBuffer();

  VkFramebuffer getHandle() const { return framebuffer_; }

  uint32_t getWidth() const { return render_target_->getWidth(); }

  uint32_t getHeight() const { return render_target_->getHeight(); }

  uint32_t getLayers() const { return render_target_->getLayers(); }

private:
    std::shared_ptr<VkDriver> driver_;
    std::shared_ptr<RenderPass> render_pass_;
    std::shared_ptr<RenderTarget> render_target_;
    
    VkFramebuffer framebuffer_{VK_NULL_HANDLE};
};
} // namespace vk_engine