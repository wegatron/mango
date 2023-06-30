#include "triangle_app.h"
#include <memory>
#include <framework/vk/shader_module.h>
#include <framework/vk/vk_driver.h>
#include <framework/vk/pipeline_state.h>
#include <framework/vk/pipeline.h>
#include <framework/vk/swapchain.h>
#include <framework/vk/render_pass.h>


namespace vk_engine {
void TriangleApp::tick(const float seconds) {
  // TODO render one frame
}

void TriangleApp::init(const std::shared_ptr<VkDriver> &driver, VkFormat color_format, VkFormat ds_format) {
  driver_ = driver;

  /// vertex data
  std::vector<float> vertex_data = {-0.5, -0.5, 0, 0, 0, 1, 0,   0,
                                    0.5,  -0.5, 0, 0, 0, 1, 1,   0,
                                    0.5,   0.5, 0, 0, 0, 1, 0.5, 0.5};

  // use device local, and transfer by staging buffer is better
  vertex_buffer_ = std::make_shared<Buffer>(
    driver_, 0, sizeof(float) * 8 * 3,
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
    VMA_MEMORY_USAGE_AUTO_PREFER_HOST);

  vertex_buffer_->update(reinterpret_cast<uint8_t *>(vertex_data.data()),
                         sizeof(float) * 8 * 3);

  // load and compile shader
  auto vs = resource_cache_->requestShaderModule("shaders/triangle.vert");
  auto fs = resource_cache_->requestShaderModule("shaders/triangle.frag");

  // create pipeline
  PipelineState pipeline_state;
  pipeline_state.setShaders({vs, fs});
  
  VertexInputState vertex_input_state;
  vertex_input_state.bindings = {{0, sizeof(float) * 8, VK_VERTEX_INPUT_RATE_VERTEX}};
  vertex_input_state.attributes = {
      {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0}, // 3 floats position
      {1, 0, VK_FORMAT_R32G32B32_SFLOAT, 12}, // 3 floats normal
      {2, 0, VK_FORMAT_R32G32_SFLOAT, 24} // 2 floats uv
  };
  pipeline_state.setVertexInputState(vertex_input_state);
  pipeline_state.setInputAssemblyState({VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false});
  pipeline_state.setRasterizationState(
    RasterizationState{false, false, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, 
    VK_FRONT_FACE_CLOCKWISE, false});
  pipeline_state.setViewportState({1, 1});
  pipeline_state.setMultisampleState({VK_SAMPLE_COUNT_1_BIT, false, 1.0f, 0, false, false});
  pipeline_state.setDepthStencilState({true, true, VK_COMPARE_OP_LESS, false, false, {}, {}, 0, 1});
  pipeline_state.setSubpassIndex(0);

  std::vector<Attachment> attachments{
    Attachment{color_format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT},
    Attachment{ds_format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT}
  };
  std::vector<LoadStoreInfo> load_store_infos
  {
    {VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE},
    {VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE}
  };
  std::vector<SubpassInfo> subpass_infos
  {
    {{}, // no input attachment
     {0}, // color attachment index 
     {}, // no msaa
     1 // depth stencil attachment index
     }
  };
  render_pass_ = resource_cache_->requestRenderPass(driver, attachments, load_store_infos, subpass_infos);
  
}
} // namespace vk_engine