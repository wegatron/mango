#include "material.h"
#include <framework/utils/app_context.h>
#include <framework/vk/buffer.h>
#include <framework/vk/pipeline.h>
#include <framework/vk/resource_cache.h>
#include <framework/vk/image.h>
#include <framework/vk/sampler.h>

namespace vk_engine {

#define MAX_FORWARD_LIGHT_COUNT 4

#define HAS_BASE_COLOR_TEXTURE "HAS_BASE_COLOR_TEXTURE"
#define HAS_METALLIC_TEXTURE "HAS_METALLIC_TEXTURE"
#define HAS_ROUGHNESS_TEXTURE "HAS_ROUGHNESS_TEXTURE"
#define HAS_NORMAL_TEXTURE "HAS_NORMAL_TEXTURE"

#define HAS_BASE_COLOR_TEXTURE_VARIANT 1
#define HAS_METALLIC_TEXTURE_VARIANT 1<<1
#define HAS_ROUGHNESS_TEXTURE_VARIANT 1<<2
#define HAS_NORMAL_TEXTURE_VARIANT 1<<3

enum PbrTextureParamIndex {
  BASE_COLOR_TEXTURE_INDEX = 0,
  METALLIC_TEXTURE_INDEX = 1,
  NORMAL_TEXTURE_INDEX = 2,
  MAT_TEXTURE_NUM_COUNT
};

MatGpuResourcePool::MatGpuResourcePool(VkFormat color_format,
                                       VkFormat ds_format) {
  auto &driver = getDefaultAppContext().driver;
  VkDescriptorPoolSize pool_size[] = {
      {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
       .descriptorCount = MAX_MAT_DESC_SET * CONFIG_UNIFORM_BINDING_COUNT},
      {.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
       .descriptorCount = MAX_MAT_DESC_SET * MAT_TEXTURE_NUM_COUNT}};
  desc_pool_ = std::make_unique<DescriptorPool>(
      driver, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, pool_size,
      sizeof(pool_size) / sizeof(VkDescriptorPoolSize), MAX_MAT_DESC_SET);
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
  auto itr = mat_pipelines_.find(mat->materialTypeId());
  if (itr != mat_pipelines_.end()) {
    return itr->second;
  }

  // pipeline cache
  auto &driver = getDefaultAppContext().driver;
  auto &rs_cache = getDefaultAppContext().resource_cache;
  auto pipeline_state = std::make_unique<PipelineState>();
  mat->setPipelineState(*pipeline_state);
  // set other pipeline state:

  auto pipeline = std::make_shared<GraphicsPipeline>(
      driver, rs_cache, default_render_pass_, std::move(pipeline_state));
  mat_pipelines_.emplace(mat->materialTypeId(), pipeline);
  return pipeline;
}

std::shared_ptr<DescriptorSet> MatGpuResourcePool::requestMatDescriptorSet(
    const std::shared_ptr<Material> &mat) {
  if (mat->mat_param_set_ != nullptr)
    return mat->mat_param_set_->desc_set;

  // find a free mat param set
  auto mat_id = mat->materialTypeId();
  auto itr =
      std::find_if(free_mat_params_set_.begin(), free_mat_params_set_.end(),
                   [mat_id](const std::shared_ptr<MatParamsSet> &ps) {
                     return ps->mat_type_id == mat_id;
                   });

  if (itr != free_mat_params_set_.end()) {
    mat->mat_param_set_ = *itr;
    used_mat_params_set_.push_back(*itr);
    free_mat_params_set_.erase(itr);
    mat->updateParams(); // update mat paramsto gpu
    return mat->mat_param_set_->desc_set;
  }

  // create new one
  auto &driver = getDefaultAppContext().driver;
  auto &rs_cache = getDefaultAppContext().resource_cache;
  auto mat_param_set = mat->createMatParamsSet(driver, *desc_pool_);
  mat->mat_param_set_ = mat_param_set;
  used_mat_params_set_.push_back(mat_param_set);
  mat->updateParams(); // update mat paramsto gpu
  return mat_param_set->desc_set;
}

PbrMaterial::PbrMaterial() {
  // all candidate inputs
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
  texture_params_ = {
    {
      .set = MATERIAL_SET_INDEX,
      .binding = 1,
      .index = 0,
      .name = BASE_COLOR_TEXTURE_NAME,
      .img_view = nullptr,
      .dirty = false
    }
  };
}

void PbrMaterial::compile() {
  ShaderVariant variant;  

  material_type_id_ = PBR_MATERIAL;

  uint32_t sr_cnt = 0;
  ShaderResource sr[2];
  sr[sr_cnt++] = {
      .stages = VK_SHADER_STAGE_FRAGMENT_BIT,
      .type = ShaderResourceType::BufferUniform,
      .mode = ShaderResourceMode::Static,
      .set = MATERIAL_SET_INDEX,
      .binding = 0,
      .array_size = 1,
      .size = ubo_info_.size,
  };

  // shader variance
  if(texture_params_[BASE_COLOR_TEXTURE_INDEX].img_view != nullptr)
  {
    variant.addDefine(HAS_BASE_COLOR_TEXTURE);
    material_type_id_ |= HAS_BASE_COLOR_TEXTURE_VARIANT;
    sr[sr_cnt++] = {
      .stages = VK_SHADER_STAGE_FRAGMENT_BIT,
      .type = ShaderResourceType::ImageSampler,
      .mode = ShaderResourceMode::Static,
      .set = MATERIAL_SET_INDEX,
      .binding = 1,
      .array_size = 1
    };
  }

  desc_set_layout_ = std::make_unique<DescriptorSetLayout>(
      getDefaultAppContext().driver, MATERIAL_SET_INDEX, sr, sr_cnt);

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
  auto ret = std::make_shared<MatParamsSet>();
  ret->mat_type_id = material_type_id_;
  // create uniform buffer
  ret->ubo = std::make_unique<Buffer>(
      driver, 0, ubo_info_.size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VMA_ALLOCATION_CREATE_MAPPED_BIT |
          VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
      VMA_MEMORY_USAGE_AUTO_PREFER_HOST);

  // create descriptor set
  ret->desc_set = desc_pool.requestDescriptorSet(*desc_set_layout_);
  // update descriptor set
  VkDescriptorBufferInfo desc_buffer_info{
      .buffer = ret->ubo->getHandle(),
      .offset = 0,
      .range = ubo_info_.size,
  };
  driver->update(
      {VkWriteDescriptorSet{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                            .dstSet = ret->desc_set->getHandle(),
                            .dstBinding = 0,
                            .descriptorCount = 1,
                            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                            .pBufferInfo = &desc_buffer_info}});

  return ret;
}

void Material::updateParams() {
  // update uniform buffer params
  if (mat_param_set_ == nullptr)
    return;
  auto &ubo = mat_param_set_->ubo;
  if (ubo_info_.dirty) {
    ubo->update(ubo_info_.data.data(), ubo_info_.size, 0);
    ubo_info_.dirty = false;
  }

  // update textures... descriptor set
  assert(mat_param_set_->desc_set != nullptr);

  std::vector<VkDescriptorImageInfo> desc_img_infos;
  desc_img_infos.reserve(texture_params_.size());

  std::vector<VkWriteDescriptorSet> wds;
  wds.reserve(texture_params_.size());
  for (auto &tp : texture_params_) {
    if(!tp.dirty) continue;
    tp.dirty = false;
    // update descriptor set
    auto driver = getDefaultAppContext().driver;
    // save sampler to texture params. to make sure sampler not deconstruct when use
    tp.sampler = getDefaultAppContext().resource_cache->requestSampler(driver, 
      VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR,
      VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT);
    
    desc_img_infos.emplace_back(VkDescriptorImageInfo{
        .sampler = tp.sampler->getHandle(),
        .imageView = tp.img_view->getHandle(),
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    });
    wds.emplace_back(VkWriteDescriptorSet{
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = mat_param_set_->desc_set->getHandle(),
      .dstBinding = tp.binding,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .pImageInfo = &desc_img_infos.back()});
  }
  if(!wds.empty()) getDefaultAppContext().driver->update(wds);  
}

void PbrMaterial::setPipelineState(PipelineState &pipeline_state) {
  VertexInputState vertex_input_state{
      {// bindings, 3 float pos + 3 float normal + 2 float uv
       {0, 8 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX}},
      {                                                       // attribute
       {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},                 // 3floats pos
       {1, 0, VK_FORMAT_R32G32B32_SFLOAT, 3 * sizeof(float)}, // 3floats normal
       {2, 0, VK_FORMAT_R32G32_SFLOAT, 6 * sizeof(float)}}};  // 2 floats uv
  pipeline_state.setVertexInputState(vertex_input_state);
  pipeline_state.setInputAssemblyState(
      {VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false});
  RasterizationState rasterize{.depth_clamp_enable = false,
                               .rasterizer_discard_enable = false,
                               .polygon_mode = VK_POLYGON_MODE_FILL,
                               //.cull_mode = VK_CULL_MODE_BACK_BIT,
                               .cull_mode = VK_CULL_MODE_NONE,
                               .front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                               .depth_bias_enable = false};
  pipeline_state.setRasterizationState(rasterize);
  pipeline_state.setShaders({vs_, fs_});
  pipeline_state.setMultisampleState(
      {VK_SAMPLE_COUNT_1_BIT, false, 0.0f, 0xFFFFFFFF, false, false});
  // default depth stencil state, depth test enable, depth write enable, depth
  ColorBlendState color_blend_st{
      .attachments = {{
          .blendEnable = VK_FALSE,
          .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                            VK_COLOR_COMPONENT_G_BIT |
                            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
      }}};
  pipeline_state.setColorBlendState(color_blend_st);

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