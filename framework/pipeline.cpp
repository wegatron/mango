#include <framework/pipeline.h>
#include <framework/resource_cache.h>

namespace vk_engine {
GraphicsPipeline::GraphicsPipeline(
    const std::shared_ptr<VkDriver> &driver,
    const std::shared_ptr<ResourceCache> &cache,
    std::unique_ptr<PipelineState> &&pipeline_state)
    : pipeline_state_(std::move(pipeline_state)), driver_(driver) {
  assert(cache != nullptr);
  auto &shader_modules = pipeline_state_->getShaderModules();
  pipeline_layout_ = cache->requestPipelineLayout(driver, shader_modules);

  std::vector<std::shared_ptr<Shader>> shaders(shader_modules.size());
  std::vector<VkPipelineShaderStageCreateInfo> shader_stages(
      shader_modules.size());
  for (auto i = 0; i < shader_modules.size(); ++i) {
    shaders[i] = cache->requestShader(driver, shader_modules[i]);
    shader_stages[i].sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[i].pNext = nullptr;
    shader_stages[i].flags = 0;
    shader_stages[i].stage = shader_modules[i]->getStage();
    shader_stages[i].module = shaders[i]->getHandle();
    shader_stages[i].pName = "main";
    shader_stages[i].pSpecializationInfo = nullptr;
  }

  VkGraphicsPipelineCreateInfo pipeline_info{};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount = shader_stages.size();
  pipeline_info.pStages = shader_stages.data();

  VkPipelineVertexInputStateCreateInfo vertex_input_state = {};
  pipeline_state_->getVertexInputState().getCreateInfo(vertex_input_state);
  pipeline_info.pVertexInputState = &vertex_input_state;

  VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {};
  pipeline_state_->getInputAssemblyState().getCreateInfo(input_assembly_state);
  pipeline_info.pInputAssemblyState = &input_assembly_state;

  VkPipelineViewportStateCreateInfo viewport_state{
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
  viewport_state.viewportCount =
      pipeline_state_->getViewportState().viewport_count;
  viewport_state.scissorCount =
      pipeline_state_->getViewportState().scissor_count;
  pipeline_info.pViewportState = &viewport_state;

  VkPipelineRasterizationStateCreateInfo rasterization_state{
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
  pipeline_state_->getRasterizationState().getCreateInfo(rasterization_state);
  pipeline_info.pRasterizationState = &rasterization_state;

  VkPipelineMultisampleStateCreateInfo multisample_state{
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
  pipeline_state_->getMultisampleState().getCreateInfo(multisample_state);
  pipeline_info.pMultisampleState = &multisample_state;

  VkPipelineDepthStencilStateCreateInfo depth_stencil_state{
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
  pipeline_state_->getDepthStencilState().getCreateInfo(depth_stencil_state);
  pipeline_info.pDepthStencilState = &depth_stencil_state;

  VkPipelineColorBlendStateCreateInfo color_blend_state{
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
  pipeline_state_->getColorBlendState().getCreateInfo(color_blend_state);
  pipeline_info.pColorBlendState = &color_blend_state;

  VkPipelineDynamicStateCreateInfo dynamic_state{
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
  pipeline_state_->getDynamicStateCreateInfo(dynamic_state);
  pipeline_info.pDynamicState = &dynamic_state;

  auto pipeline_layout = cache->requestPipelineLayout(driver, shader_modules);
  pipeline_info.layout = pipeline_layout->getHandle();
  //   pipeline_info.renderPass = pipeline_state_->render_pass->getHandle();
  //   pipeline_info.subpass = pipeline_state_->subpass;
  //   pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
  //   pipeline_info.basePipelineIndex = -1;

  assert(cache->getPipelineCache() != nullptr); // pipeline cache should always exist
  if (vkCreateGraphicsPipelines(driver->getDevice(), cache->getPipelineCache(),
                                1, &pipeline_info, nullptr,
                                &pipeline_) != VK_SUCCESS)
    throw std::runtime_error("failed to create graphics pipeline!");

  cleanDirtyFlag();
}
} // namespace vk_engine