#include <framework/utils/error.h>
#include <framework/vk/swapchain.h>
#include <framework/utils/error.h>

namespace vk_engine {
Swapchain::Swapchain(const std::shared_ptr<VkDriver> &driver,
                     VkSurfaceKHR surface,
                     const SwapchainProperties &properties)
    : driver_(driver), surface_(surface) {
  initSwapchain(properties);
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

  auto old_swapchain = swapchain_;
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
  swapchain_info.oldSwapchain = old_swapchain;
  
  // vkCreateSwapchainKHR maybe null if not enable VK_KHR_swapchain
  // make sure eanbleExtensionCount is correct when create logical device
  // refer to:
  // https://stackoverflow.com/questions/55131406/why-would-vkcreateswapchainkhr-result-in-an-access-violation-at-0
  VK_THROW_IF_ERROR(vkCreateSwapchainKHR(driver_->getDevice(), &swapchain_info,
                                         nullptr, &swapchain_), "vulkan failed to create swapchain");
  

  if(old_swapchain != VK_NULL_HANDLE) {
    image_views_.clear();
    vkDestroySwapchainKHR(driver_->getDevice(), old_swapchain, nullptr);
  }

  initImages();
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
    image_views_[i] = std::make_shared<ImageView>(driver_, images_[i], VK_IMAGE_VIEW_TYPE_2D, image_format_, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1, 1);  
  }
}

Swapchain::~Swapchain()
{
  #if !NDEBUG
  for (auto &image_view : image_views_) {
    assert(image_view.use_count() == 1);
  }
  #endif
  image_views_.clear();
  vkDestroySwapchainKHR(driver_->getDevice(), swapchain_, nullptr);  
}

uint32_t Swapchain::acquireNextImage(VkSemaphore semaphore, VkFence fence)
{
  uint32_t image_index;
  vkAcquireNextImageKHR(driver_->getDevice(), swapchain_, UINT64_MAX, semaphore, fence, &image_index);
  return image_index;
}

} // namespace vk_engine