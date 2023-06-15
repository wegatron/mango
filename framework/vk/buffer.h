#pragma once

#include <memory>
#include <volk.h>

namespace vk_engine {
class VkDriver;
class Buffer {
  Buffer(std::shared_ptr<VkDriver> &driver, VkDeviceSize size,
         VkBufferUsageFlags buffer_usage);
};
} // namespace vk_engine