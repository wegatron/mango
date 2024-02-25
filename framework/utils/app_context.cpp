#include <framework/scene/asset_manager.hpp>
#include <framework/utils/app_context.h>
#include <framework/vk/commands.h>
#include <framework/vk/descriptor_set.h>
#include <framework/vk/queue.h>
#include <framework/vk/resource_cache.h>
#include <framework/vk/stage_pool.h>
#include <framework/vk/sampler.h>
#include <framework/vk/image.h>
#include <framework/vk/syncs.h>
#include <framework/vk/vk_constants.h>
#include <framework/scene/component/light.h>
#include <framework/utils/ltc_matrix.hpp>
namespace vk_engine {
static AppContext g_app_context;

const AppContext &getDefaultAppContext() { return g_app_context; }

void destroyDefaultAppContext() { g_app_context.destroy(); }

void updateRtsInContext(const std::vector<std::shared_ptr<RenderTarget>> &rts) {
  auto &frames_data = g_app_context.frames_data;
  assert(rts.size() == frames_data.size());
  for (auto i = 0; i < rts.size(); ++i) {
    frames_data[i].render_tgt = rts[i];
  }
}

bool initAppContext(const std::shared_ptr<VkDriver> &driver,
                    const std::vector<std::shared_ptr<RenderTarget>> &rts) {
  g_app_context.resource_cache = std::make_shared<ResourceCache>();
  g_app_context.driver = driver;
  if (g_app_context.resource_cache->getPipelineCache() == nullptr) {
    auto pcw = std::make_unique<VkPipelineCacheWraper>(driver->getDevice());
    g_app_context.resource_cache->setPipelineCache(std::move(pcw));
  }
  g_app_context.stage_pool = std::make_shared<StagePool>(driver);

  // gpu asset manager
  g_app_context.gpu_asset_manager = std::make_shared<GPUAssetManager>();

  // descriptor pool
  VkDescriptorPoolSize pool_size[] = {
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
       MAX_GLOBAL_DESC_SET * CONFIG_UNIFORM_BINDING_COUNT},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
       MAX_GLOBAL_DESC_SET * MAX_TEXTURE_NUM_COUNT},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
       MAX_GLOBAL_DESC_SET * CONFIG_STORAGE_BINDING_COUNT}};
  g_app_context.descriptor_pool = std::make_unique<DescriptorPool>(
      driver, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, pool_size,
      sizeof(pool_size) / sizeof(pool_size[0]), MAX_GLOBAL_DESC_SET);

  // frames data
  auto cmd_queue = driver->getGraphicsQueue();
  auto &frames_data = g_app_context.frames_data;
  frames_data.resize(rts.size());
  g_app_context.render_output_syncs.resize(rts.size());
  for (auto i = 0; i < rts.size(); ++i) {
    frames_data[i].command_pool =
        std::make_unique<CommandPool>(driver, cmd_queue->getFamilyIndex(),
                                      CommandPool::CmbResetMode::ResetPool);
    frames_data[i].render_tgt = rts[i];
    auto &sync = g_app_context.render_output_syncs[i];
    sync.render_fence = std::make_shared<Fence>(driver, true);
    sync.render_semaphore = std::make_shared<Semaphore>(driver);
    sync.present_semaphore = std::make_shared<Semaphore>(driver);
  }
  return true;
}

void initGlobalParamSet(const std::shared_ptr<CommandBuffer> &cmd_buf)
{
  g_app_context.global_param_set = std::move(std::make_unique<GlobalParamSet>(cmd_buf));
}

GlobalParamSet::GlobalParamSet(const std::shared_ptr<CommandBuffer> &cmd_buf) {
  static_assert(sizeof(ub_data_) == GLOBAL_UBO_SIZE);
  auto driver = getDefaultAppContext().driver;
  ubo_ = std::move(std::make_unique<Buffer>(
      driver, 0, GLOBAL_UBO_SIZE, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VMA_ALLOCATION_CREATE_MAPPED_BIT |
          VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
      VMA_MEMORY_USAGE_AUTO_PREFER_HOST));
  
  ub_data_ = {
    .ev = 0.65f,
    .view  = Eigen::Matrix4f::Identity(),
    .proj = Eigen::Matrix4f::Identity(),
    .lights_count = 0
  };

  ubo_->update(&ub_data_, GLOBAL_UBO_CAMERA_SIZE);

  ShaderResource sr[] = {
    {
      .stages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      .type = ShaderResourceType::BufferUniform,
      .mode = ShaderResourceMode::Static,
      .set = GLOBAL_SET_INDEX,
      .binding = 0,
      .array_size = 1,
      .size = GLOBAL_UBO_SIZE,
    },
    {
      .stages = VK_SHADER_STAGE_FRAGMENT_BIT,
      .type = ShaderResourceType::ImageSampler,
      .mode = ShaderResourceMode::Static,
      .set = GLOBAL_SET_INDEX,
      .binding = 1
    },
    {
      .stages = VK_SHADER_STAGE_FRAGMENT_BIT,
      .type = ShaderResourceType::ImageSampler,
      .mode = ShaderResourceMode::Static,
      .set = GLOBAL_SET_INDEX,
      .binding = 2
    }    
  };
  DescriptorSetLayout desc_layout(getDefaultAppContext().driver,
                                  MATERIAL_SET_INDEX, sr, sizeof(sr)/sizeof(ShaderResource));
  auto &desc_pool = getDefaultAppContext().descriptor_pool;
  desc_set_ = desc_pool->requestDescriptorSet(desc_layout);

  // LTC texture
  auto &asset_manager = getDefaultAppContext().gpu_asset_manager;
  ltc1_imgv_ = asset_manager->request<ImageView>(LTC1, 64, 64, 4, cmd_buf);
  ltc2_imgv_ = asset_manager->request<ImageView>(LTC2, 64, 64, 4, cmd_buf);
  sampler_ = getDefaultAppContext().resource_cache->requestSampler(driver, 
    VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST,
    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
  
  VkDescriptorImageInfo ltc_img_infos[2]{
    {
      .sampler = sampler_->getHandle(),
      .imageView = ltc1_imgv_->getHandle(),
      .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    },
    {
      .sampler = sampler_->getHandle(),
      .imageView = ltc2_imgv_->getHandle(),
      .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    }
  };

  VkDescriptorBufferInfo desc_buffer_info{
      .buffer = ubo_->getHandle(), .offset = 0, .range = GLOBAL_UBO_SIZE};
  driver->update(
      {
        VkWriteDescriptorSet{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                            .dstSet = desc_set_->getHandle(),
                            .dstBinding = 0,
                            .descriptorCount = 1,
                            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                            .pBufferInfo = &desc_buffer_info},
        VkWriteDescriptorSet{
          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .dstSet = desc_set_->getHandle(),
          .dstBinding = 1,
          .descriptorCount = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .pImageInfo = &ltc_img_infos[0]
        },
        VkWriteDescriptorSet{
          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .dstSet = desc_set_->getHandle(),
          .dstBinding = 2,
          .descriptorCount = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .pImageInfo = &ltc_img_infos[1]
        }
      });
}

void GlobalParamSet::setCameraParam(const Eigen::Vector3f &pos, const float ev100, const Eigen::Matrix4f &view, const Eigen::Matrix4f &proj) {
  ub_data_.cam_pos = pos;
  ub_data_.ev = 0.65f*pow(2.0f, ev100);
  ub_data_.view = view;
  ub_data_.proj = proj;
}

void GlobalParamSet::setLights(const Lights &lights)
{
  static_assert(sizeof(Light) == 112);  
  static_assert(sizeof(Lights) == 112 * MAX_LIGHTS_COUNT + 16);
  memcpy(&(ub_data_.lights), &lights, sizeof(Lights));
}

void GlobalParamSet::update()
{
  ubo_->update(&ub_data_, sizeof(ub_data_));
}
} // namespace vk_engine