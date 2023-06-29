#pragma once

#include <memory>
#include <vk_mem_alloc.h>
#include <volk.h>

namespace vk_engine {
class VkDriver;
class Buffer final {
public:
  Buffer(const std::shared_ptr<VkDriver> &driver, VkBufferCreateFlags flags,
         VkDeviceSize size, VkBufferUsageFlags buffer_usage,
         VmaAllocationCreateFlags allocation_flags,
         VmaMemoryUsage memory_usage);
  ~Buffer();

  Buffer(const Buffer &) = delete;
  Buffer(Buffer &&) = delete;
  Buffer &operator=(const Buffer &) = delete;

  VkBuffer getHandle() const { return buffer_; }

  void update(uint8_t *data, size_t size, size_t offset=0);

private:

  void flush();

  void map();

  void unmap();

  std::shared_ptr<VkDriver> driver_;

  VkBuffer buffer_{VK_NULL_HANDLE};
  VmaAllocation allocation_{VK_NULL_HANDLE};

  bool mapped_{false};
  uint8_t *mapped_data_{nullptr};

  bool persistent_;
  VkBufferCreateFlags flags_;
  VkDeviceSize size_;
  VkBufferUsageFlags buffer_usage_;

  VmaAllocationCreateFlags allocation_flags_;
  VmaMemoryUsage memory_usage_;  
};
} // namespace vk_engine