#include <cassert>
#include <framework/scene/render.h>
#include <framework/scene/rpass.h>
#include <framework/scene/scene.h>
#include <framework/utils/app_context.h>
#include <framework/vk/commands.h>
#include <framework/vk/frame_buffer.h>
#include <framework/vk/queue.h>
namespace vk_engine {
Render::Render(VkFormat color_format, VkFormat ds_format)
    : rpass_(color_format, ds_format) {
  auto &frames_data = getDefaultAppContext().frames_data;
  frame_buffers_.resize(frames_data.size());
  auto &driver = getDefaultAppContext().driver;
  for (auto i = 0; i < frames_data.size(); ++i) {
    frame_buffers_[i] = std::move(std::make_unique<FrameBuffer>(
        driver, rpass_.getRenderPass(), frames_data[i].render_tgt));
  }
}

void Render::beginFrame(const float time_elapse, const uint32_t frame_index,
                        const uint32_t rt_index) {
  auto &render_output_syncs = getDefaultAppContext().render_output_syncs;
  assert(!render_output_syncs.empty());
  cur_frame_index_ = frame_index;
  cur_rt_index_ = rt_index;
  cur_time_ = time_elapse;
  auto &sync = render_output_syncs[frame_index];
  sync.render_fence->wait();
  sync.render_fence->reset();
  auto &cmd_pool =
      getDefaultAppContext().frames_data[cur_frame_index_].command_pool;
  cmd_pool->reset();
  cmd_buf_ = cmd_pool->requestCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  cmd_buf_->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  cmd_buf_->beginRenderPass(rpass_.getRenderPass(), frame_buffers_[rt_index]);
}

void Render::render(Scene *scene) {
  assert(scene != nullptr);
  scene->update(cur_time_, cmd_buf_);

  auto &rm = scene->renderableManager();
  auto view = rm.view<std::shared_ptr<TransformRelationship>,
                      std::shared_ptr<Material>, std::shared_ptr<StaticMesh>>();
  rpass_.gc();
  auto width = frame_buffers_[cur_rt_index_]->getWidth();
  auto height = frame_buffers_[cur_rt_index_]->getHeight();
  view.each(
      [this, width, height](const std::shared_ptr<TransformRelationship> &tr,
                            const std::shared_ptr<Material> &mat,
                            const std::shared_ptr<StaticMesh> &mesh) {
        // update materials
        mat->updateParams();
        rpass_.draw(mat, tr->gtransform, mesh, cmd_buf_, width, height);
      });
  cmd_buf_->endRenderPass();
}

void Render::endFrame() {
  // add a barrier to transition the swapchain image from color attachment to
  // present
  ImageMemoryBarrier barrier{
      .old_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .new_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
      .src_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      .dst_access_mask = 0,
      .src_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dst_stage_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
      .src_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
      .dst_queue_family_index = VK_QUEUE_FAMILY_IGNORED};
  auto &frame_data = getDefaultAppContext().frames_data[cur_rt_index_];
  cmd_buf_->imageMemoryBarrier(barrier,
                               frame_data.render_tgt->getImageViews()[0]);
  cmd_buf_->end();
  auto cmd_queue = getDefaultAppContext().driver->getGraphicsQueue();
  auto &render_output_syncs = getDefaultAppContext().render_output_syncs;

  auto cmd_buf_handle = cmd_buf_->getHandle();
  auto present_semaphore =
      render_output_syncs[cur_frame_index_].present_semaphore->getHandle();
  VkPipelineStageFlags wait_stage{
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  VkSemaphore render_semaphore =
      render_output_syncs[cur_frame_index_].render_semaphore->getHandle();
  VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &cmd_buf_handle;
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = &present_semaphore;
  submit_info.pWaitDstStageMask = &wait_stage;
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = &render_semaphore;
  cmd_queue->submit({submit_info},
      render_output_syncs[cur_frame_index_].render_fence->getHandle());
}
} // namespace vk_engine