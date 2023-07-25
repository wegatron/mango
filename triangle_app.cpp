#include "triangle_app.h"
#include <framework/vk/pipeline.h>
#include <framework/vk/pipeline_state.h>
#include <framework/vk/render_pass.h>
#include <framework/vk/shader_module.h>
#include <framework/vk/swapchain.h>
#include <framework/vk/syncs.h>
#include <framework/vk/vk_driver.h>
#include <memory>

namespace vk_engine {
void TriangleApp::tick(const float seconds, const uint32_t render_target_index,
                       const uint32_t frame_index) {
  render_output_syncs_[frame_index].render_fence->wait();
  // update command buffer if needed
  // update data if needed
  // TODO submit command buffer
}

void TriangleApp::init(const std::shared_ptr<VkDriver> &driver,
                       const std::vector<std::shared_ptr<RenderTarget>> &rts) {
  driver_ = driver;
  frames_inflight_ = rts.size();
  if(resource_cache_->getPipelineCache() == nullptr)
  {
     auto pcw = std::make_unique<VkPipelineCacheWraper>(driver_->getDevice());
     resource_cache_->setPipelineCache(std::move(pcw));
  }

  setupScene();

  assert(rts.size() > 0 && rts[0] != nullptr);

  setupRender(rts[0]->getColorFormat(0), rts[0]->getDSFormat());

  setupFramebuffers(rts);
  
  render_output_syncs_.resize(frames_inflight_);
  for (auto &sync : render_output_syncs_) {
    sync.render_fence = std::make_shared<Fence>(driver_);
    sync.render_semaphore = std::make_shared<Semaphore>(driver_);
    sync.present_semaphore = std::make_shared<Semaphore>(driver_);
  }

  buildCommandBuffers();
}

void TriangleApp::setupScene() {
  /// vertex data
  std::vector<float> vertex_data = {-0.5, -0.5, 0, 0, 0, 1, 0,   0,
                                    0.5,  -0.5, 0, 0, 0, 1, 1,   0,
                                    0.5,  0.5,  0, 0, 0, 1, 0.5, 0.5};

  // use device local, and transfer by staging buffer is better
  vertex_buffer_ = std::make_shared<Buffer>(
      driver_, 0, sizeof(float) * 8 * 3, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VMA_ALLOCATION_CREATE_MAPPED_BIT |
          VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
      VMA_MEMORY_USAGE_AUTO_PREFER_HOST);

  vertex_buffer_->update(reinterpret_cast<uint8_t *>(vertex_data.data()),
                         sizeof(float) * 8 * 3);

  frame_data_.resize(frames_inflight_);
  // uniform buffer
  for(auto &fd : frame_data_)
  {
    fd.uniform_buffer = std::make_unique<Buffer>(
        driver_, 0, sizeof(float) * 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_ALLOCATION_CREATE_MAPPED_BIT |
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_HOST);
  }
}

void TriangleApp::setupFramebuffers(const std::vector<std::shared_ptr<RenderTarget>> &rts)
{
  for (uint32_t i = 0; i < frames_inflight_; ++i) {    
    frame_data_[i].frame_buffer = std::make_unique<FrameBuffer>(
        driver_, render_pass_, rts[i]);
  }
}

void TriangleApp::setupRender(VkFormat color_format, VkFormat ds_format) {
  // load and compile shader
  auto vs = resource_cache_->requestShaderModule("shaders/triangle.vert");
  auto fs = resource_cache_->requestShaderModule("shaders/triangle.frag");

  // create pipeline
  std::unique_ptr<PipelineState> pipeline_state =
      std::make_unique<PipelineState>();
  pipeline_state->setShaders({vs, fs});

  VertexInputState vertex_input_state;
  vertex_input_state.bindings = {
      {0, sizeof(float) * 8, VK_VERTEX_INPUT_RATE_VERTEX}};
  vertex_input_state.attributes = {
      {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},  // 3 floats position
      {1, 0, VK_FORMAT_R32G32B32_SFLOAT, 12}, // 3 floats normal
      {2, 0, VK_FORMAT_R32G32_SFLOAT, 24}     // 2 floats uv
  };
  pipeline_state->setVertexInputState(vertex_input_state);
  pipeline_state->setInputAssemblyState(
      {VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false});
  pipeline_state->setRasterizationState(RasterizationState{
      false, false, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT,
      VK_FRONT_FACE_CLOCKWISE, false});
  pipeline_state->setViewportState({1, 1});
  pipeline_state->setMultisampleState(
      {VK_SAMPLE_COUNT_1_BIT, false, 1.0f, 0, false, false});
  pipeline_state->setDepthStencilState(
      {true, true, VK_COMPARE_OP_LESS, false, false, {}, {}, 0, 1});

  pipeline_state->setColorBlendState(ColorBlendState{
    .attachments = {
      {
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
      }
    }
  });
  pipeline_state->setSubpassIndex(0);

  std::vector<Attachment> attachments{
      Attachment{color_format, VK_SAMPLE_COUNT_1_BIT,
                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT},
      Attachment{ds_format, VK_SAMPLE_COUNT_1_BIT,
                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT}};
  std::vector<LoadStoreInfo> load_store_infos{
      {VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE},
      {VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE}};
  std::vector<SubpassInfo> subpass_infos{{
      {},  // no input attachment
      {0}, // color attachment index
      {},  // no msaa
      1    // depth stencil attachment index
  }};
  render_pass_ = resource_cache_->requestRenderPass(
      driver_, attachments, load_store_infos, subpass_infos);
  pipeline_ = std::make_shared<GraphicsPipeline>(
      driver_, resource_cache_, render_pass_, std::move(pipeline_state));

  std::vector<VkDescriptorPoolSize> pool_size = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1*frames_inflight_},
  };
  descriptor_pool_ = std::make_shared<DescriptorPool>(driver_, pool_size, frames_inflight_);
  const auto &ds_layout = pipeline_->getPipelineLayout()->getDescriptorSetLayout(0);
  
  for(auto &frame_data : frame_data_)
  {
    frame_data.descriptor_set = descriptor_pool_->requestDescriptorSet(ds_layout);
    VkDescriptorBufferInfo desc_buffer_info{
      .buffer = frame_data.uniform_buffer->getHandle(),
      .offset = 0,
      .range = sizeof(float) * 32,
    };
    frame_data.descriptor_set->update({
      {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = frame_data.descriptor_set->getHandle(),
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &desc_buffer_info
      }
    });
  }
}

void TriangleApp::buildCommandBuffers() {
  // init command pool
  command_pool_ = std::make_unique<CommandPool>(
      driver_, driver_->getGraphicsQueueFamilyIndex(), CommandPool::CmbResetMode::ResetPool);
  // build command buffers
  for (uint32_t i = 0; i < frames_inflight_; ++i) {
    auto &frame_data = frame_data_[i];
    auto &sync = render_output_syncs_[i];    
    frame_data.cmd_buffer = command_pool_->requestCommandBuffer(
        VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    auto cmd_buf = frame_data.cmd_buffer;
    cmd_buf->begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    cmd_buf->beginRenderPass(
      sync.render_fence,
      sync.render_semaphore,
      render_pass_, frame_data.frame_buffer);
    cmd_buf->bindPipelineWithDescriptorSets(
        pipeline_, {frame_data.descriptor_set}, {}, 0);    
    cmd_buf->bindVertexBuffer({vertex_buffer_}, {0}, 0);
    uint32_t width = frame_data_[0].frame_buffer->getWidth();
    uint32_t height = frame_data_[0].frame_buffer->getHeight();
    cmd_buf->setViewPort({VkViewport{0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.f, 1.f}});
    cmd_buf->setScissor({VkRect2D{{0, 0}, {width, height}}});
    cmd_buf->draw(3, 1, 0, 0);
    cmd_buf->endRenderPass();
    cmd_buf->end();
  }
}

} // namespace vk_engine