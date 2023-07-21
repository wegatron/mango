#include <cassert>
#include <framework/utils/logging.h>
#include <framework/utils/window_app.h>
#include <framework/vk/image.h>
#include <framework/vk/syncs.h>

namespace vk_engine {
WindowApp::~WindowApp() {
  app_.reset();
  // destroy render targets(ImageView) before swapchain and depth images
  for(auto &rt : render_targets_) {
    rt.reset();
  }
  glfwDestroyWindow(window_);
  glfwTerminate();

  // destroy swapchain and depth images implicitly
}

bool WindowApp::init() {
#if defined(VK_USE_PLATFORM_XLIB_KHR)
  glfwInitHint(GLFW_X11_XCB_VULKAN_SURFACE, false);
#endif
  if (GLFW_TRUE != glfwInit()) {
    LOGE("Failed to init glfw");
    return false;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  window_ = glfwCreateWindow(width_, height_, window_title_.c_str(), nullptr,
                             nullptr);
  driver_ = std::make_shared<VkDriver>();
  auto config = std::make_shared<Vk11Config>();
  config->setDeviceType(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
  config->setFeatureEnabled(VkConfig::FeatureExtension::GLFW_EXTENSION,
                            VkConfig::EnableState::REQUIRED);  
  config->setFeatureEnabled(VkConfig::FeatureExtension::KHR_SWAPCHAIN,
                            VkConfig::EnableState::REQUIRED);
  config->setFeatureEnabled(VkConfig::FeatureExtension::INSTANCE_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2,
                            VkConfig::EnableState::REQUIRED);
  try {
#ifdef NDEBUG
    driver_->init(window_title_, config, window_);
#else
    config->setFeatureEnabled(VkConfig::FeatureExtension::KHR_VALIDATION_LAYER,
                              VkConfig::EnableState::REQUIRED);
    driver_->init(window_title_, config, window_);
#endif
  } catch (const std::runtime_error &e) {
    LOGE(e.what());
    return false;
  }

  // create swapchain
  initSwapchain();
  initRenderTargets();

  assert(app_ != nullptr);
  app_->init(driver_, render_targets_);
  return true;
}

void WindowApp::setApp(const std::shared_ptr<AppBase> &app) {
  app_ = app;
  // TODO connect resize or mouse action
}

void WindowApp::run() {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
    auto &render_output_sync = app_->getRenderOutputSync(current_frame_index_);
    uint32_t rt_index = swapchain_->acquireNextImage(render_output_sync.present_semaphore->getHandle(), VK_NULL_HANDLE);
    // current_frame_index_ maybe different with rt_index, depends on the swapchain present mode
    app_->tick(0.016f, rt_index, current_frame_index_); // 60fps
    current_frame_index_ = (current_frame_index_ + 1) % render_targets_.size();
  }
}

void WindowApp::initSwapchain() {
  int width = 0;
  int height = 0;
  // real resolution compatiable with highdpi
  glfwGetFramebufferSize(window_, &width, &height);
  SwapchainProperties properties{
      .extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}};
  swapchain_ =
      std::make_shared<Swapchain>(driver_, driver_->getSurface(), properties);  
}

void WindowApp::initRenderTargets()
{
  // depth stencil images
  ds_format_ = VK_FORMAT_D24_UNORM_S8_UINT;
  const auto img_cnt = swapchain_->getImageCount();  
  VkExtent3D extent{swapchain_->getExtent().width, swapchain_->getExtent().height, 1};
  render_targets_.resize(img_cnt);
  depth_images_.resize(img_cnt);
  
  for (uint32_t i = 0; i < img_cnt; ++i) {
    depth_images_[i] = std::make_shared<Image>(
        driver_, 0,
        ds_format_, extent, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    auto depth_img_view = std::make_shared<ImageView>(
        depth_images_[i], VK_IMAGE_VIEW_TYPE_2D, ds_format_, 0, 0, 1, 1);        
    render_targets_[i] = std::make_shared<RenderTarget>(
      std::initializer_list<std::shared_ptr<ImageView>>{swapchain_->getImageView(i), depth_img_view},
      std::initializer_list<VkFormat>{swapchain_->getImageFormat()},
      ds_format_, extent.width,
      extent.height, 1u);
  }
}
} // namespace vk_engine
