#pragma once

#include <volk.h>

namespace vk_engine {
class PipelineLayout final {
public:
  ~PipelineLayout() = default;

  PipelineLayout(const PipelineLayout &) = delete;
  PipelineLayout &operator=(const PipelineLayout &) = delete;

  PipelineLayout(PipelineLayout &&) = delete;
  PipelineLayout &operator=(PipelineLayout &&) = delete;

  VkPipelineLayout getHandle() const { return handle_; }

private:
  VkPipelineLayout handle_{VK_NULL_HANDLE};
};
} // namespace vk_engine