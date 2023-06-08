#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace vk_engine {
struct VertexInputState {
  std::vector<VkVertexInputBindingDescription> bindings;
  std::vector<VkVertexInputAttributeDescription> attributes;
};

struct InputAssemblyState {
  VkPrimitiveTopology topology{VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
  VkBool32 primitive_restart_enable{false};
};

struct RasterizationState {
  VkBool32 depth_clamp_enable{VK_FALSE};
  VkBool32 rasterizer_discard_enable{VK_FALSE};
  VkPolygonMode polygon_mode{VK_POLYGON_MODE_FILL};
  VkCullModeFlags cull_mode{VK_CULL_MODE_BACK_BIT};
  VkFrontFace front_face{VK_FRONT_FACE_CLOCKWISE};
  VkBool32 depth_bias_enable{VK_FALSE};
};

struct ViewPortState {
  uint32_t viewport_count{1};
  uint32_t scissor_count{1};
};

struct MultisampleState {
  VkSampleCountFlagBits rasterization_samples{VK_SAMPLE_COUNT_1_BIT};
  VkBool32 sample_shading_enable{VK_FALSE};
  float min_sample_shading{1.0f};
  VkSampleMask sample_mask{};
  VkBool32 alpha_to_coverage_enable{VK_FALSE};
  VkBool32 alpha_to_one_enable{VK_FALSE};
};

struct DepthStencilState {
  VkBool32 depth_test_enable{VK_TRUE};
  VkBool32 depth_write_enable{VK_TRUE};
  VkCompareOp depth_compare_op{VK_COMPARE_OP_LESS};
  VkBool32 depth_bounds_test_enable{VK_FALSE};
  VkBool32 stencil_test_enable{VK_FALSE};
  VkStencilOpState front{};
  VkStencilOpState back{};
  float min_depth_bounds{0.0f};
  float max_depth_bounds{1.0f};
};

struct ColorBlendState {
  VkBool32 logic_op_enable{VK_FALSE};
  VkLogicOp logic_op{VK_LOGIC_OP_COPY};
  std::vector<VkPipelineColorBlendAttachmentState> attachments;
  float blend_constants[4]{0.0f, 0.0f, 0.0f, 0.0f};
};

// dynamic pipeline state vulkan 1.0: viewport, scissor, line width, depth bias,
// blend constants... 1.3: depth test enable, depth write enable, depth compare
// op, depth bounds test enable...
class PipelineState final {
public:
  PipelineState() = default;

  ~PipelineState() = default;

  void setVertexInputState(const VertexInputState &state);

  void setInputAssemblyState(const InputAssemblyState &state);

  void setRasterizationState(const RasterizationState &state);

  void setViewportState(const ViewPortState &state);

  void setMultisampleState(const MultisampleState &state);

  void setDepthStencilState(const DepthStencilState &state);

  void setColorBlendState(const ColorBlendState &state);

  const VertexInputState &getVertexInputState() const;

  const InputAssemblyState &getInputAssemblyState() const;

  const RasterizationState &getRasterizationState() const;

  const ViewPortState &getViewportState() const;

  const MultisampleState &getMultisampleState() const;

  const DepthStencilState &getDepthStencilState() const;

  const ColorBlendState &getColorBlendState() const;

  bool isDirty() const { return dirty_; }

private:
  VertexInputState vertex_input_state_;

  InputAssemblyState input_assembly_state_;

  RasterizationState rasterization_state_;

  ViewPortState viewport_state_;

  MultisampleState multisample_state_;

  DepthStencilState depth_stencil_state_;

  ColorBlendState color_blend_state_;

  bool dirty_{false};
};
} // namespace vk_engine