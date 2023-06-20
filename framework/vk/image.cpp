#include <framework/vk/image.h>

namespace vk_engine {

Image::Image(std::shared_ptr<VkDriver> driver, const VkExtent3D &extent,
             VkFormat format, VkImageUsageFlags image_usage,
             VkSampleCountFlagBits sample_count)
    : driver_(driver), extent_(extent), format_(format),
      image_usage_(image_usage), sample_count_(sample_count) {
  {
    
  }
} // namespace vk_engine