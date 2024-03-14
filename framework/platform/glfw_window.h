#pragma once
#include <framework/platform/window.h>
#include <GLFW/glfw3.h>

namespace vk_engine
{
class GlfwWindow final : public Window {
public:
  GlfwWindow(const std::string &window_title, const int width,
             const int height);
  
  ~GlfwWindow() override;

  VkSurfaceKHR createSurface(VkInstance instance) override;

  bool shouldClose() override;

  void initImgui() override;

  void shutdownImgui() override;

  void imguiNewFrame() override;  

  void getExtent(uint32_t &width, uint32_t &height) const override;

  void processEvents() override
  {
    glfwPollEvents();
  }

  // setup callback function
  void setupCallback(AppBase * app) override;
private:  
  GLFWwindow *window_;
};
} // namespace vk_engine