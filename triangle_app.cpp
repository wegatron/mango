#include "triangle_app.h"
#include <memory>
#include <framework/vk/shader_module.h>
#include <framework/vk/vk_driver.h>
#include <framework/vk/pipeline_state.h>
#include <framework/vk/pipeline.h>
#include <framework/vk/render_pass.h>

namespace vk_engine {
void TriangleApp::tick(const float seconds) {
  // TODO render one frame
}

void TriangleApp::init(const std::shared_ptr<VkDriver> &driver) {
  /// prepare data
  // vertex data
  std::vector<float> vertex_data = {-0.5, -0.5, 0, 0, 0, 1, 0,   0,
                                    0.5,  -0.5, 0, 0, 0, 1, 1,   0,
                                    0.5,   0.5, 0, 0, 0, 1, 0.5, 0.5};

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

  // std::vector<Attachment> attachments{{VK_FORMAT_R8G8B8_SRGB, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR,
  //                                      VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
  //                                      VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
  //                                      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR}};
  // std::vector<LoadStoreInfo> load_store_infos;
  // std::vector<SubpassInfo> subpass_infos; 
  // auto render_pass = resource_cache_->requestRenderPass(driver, attachments, load_store_infos, subpass_infos);
}
} // namespace vk_engine