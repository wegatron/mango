#pragma once

#include <framework/vk/vk_driver.h>
#include <memory>

namespace vk_engine {
class Image final {
public:
  Image(std::shared_ptr<VkDriver> driver, const VkExtent3D &extent,
        VkFormat format, VkImageUsageFlags image_usage,
        VkSampleCountFlagBits sample_count);

  Image(const Image &) = delete;
  Image(Image &&) = delete;
  Image &operator=(const Image &) = delete;  

  ~Image();

  VkImage getHandle() const { return image_; }
private:
  std::shared_ptr<VkDriver> driver_;
  VkExtent3D extent_;
  VkFormat format_;
  VkImageUsageFlags image_usage_;
  VkSampleCountFlagBits sample_count_;
  VkImage image_{VK_NULL_HANDLE};
};
} // namespace vk_engine