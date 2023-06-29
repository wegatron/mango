
#include <framework/utils/error.h>
#include <framework/vk/buffer.h>
#include <framework/vk/vk_driver.h>

namespace vk_engine {
Buffer::Buffer(const std::shared_ptr<VkDriver> &driver,
               VkBufferCreateFlags flags, VkDeviceSize size,
               VkBufferUsageFlags buffer_usage,
               VmaAllocationCreateFlags allocation_flags,
               VmaMemoryUsage memory_usage)
    : driver_(driver), flags_(flags), size_(size), buffer_usage_(buffer_usage),
      allocation_flags_(allocation_flags), memory_usage_(memory_usage) {
  persistent_ = (allocation_flags & VMA_ALLOCATION_CREATE_MAPPED_BIT) != 0;

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
      allocation_flags_,        // VmaAllocationCreateFlags  flags;
      memory_usage_, // VmaMemoryUsage            usage;
  };

  VmaAllocationInfo allocation_info{};
  auto result = vmaCreateBuffer(driver_->getAllocator(), &buffer_create_info,
                                &alloc_create_info, &buffer_, &allocation_,
                                &allocation_info);
  if (result != VK_SUCCESS) {
    throw VulkanException(result, "failed to create buffer!");
  }

  if (persistent_) {
    mapped_data_ = static_cast<uint8_t *>(allocation_info.pMappedData);
    mapped_ = true;
  }
}

Buffer::~Buffer() {
  vmaDestroyBuffer(driver_->getAllocator(), buffer_, allocation_);
}

void Buffer::update(uint8_t *data, size_t size, size_t offset) {
  if (persistent_) {
    std::copy(data, data + size, mapped_data_ + offset);
    flush();
  } else {
    map();
    std::copy(data, data + size, mapped_data_ + offset);
    flush();
    unmap();
  }
}

void Buffer::flush() {
  vmaFlushAllocation(driver_->getAllocator(), allocation_, 0, size_);
}

void Buffer::map() {
  if (!mapped_) {
    auto result = vmaMapMemory(driver_->getAllocator(), allocation_,
                               reinterpret_cast<void **>(&mapped_data_));
    if (result != VK_SUCCESS) {
      throw VulkanException(result, "failed to map buffer memory!");
    }
    mapped_ = true;
  }
}

void Buffer::unmap() {
  if (mapped_) {
    vmaUnmapMemory(driver_->getAllocator(), allocation_);
    mapped_ = false;
  }
}

} // namespace vk_engine