#include <framework/utils/error.h>
#include <framework/vk/swapchain.h>

namespace vk_engine {
Swapchain::Swapchain(const std::shared_ptr<VkDriver> &driver,
                     VkSurfaceKHR surface,
                     const SwapchainProperties &properties)
    : driver_(driver), surface_(surface) {
  initSwapchain(properties);
  initImages();
}

void Swapchain::initSwapchain(const SwapchainProperties &properties) {
  // check surface capabilities
  VkSurfaceCapabilitiesKHR surface_capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(driver_->getPhysicalDevice(),
                                            surface_, &surface_capabilities);
  extent_.width = std::clamp(properties.extent.width,
                             surface_capabilities.minImageExtent.width,
                             surface_capabilities.maxImageExtent.width);
  extent_.height = std::clamp(properties.extent.height,
                              surface_capabilities.minImageExtent.height,
                              surface_capabilities.maxImageExtent.height);
  image_count_ =
      std::clamp(properties.image_count, surface_capabilities.minImageCount,
                 surface_capabilities.maxImageCount);
  image_format_ = properties.surface_format.format;

  // create swapchain
  VkSwapchainCreateInfoKHR swapchain_info{};
  swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchain_info.surface = surface_;
  swapchain_info.imageExtent = extent_;
  swapchain_info.minImageCount = image_count_;

  // imageFormat specifies what the image format is (same as it does in vkCreateImage()).
  // imageColorSpace is how the swapchain\display interprets the values when the image is presented.
  swapchain_info.imageFormat = properties.surface_format.format;
  swapchain_info.imageColorSpace = properties.surface_format.colorSpace;
  swapchain_info.imageArrayLayers = 1;
  swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchain_info.preTransform = surface_capabilities.currentTransform;
  swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchain_info.clipped = VK_TRUE;
  swapchain_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
  swapchain_info.oldSwapchain = swapchain_;

  // vkCreateSwapchainKHR maybe null if not enable VK_KHR_swapchain
  // make sure eanbleExtensionCount is correct when create logical device
  // refer to:
  // https://stackoverflow.com/questions/55131406/why-would-vkcreateswapchainkhr-result-in-an-access-violation-at-0
  VK_THROW_IF_ERROR(vkCreateSwapchainKHR(driver_->getDevice(), &swapchain_info,
                                         nullptr, &swapchain_));
}

void Swapchain::initImages() {
  // get swapchain images
  uint32_t image_count;
  vkGetSwapchainImagesKHR(driver_->getDevice(), swapchain_, &image_count,
                          nullptr);
  images_.resize(image_count);
  vkGetSwapchainImagesKHR(driver_->getDevice(), swapchain_, &image_count,
                          images_.data());

  // image views
  image_views_.resize(images_.size());
  VkImageViewCreateInfo image_view_info{};
  image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  image_view_info.format = image_format_;
  image_view_info.subresourceRange.baseMipLevel = 0;
  image_view_info.subresourceRange.levelCount = 1;
  image_view_info.subresourceRange.baseArrayLayer = 0;
  image_view_info.subresourceRange.layerCount = 1;
  image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  image_view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  image_view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  image_view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  image_view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

  for (size_t i = 0; i < images_.size(); ++i) {
    image_view_info.image = images_[i];
    VK_THROW_IF_ERROR(vkCreateImageView(driver_->getDevice(), &image_view_info,
                                        nullptr, &image_views_[i]));
  }
}

Swapchain::~Swapchain()
{
  for (auto &image_view : image_views_) {
    vkDestroyImageView(driver_->getDevice(), image_view, nullptr);
  }
  vkDestroySwapchainKHR(driver_->getDevice(), swapchain_, nullptr);  
}

} // namespace vk_engine