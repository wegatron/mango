#pragma once

#include <framework/vk/barriers.h>
#include <framework/vk/vk_driver.h>
#include <memory>

namespace vk_engine {
class CommandBuffer;
class RenderPass;
class FrameBuffer;
class Fence;
class Semaphore;
class Pipeline;
class Buffer;
class DescriptorSet;
class ImageView;

class CommandPool final {
public:
  // refer to:
  // https://arm-software.github.io/vulkan_best_practice_for_mobile_developers/samples/performance/command_buffer_usage/command_buffer_usage_tutorial.html#allocate-and-free
  enum class CmbResetMode {
    AlwaysAllocate = 0,
    ResetIndividually = 1, // reset command buffers individually
    ResetPool = 2
  };

  CommandPool(const std::shared_ptr<VkDriver> &driver,
              uint32_t queue_family_index, CmbResetMode mode);

  CommandPool(const CommandPool &) = delete;
  CommandPool &operator=(const CommandPool &) = delete;
  CommandPool(CommandPool &&) = delete;

  ~CommandPool();

  void reset(bool memory2system);

  std::shared_ptr<CommandBuffer>
  requestCommandBuffer(VkCommandBufferLevel level);

  VkCommandPool getHandle() const { return command_pool_; }

  CmbResetMode getResetMode() const { return mode_; }

private:
  std::shared_ptr<VkDriver> driver_;
  VkCommandPool command_pool_{VK_NULL_HANDLE};
  CmbResetMode mode_;
  std::vector<std::shared_ptr<CommandBuffer>> command_buffers_;
};

class CommandBuffer final {
public:
  CommandBuffer(const CommandBuffer &) = delete;
  CommandBuffer &operator=(const CommandBuffer &) = delete;
  CommandBuffer(CommandBuffer &&) = delete;

  ~CommandBuffer(); // free command buffer

  void reset(bool memory2pool);

  VkCommandBuffer getHandle() const { return command_buffer_; }

  void begin(VkCommandBufferUsageFlags flags);

  void end();

  void beginRenderPass(const std::shared_ptr<Fence> &render_fence,
                       const std::shared_ptr<Semaphore> &render_semaphore,
                       const std::shared_ptr<RenderPass> &render_pass,
                       const std::unique_ptr<FrameBuffer> &frame_buffer);

  void endRenderPass();

  void setViewPort(const std::initializer_list<VkViewport> &viewports);

  void setScissor(const std::initializer_list<VkRect2D> &scissors);

  void bindPipelineWithDescriptorSets(
      const std::shared_ptr<Pipeline> &pipeline,
      const std::initializer_list<std::shared_ptr<DescriptorSet>>
          &descriptor_sets,
      const std::initializer_list<uint32_t> &dynamic_offsets,
      const uint32_t first_set);

  void
  bindVertexBuffer(const std::initializer_list<std::shared_ptr<Buffer>> &buffer,
                   const std::initializer_list<VkDeviceSize> &offsets,
                   const uint32_t first_binding);

  void bindIndexBuffer(const std::shared_ptr<Buffer> &buffer,
                       const VkDeviceSize offset, const VkIndexType index_type);

  void draw(const uint32_t vertex_count, const uint32_t instance_count,
            const uint32_t first_vertex, const uint32_t first_instance);

  void drawIndexed(const uint32_t index_count, const uint32_t instance_count,
                   const uint32_t first_index, const int32_t vertex_offset,
                   const uint32_t first_instance);

  void imageMemoryBarrier(const ImageMemoryBarrier &image_memory_barrier,
                          const std::shared_ptr<ImageView> &image_view);

private:
  CommandBuffer(const std::shared_ptr<VkDriver> &driver,
                CommandPool &command_pool, VkCommandBufferLevel level);
  std::shared_ptr<VkDriver> driver_;
  VkCommandPool command_pool_;
  VkCommandBuffer command_buffer_{VK_NULL_HANDLE};
#ifndef NDEBUG
  bool resetable_;
#endif

  friend class CommandPool;
};
} // namespace vk_engine