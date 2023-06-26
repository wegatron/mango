#pragma once

#include <framework/vk/vk_driver.h>
#include <memory>
#include <vk_mem_alloc.h>

namespace vk_engine {
class Image final {
public:
  Image(const std::shared_ptr<VkDriver> &driver, VkFormat format,
        const VkExtent3D &extent, VkSampleCountFlagBits sample_count,
        VkImageUsageFlags image_usage, VmaMemoryUsage memory_usage);

  Image(const Image &) = delete;
  Image(Image &&) = delete;
  Image &operator=(const Image &) = delete;

  ~Image();

  VkImage getHandle() const { return image_; }

private:
  std::shared_ptr<VkDriver> driver_;
  VmaAllocation memory_{VK_NULL_HANDLE};
  VkExtent3D extent_;
  VkFormat format_;
  VkImageUsageFlags image_usage_;
  VkSampleCountFlagBits sample_count_;
  VkImage image_{VK_NULL_HANDLE};
};
} // namespace vk_engine