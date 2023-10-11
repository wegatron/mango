#include "viewer_app.h"
#include <framework/scene/loader.h>
#include <framework/utils/app_context.h>
#include <framework/vk/stage_pool.h>
#include <framework/scene/asset_manager.hpp>

namespace vk_engine {

static AppContext g_app_context;

const AppContext &getDefaultAppContext() { return g_app_context; }

void ViewerApp::init(const std::shared_ptr<VkDriver> &driver,
                     const std::vector<std::shared_ptr<RenderTarget>> &rts) {
  context_.resource_cache = std::make_shared<ResourceCache>();
  context_.driver = driver;
  rts_ = rts;
  if (context_.resource_cache->getPipelineCache() == nullptr) {
    auto pcw = std::make_unique<VkPipelineCacheWraper>(driver->getDevice());
    context_.resource_cache->setPipelineCache(std::move(pcw));
  }
  context_.stage_pool = std::make_shared<StagePool>(driver);
  context_.command_pool = std::make_unique<CommandPool>(
      driver, driver->getGraphicsQueueFamilyIndex(),
      CommandPool::CmbResetMode::ResetPool);

  g_app_context = context_; // set default app context

  auto cmd_buf =
      context_.command_pool->requestCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  cmd_buf->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  AssimpLoader loader;
  loader.loadScene(scene_path_, *scene_, cmd_buf);
  cmd_buf->end();
  
  // submit
  auto cmd_buf_handle = cmd_buf->getHandle();
  VkSubmitInfo info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
  info.commandBufferCount = 1;
  info.pCommandBuffers = &cmd_buf_handle;
  info.waitSemaphoreCount = 1;
  info.pWaitSemaphores = NULL;
  info.pWaitDstStageMask = NULL;
  info.signalSemaphoreCount = 1;
  info.pSignalSemaphores = NULL;
  // Submit command buffer to graphics queue
  auto result = vkQueueSubmit(
      driver->getGraphicsQueue(), 1, &info, NULL);
}

void ViewerApp::setScene(const std::string &path) { scene_path_ = path; }

void ViewerApp::tick(const float seconds, const uint32_t rt_index,
                     const uint32_t frame_index) {
  context_.stage_pool->gc();                      
  context_.gpu_asset_manager->gc();
  render_.beginFrame();
  render_.render(scene_.get());
  render_.endFrame();
}
} // namespace vk_engine