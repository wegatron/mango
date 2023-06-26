#pragma once

#include <memory>
#include <volk.h>
#include <vk_mem_alloc.h>

namespace vk_engine {
class VkDriver;
class Buffer final 
{
public:
  Buffer(const std::shared_ptr<VkDriver> &driver, VkBufferCreateFlags flags,
         VkDeviceSize size, VkBufferUsageFlags buffer_usage,
         VmaMemoryUsage memory_usage);
  ~Buffer();
  
  Buffer(const Buffer &) = delete;
  Buffer(Buffer &&) = delete;
  Buffer &operator=(const Buffer &) = delete;

  VkBuffer getHandle() const { return buffer_; }
private:
  std::shared_ptr<VkDriver> driver_;
  
  VkBuffer buffer_{VK_NULL_HANDLE};
  VmaAllocation allocation_{VK_NULL_HANDLE};

  VkBufferCreateFlags flags_;
  VkDeviceSize size_;
  VkBufferUsageFlags buffer_usage_;
  VmaMemoryUsage memory_usage_;
};
} // namespace vk_engine