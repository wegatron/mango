#include <framework/utils/error.h>
#include <framework/vk/image.h>

namespace vk_engine {

Image::Image(const std::shared_ptr<VkDriver> &driver, VkImageCreateFlags flags,
             VkFormat format, const VkExtent3D &extent,
             VkSampleCountFlagBits sample_count, VkImageUsageFlags image_usage,
             VmaMemoryUsage memory_usage)
    : driver_(driver), flags_(flags), format_(format), extent_(extent),
      sample_count_(sample_count), image_usage_(image_usage),
      memory_usage_(memory_usage) {
  VkImageCreateInfo image_info = {};
  image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.flags = flags;
  image_info.imageType = VK_IMAGE_TYPE_2D;
  image_info.format = format_;
  image_info.extent = extent_;
  image_info.mipLevels = 1;
  image_info.arrayLayers = 1;
  image_info.samples = sample_count_;
  image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  image_info.usage = image_usage_;
  image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  VmaAllocationCreateInfo alloc_create_info = {};
  alloc_create_info.usage = memory_usage;

  // used for for memory less attachment, like depth stencil
  if (image_usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) {
    alloc_create_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
  }

  auto result =
      vmaCreateImage(driver_->getAllocator(), &image_info, &alloc_create_info,
                     &image_, &allocation_, nullptr);
  if (result != VK_SUCCESS) {
    throw VulkanException(result, "failed to create image!");
  }
}

Image::~Image() {
  vmaDestroyImage(driver_->getAllocator(), image_, allocation_);
}


ImageView::ImageView(const std::shared_ptr<Image> &image,
                     VkImageViewType view_type, VkFormat format,
                     VkImageAspectFlags aspect_flags,
                     uint32_t base_mip_level, uint32_t base_array_layer,
                     uint32_t n_mip_levels, uint32_t n_array_layers) : driver_(image->getDriver())
{
  VkImageViewCreateInfo view_info = {};
  view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.image = image->getHandle();
  view_info.viewType = view_type;
  view_info.format = format;
  view_info.subresourceRange.aspectMask = aspect_flags;
  view_info.subresourceRange.baseMipLevel = base_mip_level;
  view_info.subresourceRange.levelCount = n_mip_levels;
  view_info.subresourceRange.baseArrayLayer = base_array_layer;
  view_info.subresourceRange.layerCount = n_array_layers;

  view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

#if !NDEBUG
  view_type_ = view_type;
  format_ = format;
  subresource_range_ = view_info.subresourceRange;
#endif

  auto result = vkCreateImageView(image->getDriver()->getDevice(), &view_info,
                                  nullptr, &image_view_);
  if (result != VK_SUCCESS) {
    throw VulkanException(result, "failed to create image view!");
  }
}

ImageView::ImageView(const std::shared_ptr<VkDriver> &driver, VkImage image, VkImageViewType view_type, VkFormat format,
          VkImageAspectFlags aspect_flags,
          uint32_t base_mip_level, uint32_t base_array_layer,
          uint32_t n_mip_levels, uint32_t n_array_layers) : driver_(driver) 
{
VkImageViewCreateInfo view_info = {};
  view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.image = image;
  view_info.viewType = view_type;
  view_info.format = format;
  view_info.subresourceRange.aspectMask = aspect_flags;
  view_info.subresourceRange.baseMipLevel = base_mip_level;
  view_info.subresourceRange.levelCount = n_mip_levels;
  view_info.subresourceRange.baseArrayLayer = base_array_layer;
  view_info.subresourceRange.layerCount = n_array_layers;

  view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

#if !NDEBUG
  view_type_ = view_type;
  format_ = format;
  subresource_range_ = view_info.subresourceRange;
#endif

  auto result = vkCreateImageView(driver->getDevice(), &view_info,
                                  nullptr, &image_view_);
  if (result != VK_SUCCESS) {
    throw VulkanException(result, "failed to create image view!");
  }  
}

ImageView::~ImageView() {
  vkDestroyImageView(driver_->getDevice(), image_view_, nullptr);
}

} // namespace vk_engine