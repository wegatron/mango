#include "material.h"
#include <framework/utils/app_context.h>
#include <framework/vk/pipeline.h>
#include <framework/vk/resource_cache.h>

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

constexpr uint32_t MAX_MAT_DESC_SET = 100;

MatGpuResourcePool::MatGpuResourcePool(VkFormat color_format,
                                       VkFormat ds_format) {
  auto &driver = getDefaultAppContext().driver;
  VkDescriptorPoolSize pool_size[] = {
      {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
       .descriptorCount = MAX_MAT_DESC_SET * CONFIG_UNIFORM_BINDING_COUNT},
      {.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
       .descriptorCount = MAX_MAT_DESC_SET * TEXTURE_NUM_COUNT}};
  desc_pool_ = std::make_unique<DescriptorPool>(
      driver, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, &pool_size, 2,
      100);
  auto &rs_cache = getDefaultAppContext().resource_cache;
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
  default_render_pass_ = rs_cache->requestRenderPass(
      driver, attachments, load_store_infos, subpass_infos);
}

void MatGpuResourcePool::gc() {
  for (auto itr = used_mat_params_set_.begin();
       itr != used_mat_params_set_.end();) {
    if (itr->use_count() == 1) {
      free_mat_params_set_.push_back(*itr);
      itr = used_mat_params_set_.erase(itr);
    } else
      ++itr;
  }
}

std::shared_ptr<GraphicsPipeline> MatGpuResourcePool::requestGraphicsPipeline(
    const std::shared_ptr<Material> &mat) {
  auto itr = mat_pipelines_.find(mat->hashId());
  if (itr != mat_pipelines_.end()) {
    return itr->second;
  }

  // pipeline cache
  auto &driver = getDefaultAppContext().driver;
  auto &rs_cache = getDefaultAppContext().resource_cache;
  auto pipeline_state = std::make_unique<PipelineState>();
  mat->setPipelineState(*pipeline_state);
  // TODO other pipeline state
  //pipeline_state

  auto pipeline = std::make_shared<GraphicsPipeline>(
      driver, rs_cache, default_render_pass_, std::move(pipeline_state));
  mat_pipelines_.emplace(mat->hashId(), pipeline);
  return pipeline;
}

std::shared_ptr<DescriptorSet> MatGpuResourcePool::requestMatDescriptorSet(
    const std::shared_ptr<Material> &mat) {
  if (mat->mat_param_set_ != nullptr)
    return mat->mat_param_set_->desc_set;

  // find a free mat param set
  auto mat_id = mat->hashId();
  auto itr =
      std::find_if(free_mat_params_set_.begin(), free_mat_params_set_.end(),
                   [mat_id](const std::shared_ptr<MatParamsSet> &ps) {
                     return ps->mat_hash_id == mat_id;
                   });

  if (itr != free_mat_params_set_.end()) {
    mat->mat_param_set_ = *itr;
    used_mat_params_set_.push_back(*itr);
    free_mat_params_set_.erase(itr);
    return mat->mat_param_set_->desc_set;
  }

  // create new one
  auto &driver = getDefaultAppContext().driver;
  auto &rs_cache = getDefaultAppContext().resource_cache;
  auto mat_param_set = mat->createMatParamsSet(driver, *desc_pool_);
  mat->mat_param_set_ = mat_param_set;
  used_mat_params_set_.push_back(mat_param_set);
  return mat_param_set->desc_set;
}

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
  ShaderResource sr[] = {{
      .stages = VK_SHADER_STAGE_FRAGMENT_BIT,
      .type = ShaderResourceType::BufferUniform,
      .mode = ShaderResourceMode::Static,
      .set = MATERIAL_SET_INDEX,
      .binding = 0,
      .size = ubo_info_.size,
  }};
  desc_set_layout_ = std::make_unique<DescriptorSetLayout>(
      getDefaultAppContext().driver, MATERIAL_SET_INDEX, sr, 1);
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

std::shared_ptr<MatParamsSet>
PbrMaterial::createMatParamsSet(const std::shared_ptr<VkDriver> &driver,
                                DescriptorPool &desc_pool) {
  // create uniform buffer
  auto ubo =
      std::make_shared<Buffer>(driver, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                               ubo_info_.size);

  // create descriptor set
  auto desc_set = desc_pool.requestDescriptorSet(*desc_set_layout_);
  // update descriptor set
  VkDescriptorBufferInfo desc_buffer_info{
      .buffer = ubo->getHandle(),
      .offset = 0,
      .range = ubo_info_.size,
  };
  driver->update(
      {VkWriteDescriptorSet{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                            .dstSet = desc_set->getHandle(),
                            .dstBinding = 0,
                            .descriptorCount = 1,
                            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                            .pBufferInfo = &desc_buffer_info}});

  return std::make_shared<MatParamsSet>(hash_id_, ubo, desc_set);
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