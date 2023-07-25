#include <framework/utils/error.h>
#include <framework/vk/commands.h>
#include <framework/vk/frame_buffer.h>
#include <framework/vk/pipeline.h>

namespace vk_engine {
CommandPool::CommandPool(const std::shared_ptr<VkDriver> &driver,
                         uint32_t queue_family_index, CmbResetMode mode)
    : driver_(driver), mode_(mode) {
  VkCommandPoolCreateFlags flags[] = {
      VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      VK_COMMAND_POOL_CREATE_TRANSIENT_BIT};

  VkCommandPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.queueFamilyIndex = queue_family_index;
  pool_info.flags = flags[static_cast<int>(mode)];

  auto result = vkCreateCommandPool(driver->getDevice(), &pool_info, nullptr,
                                    &command_pool_);
  if (result != VK_SUCCESS) {
    throw VulkanException(result, "failed to create command pool!");
  }
}

CommandPool::~CommandPool() {
  for (auto &command_buffer : command_buffers_) {
    if (command_buffer.use_count() != 1) {
      throw VulkanException(
          VK_RESULT_MAX_ENUM,
          "command pool destroy with command buffer is still in use!");
    }
  }
  vkDestroyCommandPool(driver_->getDevice(), command_pool_, nullptr);
}

void CommandPool::reset(bool memory2system) {
  VkCommandPoolResetFlags flag =
      memory2system ? VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT : 0;
  vkResetCommandPool(driver_->getDevice(), command_pool_, flag);
}

std::shared_ptr<CommandBuffer>
CommandPool::requestCommandBuffer(VkCommandBufferLevel level) {
  std::shared_ptr<CommandBuffer> cmdb(new CommandBuffer(driver_, *this, level));
  command_buffers_.emplace_back(cmdb);
  return cmdb;
}

CommandBuffer::CommandBuffer(const std::shared_ptr<VkDriver> &driver,
                             CommandPool &command_pool,
                             VkCommandBufferLevel level)
    : driver_(driver), command_pool_(command_pool.getHandle()) {
#if !defined(NDEBUG)
  resetable_ =
      (command_pool.getResetMode() != CommandPool::CmbResetMode::ResetPool);
#endif

  VkCommandBufferAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = command_pool.getHandle();
  alloc_info.level = level;
  alloc_info.commandBufferCount = 1;

  auto result = vkAllocateCommandBuffers(driver_->getDevice(), &alloc_info,
                                         &command_buffer_);

  if (result != VK_SUCCESS) {
    throw VulkanException(result, "failed to allocate command buffers!");
  }
}

CommandBuffer::~CommandBuffer() {
  if (command_buffer_ != VK_NULL_HANDLE) {
    vkFreeCommandBuffers(driver_->getDevice(), command_pool_, 1,
                         &command_buffer_);
  }
}

void CommandBuffer::reset(bool memory2pool) {
#if !defined(NDEBUG)
  if (!resetable_) {
    throw VulkanException(VK_RESULT_MAX_ENUM,
                          "command buffer is not resetable!");
  }
#endif

  VkCommandBufferResetFlags flag =
      memory2pool ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0;
  vkResetCommandBuffer(command_buffer_, flag);
}

void CommandBuffer::begin(VkCommandBufferUsageFlags flags) {
  VkCommandBufferBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = flags;
  begin_info.pInheritanceInfo = nullptr;

  auto result = vkBeginCommandBuffer(command_buffer_, &begin_info);
  VK_THROW_IF_ERROR(result, "failed to begin recording command buffer!");
}

void CommandBuffer::end() {
  auto result = vkEndCommandBuffer(command_buffer_);
  VK_THROW_IF_ERROR(result, "failed to record command buffer!");
}

void CommandBuffer::beginRenderPass(
    const std::shared_ptr<Fence> &render_fence,
    const std::shared_ptr<Semaphore> &render_semaphore,
    const std::shared_ptr<RenderPass> &render_pass,
    const std::shared_ptr<FrameBuffer> &frame_buffer) {
  VkCommandBufferBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
  begin_info.pInheritanceInfo = nullptr; // primary command buffer

  auto result = vkBeginCommandBuffer(command_buffer_, &begin_info);
  VK_THROW_IF_ERROR(result, "failed to begin recording command buffer!");

  VkClearValue clear_values[2]; // 与render pass load store clear attachment对应
  clear_values[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
  clear_values[1].depthStencil = {1.0f, 0};

  VkRenderPassBeginInfo render_pass_begin_info = {};
  render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_begin_info.renderPass = render_pass->getHandle();
  render_pass_begin_info.framebuffer = frame_buffer->getHandle();
  render_pass_begin_info.renderArea.offset = {0, 0};
  render_pass_begin_info.renderArea.extent.width = frame_buffer->getWidth();
  render_pass_begin_info.renderArea.extent.height = frame_buffer->getHeight();
  render_pass_begin_info.clearValueCount = 2;
  render_pass_begin_info.pClearValues = clear_values;

  vkCmdBeginRenderPass(command_buffer_, &render_pass_begin_info,
                       VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::setViewPort(const std::vector<VkViewport> &viewports) {
  vkCmdSetViewport(command_buffer_, 0, viewports.size(), viewports.data());
}

void CommandBuffer::setScissor(const std::vector<VkRect2D> &scissors) {
  vkCmdSetScissor(command_buffer_, 0, scissors.size(), scissors.data());
}

void CommandBuffer::bindPipelineWithDescriptorSets(
    const std::shared_ptr<Pipeline> &pipeline,
    const std::initializer_list<std::shared_ptr<DescriptorSet>>
        &descriptor_sets,
    const std::initializer_list<uint32_t> &dynamic_offsets,
    const uint32_t first_set) {
  auto pipeline_bind_point = pipeline->getType() == Pipeline::Type::GRAPHICS
                                 ? VK_PIPELINE_BIND_POINT_GRAPHICS
                                 : VK_PIPELINE_BIND_POINT_COMPUTE;
  vkCmdBindPipeline(command_buffer_, pipeline_bind_point,
                    pipeline->getHandle());
  vkCmdBindDescriptorSets(
      command_buffer_, pipeline_bind_point,
      pipeline->getPipelineLayout()->getHandle(), first_set,
      descriptor_sets.size(),
      reinterpret_cast<const VkDescriptorSet *>(descriptor_sets.begin()),
      dynamic_offsets.size(), dynamic_offsets.begin());
}

void CommandBuffer::bindVertexBuffer(
    const std::initializer_list<std::shared_ptr<Buffer>> &buffer,
    const std::initializer_list<VkDeviceSize> &offsets,
    const uint32_t first_binding) {}

void CommandBuffer::bindIndexBuffer(const std::shared_ptr<Buffer> &buffer,
                                    const VkDeviceSize offset,
                                    const VkIndexType index_type) {}

void CommandBuffer::draw(const uint32_t vertex_count,
                         const uint32_t instance_count,
                         const uint32_t first_vertex,
                         const uint32_t first_instance) {}

void CommandBuffer::drawIndexed(const uint32_t index_count,
                                const uint32_t instance_count,
                                const uint32_t first_index,
                                const int32_t vertex_offset,
                                const uint32_t first_instance) {}

void CommandBuffer::endRenderPass() {}

void CommandBuffer::end() {}
} // namespace vk_engine