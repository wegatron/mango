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
  for(auto i=0; i<images_.size(); ++i) {
    image_views_[i] = std::make_shared<ImageView>(driver_, images_[i], VK_IMAGE_VIEW_TYPE_2D, image_format_, 0, 0, 1, 1);  
  }
}

Swapchain::~Swapchain()
{
  for (auto &image_view : image_views_) {
    assert(image_view.use_count() == 1);
    image_view.reset();
    //vkDestroyImageView(driver_->getDevice(), image_view, nullptr);
  }
  vkDestroySwapchainKHR(driver_->getDevice(), swapchain_, nullptr);  
}

} // namespace vk_engine