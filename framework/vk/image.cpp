#include <framework/utils/error.h>
#include <framework/vk/image.h>

namespace vk_engine {

Image::Image(const std::shared_ptr<VkDriver> &driver, VkFormat format,
             const VkExtent3D &extent, VkSampleCountFlagBits sample_count,
             VkImageUsageFlags image_usage, VmaMemoryUsage memory_usage)
    : driver_(driver), extent_(extent), format_(format),
      image_usage_(image_usage), sample_count_(sample_count) {
  VkImageCreateInfo image_info = {};
  image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
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

  VmaAllocationCreateInfo alloc_info = {};
  alloc_info.usage = memory_usage;

  // used for for memory less attachment, like depth stencil
  if (image_usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) {
    alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
  }

  auto result = vmaCreateImage(driver_->getAllocator(), &image_info,
                               &alloc_info, &image_, &memory_, nullptr);
  if (result != VK_SUCCESS) {
    throw VulkanException(result, "failed to create image!");
  }
}
} // namespace vk_engine