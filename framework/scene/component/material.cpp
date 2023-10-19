#include "material.h"
#include <iostream>

namespace vk_engine {

#define MAX_FORWARD_LIGHT_COUNT 4

#define HAS_BASE_COLOR_TEXTURE "HAS_BASE_COLOR_TEXTURE"
#define HAS_METALLIC_TEXTURE "HAS_METALLIC_TEXTURE"
#define HAS_ROUGHNESS_TEXTURE "HAS_ROUGHNESS_TEXTURE"
#define HAS_NORMAL_TEXTURE "HAS_NORMAL_TEXTURE"

#define BASE_COLOR_TEXTURE_NAME "base_color_texture"
#define METALLIC_TEXTURE_NAME "metallic_texture"
#define NORMAL_TEXTURE_NAME "normal_texture"

enum PbrTextureParamIndex {
  BASE_COLOR_TEXTURE_INDEX = 0,
  METALLIC_TEXTURE_INDEX = 1,
  NORMAL_TEXTURE_INDEX = 2,
  TEXTURE_NUM_COUNT
};

PbrMaterial::PbrMaterial() {
  hash_id_ = typeid(PbrMaterial).hash_code();
  ubo_info_ = MaterialUboInfo{.set = MATERIAL_SET_INDEX,
                           .binding = 0,
                           .size = 32,
                           .data = std::vector<std::byte>(32, std::byte{0}),
                           .params{
                               {.stride = 0,
                                .tinfo = typeid(glm::vec4),
                                .ub_offset = 0,
                                .name = "pbr_mat.base_color"},
                               {0, typeid(glm::vec2), sizeof(glm::vec4),
                                "pbr_mat.metallic_roughness"},
                           }};
  // // uniform buffer information
  // ubos_.emplace_back(globalMVPUbo());

  // // lights
  // ubos_.emplace_back(MaterialUbo{
  //     .set = 0,
  //     .binding = 1,
  //     .size = 64 * MAX_FORWARD_LIGHT_COUNT + 16,
  //     .params{
  //         {0, typeid(uint32_t), 0, "lights.count"},
  //         {64, typeid(glm::vec4), 16, "lights.position"},
  //         {64, typeid(glm::vec4), 16 + sizeof(glm::vec4), "lights.color"},
  //         {64, typeid(glm::vec4), 16 + sizeof(glm::vec4) * 2,
  //          "lights.direction"},
  //         {64, typeid(glm::vec2), 16 + sizeof(glm::vec4) * 3,
  //         "lights.info"}}});

  // pbr material
  // texture_params_.resize(TEXTURE_NUM_COUNT);
  // texture_params_[BASE_COLOR_TEXTURE_INDEX] =
  //   MaterialTextureParam{ // 0 for
  //     .set = 2,
  //     .binding = 0,
  //     .index = 0,
  //     .name = BASE_COLOR_TEXTURE_NAME,
  //     .img_file_path = "",
  //     .dirty = true
  //   };
  // unable to check because of variance
  //   // check uniform buffer in resources is consistent with ubos_
  // #ifndef NDEBUG
  //   for (auto &resource : shader_resources_) {
  //     if (resource.type == ShaderResourceType::BufferUniform) {
  //       if (resource.set != MATERIAL_SET_INDEX)
  //         continue; // only for material ubo
  //       auto itr = std::find_if(
  //           ubos_.begin(), ubos_.end(), [&resource](const MaterialUbo &ubo) {
  //             return ubo.set == resource.set && ubo.binding ==
  //             resource.binding;
  //           });
  //       if (itr == ubos_.end())
  //         throw std::runtime_error("uniform buffer not found");
  //       if (itr->size < resource.size)
  //         throw std::runtime_error("uniform buffer size not match");
  //     }
  //   }
  // #endif
}

void PbrMaterial::compile() {
  ShaderVariant variant;

  // // shader variance
  // if(!texture_params_[BASE_COLOR_TEXTURE_INDEX].img_file_path.empty())
  // {
  //   variant.addDefine(HAS_BASE_COLOR_TEXTURE);
  // }

  vs_ = std::make_shared<ShaderModule>(variant);
  vs_->load("shaders/basic.vert");

  fs_ = std::make_shared<ShaderModule>(variant);
  fs_->load("shaders/basic.frag");

  // create uniform buffers

  // shader_resources_ = parseShaderResources({vs_, fs_});
}

bool Material::updateParams() {
  // update uniform buffer params
  assert(mat_param_set_ != nullptr);
  auto &ubo = mat_param_set_->ubo;
  if (ubo_info_.dirty) {
    ubo->update(ubo_info_.data.data(), ubo_info_.size, 0);
    ubo_info_.dirty = false;
  }

  // update textures... descriptor set
  return true;
}

void PbrMaterial::setPipelineState(PipelineState &pipeline_state) {
  pipeline_state.setShaders({vs_, fs_});
  pipeline_state.setMultisampleState(
      {VK_SAMPLE_COUNT_1_BIT, false, 0.0f, 0xFFFFFFFF, false, false});
  pipeline_state.setSubpassIndex(0);
}

// void Material::writeDescriptorSets(
//     VkDescriptorSet descriptor_set) {
//   std::vector<VkWriteDescriptorSet> wds;
//   wds.reserve(ubos_.size());
//   std::vector<VkDescriptorBufferInfo> desc_buffer_infos;
//   desc_buffer_infos.reserve(ubos_.size());
//   for (auto i = 0; i < ubos_.size(); ++i) {
//     desc_buffer_infos.emplace_back(VkDescriptorBufferInfo{
//         .buffer = ubos_[i]->getHandle(),
//         .offset = 0,
//         .range = ubos_info_[i].size,
//     });
//     wds.emplace_back(VkWriteDescriptorSet{
//         .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
//         .dstSet = descriptor_set,
//         .dstBinding = ubos_info_[i].binding,
//         .descriptorCount = 1,
//         .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//         .pBufferInfo = &desc_buffer_infos.back(),
//     });
//   }

//   if (wds.empty())
//     throw std::runtime_error("no ubo to update");

//   driver_->update(wds);
// }

} // namespace vk_engine