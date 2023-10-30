#include <framework/scene/asset_manager.hpp>
#include <framework/utils/app_context.h>
#include <framework/vk/commands.h>
#include <framework/vk/queue.h>
#include <framework/vk/resource_cache.h>
#include <framework/vk/stage_pool.h>
#include <framework/vk/syncs.h>
#include <framework/vk/vk_constants.h>
#include <framework/vk/descriptor_set.h>

namespace vk_engine {
static AppContext g_app_context;

const AppContext &getDefaultAppContext() { return g_app_context; }

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

  // global param set
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

GlobalParamSet::GlobalParamSet() {
  auto driver = getDefaultAppContext().driver;
  constexpr uint32_t ubo_size = 64;
  ubo_ = std::move(std::make_unique<Buffer>(
      driver, 0, ubo_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VMA_ALLOCATION_CREATE_MAPPED_BIT |
          VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
      VMA_MEMORY_USAGE_AUTO_PREFER_HOST));
  
  Eigen::Matrix4f identity = Eigen::Matrix4f::Identity();
  ubo_->update(&identity, sizeof(identity));
  
  ShaderResource sr[] = {{
      .stages = VK_SHADER_STAGE_FRAGMENT_BIT,
      .type = ShaderResourceType::BufferUniform,
      .mode = ShaderResourceMode::Static,
      .set = GLOBAL_SET_INDEX,
      .binding = 0,
      .array_size = 1,
      .size = ubo_size,
  }};
  DescriptorSetLayout desc_layout(getDefaultAppContext().driver, MATERIAL_SET_INDEX, sr, 1);
  auto &desc_pool = getDefaultAppContext().descriptor_pool;
  desc_set_ = desc_pool->requestDescriptorSet(desc_layout);
}
} // namespace vk_engine