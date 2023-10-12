#include "viewer_app.h"
#include <framework/scene/loader.h>
#include <framework/utils/app_context.h>
#include <framework/vk/stage_pool.h>
#include <framework/scene/asset_manager.hpp>
#include <framework/vk/queue.h>

namespace vk_engine {

static AppContext g_app_context;

const AppContext &getDefaultAppContext() { return g_app_context; }

void ViewerApp::init(const std::shared_ptr<VkDriver> &driver,
                     const std::vector<std::shared_ptr<RenderTarget>> &rts) {
  context_.resource_cache = std::make_shared<ResourceCache>();
  context_.driver = driver;
  auto &frames_data = context_.frames_data;
  frames_data.resize(rts.size());
  
  if (context_.resource_cache->getPipelineCache() == nullptr) {
    auto pcw = std::make_unique<VkPipelineCacheWraper>(driver->getDevice());
    context_.resource_cache->setPipelineCache(std::move(pcw));
  }
  context_.stage_pool = std::make_shared<StagePool>(driver);

  auto cmd_queue = driver->getGraphicsQueue();
  
  for(auto i=0; i<rts.size(); ++i)
  {
    frames_data[i].command_pool = std::make_unique<CommandPool>(
      driver, cmd_queue->getFamilyIndex(),
      CommandPool::CmbResetMode::ResetPool);
    frames_data[i].render_tgt = rts[i];
  }

  g_app_context = context_; // set default app context

  auto cmd_buf =
      frames_data[0].command_pool->requestCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  cmd_buf->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  AssimpLoader loader;
  loader.loadScene(scene_path_, *scene_, cmd_buf);
  cmd_buf->end();
  cmd_queue->submit(cmd_buf, VK_NULL_HANDLE);
}

void ViewerApp::setScene(const std::string &path) { scene_path_ = path; }

void ViewerApp::tick(const float seconds, const uint32_t rt_index,
                     const uint32_t frame_index) {
  context_.stage_pool->gc();
  context_.gpu_asset_manager->gc();
  render_.beginFrame(frame_index, rt_index);
  render_.render(scene_.get());
  render_.endFrame();
}
} // namespace vk_engine