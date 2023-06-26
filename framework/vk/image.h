#pragma once

#include <framework/vk/vk_driver.h>
#include <memory>
#include <vk_mem_alloc.h>

namespace vk_engine {
class Image final {
public:
  Image(const std::shared_ptr<VkDriver> &driver, VkImageCreateFlags flags,
        VkFormat format, const VkExtent3D &extent,
        VkSampleCountFlagBits sample_count, VkImageUsageFlags image_usage,
        VmaMemoryUsage memory_usage);

  Image(const Image &) = delete;
  Image(Image &&) = delete;
  Image &operator=(const Image &) = delete;

  ~Image();

  VkImage getHandle() const { return image_; }

private:
  std::shared_ptr<VkDriver> driver_;
  VkImageCreateFlags flags_;
  VkFormat format_;
  VkExtent3D extent_;
  VkSampleCountFlagBits sample_count_;
  VkImageUsageFlags image_usage_;
  VmaMemoryUsage memory_usage_;

  VmaAllocation allocation_{VK_NULL_HANDLE};
  VkImage image_{VK_NULL_HANDLE};
};
} // namespace vk_engine