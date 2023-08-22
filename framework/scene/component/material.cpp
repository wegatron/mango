#include "material.h"
#include <iostream>

namespace vk_engine {

#define MAX_FORWARD_LIGHT_COUNT 4

PbrMaterial::PbrMaterial() {
  vs_ = std::make_shared<ShaderModule>();
  vs_->load("shaders/pbr.vert");

  fs_ = std::make_shared<ShaderModule>();
  fs_->load("shaders/pbr.frag");

  shader_resources_ = parseShaderResources({vs_, fs_});

  // uniform buffer information
  ubos_.emplace_back(globalMVPUbo());

  // lights
  ubos_.emplace_back(MaterialUbo{
      .set = 0,
      .binding = 1,
      .size = 64 * MAX_FORWARD_LIGHT_COUNT + 16,
      .params{
          {0, typeid(uint32_t), 0, "lights.count"},
          {64, typeid(glm::vec4), 16, "lights.position"},
          {64, typeid(glm::vec4), 16 + sizeof(glm::vec4), "lights.color"},
          {64, typeid(glm::vec4), 16 + sizeof(glm::vec4) * 2,
           "lights.direction"},
          {64, typeid(glm::vec2), 16 + sizeof(glm::vec4) * 3, "lights.info"}}});

  // pbr material
  ubos_.emplace_back(MaterialUbo{
      .set = 2,
      .binding = 0,
      .size = 32,
      .params{
          {0, typeid(glm::vec4), 0, "pbr_mat.base_color"},
          {0, typeid(float), sizeof(glm::vec4), "pbr_mat.metallic"},
          {0, typeid(float), sizeof(glm::vec4) + sizeof(float),
           "pbr_mat.roughness"}}});

  // update reference cpu data
  uint32_t ubo_data_size = 0;
  for (auto &ubo : ubos_) {
    ubo_data_size += ubo.size;
  }
  ubo_data_.resize(ubo_data_size);

  // check uniform buffer in resources is consistent with ubos_ 
}

void PbrMaterial::update2PipelineState(PipelineState &pipeline_state) {}

void PbrMaterial::updateParam2DescriptorSet(
    std::vector<VkWriteDescriptorSet> &write_desc_sets) {}

MaterialUbo globalMVPUbo() {
  static_assert(sizeof(glm::mat4) * 2 + sizeof(glm::vec4) == 64 * 2 + 16,
                "ubo size error");
  MaterialUbo ubo{
      .set = 0,
      .binding = 0,
      .size = 64 * 2 + 16,
      .params{
          {0, typeid(glm::mat4), 0, "mvp.model"},
          {0, typeid(glm::mat4), sizeof(glm::mat4), "mvp.view_proj"},
          {0, typeid(glm::vec3), sizeof(glm::mat4) * 2, "mvp.camera_position"}}};
  return ubo;
}

} // namespace vk_engine