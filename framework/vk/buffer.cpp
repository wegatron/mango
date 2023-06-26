
#include <framework/utils/error.h>
#include <framework/vk/buffer.h>
#include <framework/vk/vk_driver.h>

namespace vk_engine {
Buffer::Buffer(const std::shared_ptr<VkDriver> &driver,
               VkBufferCreateFlags flags, VkDeviceSize size,
               VkBufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage)
    : driver_(driver), flags_(flags), size_(size), buffer_usage_(buffer_usage),
      memory_usage_(memory_usage) {
  VkBufferCreateInfo buffer_create_info = {
      VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, // VkStructureType        sType;
      nullptr,                              // const void*            pNext;
      flags_,                               // VkBufferCreateFlags    flags;
      size_,                                // VkDeviceSize           size;
      buffer_usage_,                        // VkBufferUsageFlags     usage;
      VK_SHARING_MODE_EXCLUSIVE, // VkSharingMode          sharingMode;
      0,      // uint32_t               queueFamilyIndexCount;
      nullptr // const uint32_t*        pQueueFamilyIndices;
  };

  VmaAllocationCreateInfo alloc_create_info = {
      flags_,        // VmaAllocationCreateFlags  flags;
      memory_usage_, // VmaMemoryUsage            usage;
  };

  auto result =
      vmaCreateBuffer(driver_->getAllocator(), &buffer_create_info,
                      &alloc_create_info, &buffer_, &allocation_, nullptr);
  if (result != VK_SUCCESS) {
    throw VulkanException(result, "failed to create buffer!");
  }
}
} // namespace vk_engine