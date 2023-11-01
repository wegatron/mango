#include <GLFW/glfw3.h>
#include <framework/utils/app_context.h>
#include <framework/utils/gui.h>
#include <framework/vk/queue.h>
#include <framework/vk/render_pass.h>
#include <imgui/backends/imgui_impl_glfw.h>

namespace vk_engine {
Gui::Gui(GLFWwindow *window, const float font_size = 21.0f,
         bool explicit_update = false) {
  // refer to: https://frguthmann.github.io/posts/vulkan_imgui/
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable
  // Keyboard Controls io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; //
  // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForVulkan(window,
                               true); // init viewport and key/mouse events

  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = g_Instance;
  init_info.PhysicalDevice = g_PhysicalDevice;
  init_info.Device = g_Device;
  init_info.QueueFamily = g_QueueFamily;
  init_info.Queue = g_Queue;
  init_info.PipelineCache = g_PipelineCache;
  init_info.DescriptorPool = g_DescriptorPool;
  init_info.Allocator = g_Allocator;
  init_info.MinImageCount = g_MinImageCount;
  init_info.ImageCount = wd->ImageCount;
  init_info.CheckVkResultFn = check_vk_result;

  initRenderStuffs();
  ImGui_ImplVulkan_Init(
      &init_info,
      render_pass_->getHandle()); // init create resources for render imgui
  ImGui_ImplVulkan_CreateFontsTexture(cmd_buffer);

  auto cmd_buf =
      frames_data[0].command_pool->requestCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);  
  auto &queue = getDefaultAppContext().driver->getGraphicsQueue();
  queue->submit(cmd_buf, frames_data[0].render_fence->getHandle());
  frames_data[0].render_fence->wait(); // signaled -> unsignaled
  frames_data[0].render_fence->reset(); // unsignaled -> signaled
  ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void Gui::initRenderStuffs() {
  auto &driver = getDefaultAppContext().driver;
  std::vector<Attachment> attachments{Attachment{
      color_format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};
  std::vector<LoadStoreInfo> load_store_infos{
      {VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE}};
  std::vector<SubpassInfo> subpass_infos{{
      {},  // no input attachment
      {0}, // color attachment index
      {}  // no msaa
  }};
  auto &resource_cache = getDefaultAppContext().resource_cache;
  auto &driver = getDefaultAppContext().driver;
  render_pass_ = resource_cache->requestRenderPass(
      driver, attachments, load_store_infos, subpass_infos);

  // frame buffer
  auto &frames_data = getDefaultAppContext().frames_data;
  frame_buffers_.resize(frames_data.size());
  for (auto i = 0; i < frames_data.size(); ++i) {
    frame_buffers_[i] = std::move(std::make_unique<FrameBuffer>(
        driver, render_pass_, frames_data[i].render_tgt));
  }
}

Gui::~Gui() {
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void Gui::update(const float delta_time, uint32_t frame_index, uint32_t rt_index)
{
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  // ... imgui elements

  ImGui::ShowDemoWindow();

  // (Your code clears your framebuffer, renders your other stuff etc.)
  // render imgui
  auto &cmd_pool =
      getDefaultAppContext().frames_data[cur_frame_index_].command_pool;
  auto cmd_buf = cmd_pool->requestCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  cmd_buf->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  cmd_buf->beginRenderPass(rpass_.getRenderPass(), frame_buffers_[rt_index]);

  ImGui::Render(); // draw data
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                  cmd_buf->getHandle()); // draw cmd
  // (Your code calls vkCmdEndRenderPass, vkQueueSubmit, vkQueuePresentKHR etc.)
  cmd_buf->endRenderPass();
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
}
} // namespace vk_engine