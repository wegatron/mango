#include <cassert>
#include <framework/utils/logging.h>
#include <framework/utils/window_app.h>
#include <framework/vk/image.h>

namespace vk_engine {
WindowApp::~WindowApp() {
  glfwDestroyWindow(window_);
  glfwTerminate();
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
  try {
#ifdef NDEBUG
    driver_->init(window_title_, false, window_);
#else
    driver_->init(window_title_, true, window_);
#endif
  } catch (const std::runtime_error &e) {
    LOGE(e.what());
    return false;
  }

  // create swapchain
  initSwapchain();
  initDepthStencil();

  assert(app_ != nullptr);
  app_->init(driver_, swapchain_->getImageFormat(), depth_format_);
  return true;
}

void WindowApp::setApp(const std::shared_ptr<AppBase> &app) {
  app_ = app;
  // TODO connect resize or mouse action
}

void WindowApp::run() {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
    app_->tick(0.016f); // 60fps
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

void WindowApp::initDepthStencil()
{
  depth_format_ = VK_FORMAT_D24_UNORM_S8_UINT;
  const auto img_cnt = swapchain_->getImageCount();
  depth_images_.resize(img_cnt);
  VkExtent3D extent{swapchain_->getExtent().width, swapchain_->getExtent().height, 1};
  for (uint32_t i = 0; i < img_cnt; ++i) {
    depth_images_[i] = std::make_shared<Image>(
        driver_, 0,
        depth_format_, extent, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
  }
}
} // namespace vk_engine
