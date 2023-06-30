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
  create(const std::vector<std::shared_ptr<Image>> &images,
         const std::vector<std::shared_ptr<ImageView>> &image_views);

  uint32_t getWidth() const { return width_; }

  uint32_t getHeight() const { return height_; }

  uint32_t getLayers() const { return layers_; }

private:
  RenderTarget() = default;

  bool own_images_{false};
  uint32_t width_{0};
  uint32_t height_{0};
  uint32_t layers_{0};

  std::shared_ptr<VkDriver> driver_;

  std::vector<std::shared_ptr<Image>> images_;
  std::vector<std::shared_ptr<ImageView>> images_views_;
};

/**
 * \brief framebuffer is a combination of render target and render pass,\
 * and used for manage the VkFramebuffer
 */
class FrameBuffer final {
public:
  FrameBuffer(const FrameBuffer &) = delete;
  FrameBuffer &operator=(const FrameBuffer &) = delete;

  FrameBuffer() = default;
  ~FrameBuffer() = default;

  void init(const std::shared_ptr<VkDriver> &driver,
            const std::shared_ptr<RenderPass> &render_pass,
            const std::shared_ptr<RenderTarget> &render_target);

  VkFramebuffer getHandle() const { return framebuffer_; }

  uint32_t getWidth() const { return width_; }

  uint32_t getHeight() const { return height_; }

  uint32_t getLayers() const { return layers_; }

private:
    std::shared_ptr<VkDriver> driver_;
    std::shared_ptr<RenderPass> render_pass_;
    std::shared_ptr<RenderTarget> render_target_;
    
    VkFramebuffer framebuffer_{VK_NULL_HANDLE};
    uint32_t width_{0};
    uint32_t height_{0};
    uint32_t layers_{0};
};
} // namespace vk_engine