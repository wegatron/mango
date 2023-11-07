#include <framework/platform/glfw_window.h>
#include <framework/utils/app_base.h>

namespace vk_engine {

MouseButton translateMouseButton(int button)
{
	if (button < GLFW_MOUSE_BUTTON_6)
	{
		return static_cast<MouseButton>(button);
	}

	return MouseButton::Unknown;
}

MouseAction translateMouseAction(int action)
{
	if (action == GLFW_PRESS)
	{
		return MouseAction::Down;
	}
	else if (action == GLFW_RELEASE)
	{
		return MouseAction::Up;
	}

	return MouseAction::Unknown;
}

void cursorPositionCallback(GLFWwindow *window, double xpos, double ypos) {
  if (auto *app =
          reinterpret_cast<AppBase *>(glfwGetWindowUserPointer(window))) {
    app->inputEvent(MouseButtonInputEvent{
        MouseButton::Unknown, MouseAction::Move, static_cast<float>(xpos),
        static_cast<float>(ypos)});
  }
}

void mouseButtonCallback(GLFWwindow *window, int button, int action,
                         int /*mods*/) {
  MouseAction mouse_action = translateMouseAction(action);

  if (auto *app=
          reinterpret_cast<AppBase *>(glfwGetWindowUserPointer(window))) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    app->inputEvent(MouseButtonInputEvent{
        translateMouseButton(button), mouse_action, static_cast<float>(xpos),
        static_cast<float>(ypos)});
  }
}

GlfwWindow::GlfwWindow(const std::string &window_title, const int width,
                       const int height)
    : Window(window_title, width, height) {

#if defined(VK_USE_PLATFORM_XLIB_KHR)
  glfwInitHint(GLFW_X11_XCB_VULKAN_SURFACE, false);
#endif
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window_ =
      glfwCreateWindow(width, height, window_title.c_str(), nullptr, nullptr);
}

VkSurfaceKHR GlfwWindow::create_surface(VkInstance instance,
                                        VkPhysicalDevice physical_device) {
  VkSurfaceKHR surface;
  glfwCreateWindowSurface(instance, window_, nullptr, &surface);
  return surface;
}

void GlfwWindow::getExtent(uint32_t &width, uint32_t &height) const {
  glfwGetWindowSize(window_, reinterpret_cast<int *>(&width),
                    reinterpret_cast<int *>(&height));
}

void GlfwWindow::init(AppBase * app) {
  glfwSetWindowUserPointer(window_, app);
  glfwSetCursorPosCallback(window_, cursorPositionCallback);
  glfwSetMouseButtonCallback(window_, mouseButtonCallback);
}
} // namespace vk_engine