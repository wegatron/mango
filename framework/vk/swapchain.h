#pragma once

#include <vector>
#include <volk.h>

#include <framework/vk/vk_driver.h>

namespace vk_engine {

enum ImageFormat { sRGB, UNORM };

struct SwapchainProperties {
  VkExtent2D extent{};
  VkSurfaceFormatKHR surface_format{VK_FORMAT_B8G8R8A8_SRGB,
                                    VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};    
  VkSwapchainKHR old_swapchain{VK_NULL_HANDLE};
  uint32_t image_count{3};
  uint32_t array_layers{1};
  VkCompositeAlphaFlagBitsKHR composite_alpha{VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR};
  VkPresentModeKHR present_mode{VK_PRESENT_MODE_FIFO_KHR};
};

class Swapchain final {
public:
  Swapchain(const std::shared_ptr<VkDriver> &driver, VkSurfaceKHR surface,
            const SwapchainProperties &properties);

  ~Swapchain();

  Swapchain(const Swapchain &) = delete;
  Swapchain &operator=(const Swapchain &) = delete;

  VkSwapchainKHR getHandle() const { return swapchain_; }

  VkExtent2D getExtent() const { return extent_; }

  VkFormat getImageFormat() const { return image_format_; }

  uint32_t getImageCount() const { return image_count_; }

  VkImageView getImageView(uint32_t index) const { return image_views_[index]; }

private:
  void initSwapchain(const SwapchainProperties &properties);

  void initImages();

  std::shared_ptr<VkDriver> driver_;
  VkSurfaceKHR surface_;
  VkSwapchainKHR swapchain_;
  VkExtent2D extent_;
  VkFormat image_format_;
  uint32_t image_count_;
  std::vector<VkImage> images_;
  std::vector<VkImageView> image_views_;
};

} // namespace vk_engine