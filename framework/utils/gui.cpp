#include <GLFW/glfw3.h>
#include <framework/utils/app_context.h>
#include <framework/utils/error.h>
#include <framework/utils/gui.h>
#include <framework/vk/commands.h>
#include <framework/vk/descriptor_set.h>
#include <framework/vk/frame_buffer.h>
#include <framework/vk/queue.h>
#include <framework/vk/render_pass.h>
#include <framework/vk/resource_cache.h>
#include <framework/vk/syncs.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_vulkan.h>

static void check_vk_result(VkResult err) {
  if (err == 0)
    return;
  LOGE("Detected Vulkan error: {}", vkb::to_string(err));
  if (err < 0)
    abort();
}

namespace vk_engine {
void Gui::init(GLFWwindow *window) {
  // refer to: https://frguthmann.github.io/posts/vulkan_imgui/
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable
  // Keyboard Controls io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; //
  // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForVulkan(window,
                               true); // init viewport and key/mouse events
  auto &ctx = getDefaultAppContext();
  auto &driver = ctx.driver;
  auto &frames_data = ctx.frames_data;
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = driver->getInstance();
  init_info.PhysicalDevice = driver->getPhysicalDevice();
  init_info.Device = driver->getDevice();
  auto queue = driver->getGraphicsQueue();
  init_info.QueueFamily = queue->getFamilyIndex();
  init_info.Queue = queue->getHandle();
  init_info.PipelineCache = ctx.resource_cache->getPipelineCache();
  init_info.DescriptorPool = ctx.descriptor_pool->getHandle();
  init_info.Allocator = nullptr;
  init_info.MinImageCount = frames_data.size();
  init_info.ImageCount = frames_data.size();
  init_info.CheckVkResultFn = check_vk_result;

  initRenderStuffs();
  auto instance_ptr = getDefaultAppContext().driver->getInstancePtr();

  // refer to https://zhuanlan.zhihu.com/p/634912614
  ImGui_ImplVulkan_LoadFunctions(
      [](const char *function_name, void *vulkan_instance) {
        auto instance_ptr = getDefaultAppContext().driver->getInstancePtr();
        return vkGetInstanceProcAddr(*instance_ptr, function_name);
      },
      reinterpret_cast<void *>(instance_ptr));

  ImGui_ImplVulkan_Init(
      &init_info,
      render_pass_->getHandle()); // init create resources for render imgui
  auto cmd_buf = frames_data[0].command_pool->requestCommandBuffer(
      VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  cmd_buf->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  ImGui_ImplVulkan_CreateFontsTexture(cmd_buf->getHandle());
  cmd_buf->end();
  auto &sync = ctx.render_output_syncs[0];
  sync.render_fence->reset(); // signaled -> unsignaled
  queue->submit(cmd_buf, sync.render_fence->getHandle());
  sync.render_fence->wait(); // unsignaled -> signaled
  ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void Gui::initRenderStuffs() {
  auto &driver = getDefaultAppContext().driver;
  auto &frames_data = getDefaultAppContext().frames_data;
  auto color_format = frames_data[0].render_tgt->getColorFormat(0);
  std::vector<Attachment> attachments{Attachment{
      color_format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};
  std::vector<LoadStoreInfo> load_store_infos{
      {VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE}};
  std::vector<SubpassInfo> subpass_infos{{
      {},  // no input attachment
      {0}, // color attachment index
      {}   // no msaa
  }};
  auto &resource_cache = getDefaultAppContext().resource_cache;
  render_pass_ = resource_cache->requestRenderPass(
      driver, attachments, load_store_infos, subpass_infos);

  // frame buffer
  frame_buffers_.resize(frames_data.size());
  for (auto i = 0; i < frames_data.size(); ++i) {
    frame_buffers_[i] = std::move(std::make_unique<FrameBuffer>(
        driver, render_pass_, frames_data[i].render_tgt, 1));
  }
}

Gui::~Gui() {
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void Gui::update(const std::shared_ptr<CommandBuffer> &cmd_buf,
                 const float time_elapse, uint32_t frame_index,
                 uint32_t rt_index) {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGui::ShowDemoWindow();
  // ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!"
  // and
  //  append into it.
  // ImGui::Text("This is some useful text."); // Display some text (you can use
  // a
  //  format strings too)
  // ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools
  // storing our window open/close state ImGui::Checkbox("Another Window",
  // &show_another_window);

  // ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float
  // using a slider from 0.0f to 1.0f ImGui::ColorEdit3("clear color",
  // (float*)&clear_color); // Edit 3 floats representing a color

  // if (ImGui::Button("Button"))                            // Buttons return
  // true when clicked (most widgets return true when edited/activated)
  //     counter++;
  // ImGui::SameLine();
  // ImGui::Text("counter = %d", counter);

  // ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f /
  // io.Framerate, io.Framerate);
  // ImGui::End();
  // (Your code clears your framebuffer, renders your other stuff etc.)
  // render imgui
  cmd_buf->beginRenderPass(render_pass_, frame_buffers_[rt_index]);

  ImGui::Render(); // draw data
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                  cmd_buf->getHandle()); // draw cmd
  // (Your code calls vkCmdEndRenderPass, vkQueueSubmit, vkQueuePresentKHR etc.)
  cmd_buf->endRenderPass();
}
} // namespace vk_engine