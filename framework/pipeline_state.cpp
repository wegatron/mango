#include "pipeline_state.h"
#include <tuple>

bool operator==(const VkVertexInputBindingDescription &lhs,
                const VkVertexInputBindingDescription &rhs) {
  return lhs.binding == rhs.binding && lhs.stride == rhs.stride &&
         lhs.inputRate == rhs.inputRate;
}

bool operator==(const VkVertexInputAttributeDescription &lhs,
                const VkVertexInputAttributeDescription &rhs) {
  return lhs.binding == rhs.binding &&
         lhs.location == rhs.location & lhs.format == rhs.format &&
         lhs.offset == rhs.offset;
}

bool operator==(const VkPipelineColorBlendAttachmentState &lhs,
                const VkPipelineColorBlendAttachmentState &rhs) {
  return lhs.blendEnable == rhs.blendEnable &&
         lhs.srcColorBlendFactor == rhs.srcColorBlendFactor &&
         lhs.dstColorBlendFactor == rhs.dstColorBlendFactor &&
         lhs.colorBlendOp == rhs.colorBlendOp &&
         lhs.srcAlphaBlendFactor == rhs.srcAlphaBlendFactor &&
         lhs.dstAlphaBlendFactor == rhs.dstAlphaBlendFactor &&
         lhs.alphaBlendOp == rhs.alphaBlendOp &&
         lhs.colorWriteMask == rhs.colorWriteMask;
}

namespace vk_engine {
bool operator!=(const VkPipelineColorBlendAttachmentState &lhs,
                const VkPipelineColorBlendAttachmentState &rhs) {
  return lhs.blendEnable != rhs.blendEnable ||
         lhs.srcColorBlendFactor != rhs.srcColorBlendFactor ||
         lhs.dstColorBlendFactor != rhs.dstColorBlendFactor ||
         lhs.colorBlendOp != rhs.colorBlendOp ||
         lhs.srcAlphaBlendFactor != rhs.srcAlphaBlendFactor ||
         lhs.dstAlphaBlendFactor != rhs.dstAlphaBlendFactor ||
         lhs.alphaBlendOp != rhs.alphaBlendOp ||
         lhs.colorWriteMask != rhs.colorWriteMask;
}

bool operator!=(const VkStencilOpState &lhs, const VkStencilOpState &rhs) {
  return lhs.failOp != rhs.failOp || lhs.passOp != rhs.passOp ||
         lhs.depthFailOp != rhs.depthFailOp || lhs.compareOp != rhs.compareOp ||
         lhs.compareMask != rhs.compareMask || lhs.writeMask != rhs.writeMask ||
         lhs.reference != rhs.reference;
}

bool operator!=(const VertexInputState &lhs, const VertexInputState &rhs) {
  return lhs.bindings != rhs.bindings || lhs.attributes != rhs.attributes;
}

bool operator!=(const InputAssemblyState &lhs, const InputAssemblyState &rhs) {
  return lhs.topology != rhs.topology ||
         lhs.primitive_restart_enable != rhs.primitive_restart_enable;
}

bool operator!=(const RasterizationState &lhs, const RasterizationState &rhs) {
  return lhs.depth_clamp_enable != rhs.depth_clamp_enable ||
         lhs.rasterizer_discard_enable != rhs.rasterizer_discard_enable ||
         lhs.polygon_mode != rhs.polygon_mode ||
         lhs.cull_mode != rhs.cull_mode || lhs.front_face != rhs.front_face ||
         lhs.depth_bias_enable != rhs.depth_bias_enable;
}

bool operator!=(const ViewPortState &lhs, const ViewPortState &rhs) {
  return lhs.viewport_count != rhs.viewport_count ||
         lhs.scissor_count != rhs.scissor_count;
}

bool operator!=(const MultisampleState &lhs, const MultisampleState &rhs) {
  return lhs.rasterization_samples != rhs.rasterization_samples ||
         lhs.sample_shading_enable != rhs.sample_shading_enable ||
         lhs.min_sample_shading != rhs.min_sample_shading ||
         lhs.sample_mask != rhs.sample_mask ||
         lhs.alpha_to_coverage_enable != rhs.alpha_to_coverage_enable ||
         lhs.alpha_to_one_enable != rhs.alpha_to_one_enable;
}

bool operator!=(const DepthStencilState &lhs, const DepthStencilState &rhs) {
  return lhs.depth_test_enable != rhs.depth_test_enable ||
         lhs.depth_write_enable != rhs.depth_write_enable ||
         lhs.depth_compare_op != rhs.depth_compare_op ||
         lhs.depth_bounds_test_enable != rhs.depth_bounds_test_enable ||
         lhs.stencil_test_enable != rhs.stencil_test_enable ||
         lhs.front != rhs.front || lhs.back != rhs.back ||
         lhs.min_depth_bounds != rhs.min_depth_bounds ||
         lhs.max_depth_bounds != rhs.max_depth_bounds;
}

bool operator!=(const ColorBlendState &lhs, const ColorBlendState &rhs) {
  return lhs.logic_op_enable != rhs.logic_op_enable ||
         lhs.logic_op != rhs.logic_op || lhs.attachments != rhs.attachments ||
         lhs.blend_constants != rhs.blend_constants;
}

void PipelineState::setVertexInputState(const VertexInputState &state) {
  if (state != vertex_input_state_) {
    vertex_input_state_ = state;
    dirty_ = true;
  }
}
} // namespace vk_engine