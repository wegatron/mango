#include "viewer_app.h"
#include <framework/scene/loader.h>
#include <framework/utils/app_context.h>
#include <framework/vk/stage_pool.h>
#include <framework/scene/asset_manager.hpp>
#include <framework/vk/queue.h>
#include <framework/vk/frame_buffer.h>

namespace vk_engine {

void ViewerApp::init(const std::shared_ptr<VkDriver> &driver,
                     const std::vector<std::shared_ptr<RenderTarget>> &rts) {
  initAppContext(driver, rts);
  auto &frames_data = getDefaultAppContext().frames_data;

  // render
  render_ = std::make_unique<Render>(rts[0]->getColorFormat(0), rts[0]->getDSFormat());
  auto cmd_buf =
      frames_data[0].command_pool->requestCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  cmd_buf->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  AssimpLoader loader;
  loader.loadScene(scene_path_, *scene_, cmd_buf);
  cmd_buf->end();
  auto &render_output_syncs = getDefaultAppContext().render_output_syncs;
  render_output_syncs[0].render_fence->reset();
  auto cmd_queue = driver->getGraphicsQueue();
  cmd_queue->submit(cmd_buf, render_output_syncs[0].render_fence->getHandle());
}

void ViewerApp::setScene(const std::string &path) { scene_path_ = path; }

void ViewerApp::tick(const float seconds, const uint32_t rt_index,
                     const uint32_t frame_index) {
  
  getDefaultAppContext().stage_pool->gc();
  getDefaultAppContext().gpu_asset_manager->gc();
  render_->beginFrame(seconds, frame_index, rt_index);
  render_->render(scene_.get());
  render_->endFrame();
}
} // namespace vk_engine