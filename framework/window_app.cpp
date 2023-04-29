#include "framework/window_app.h"
#include <cassert>

namespace vk_engine
{
    WindowApp::WindowApp(const std::string &window_title, const int width, const int height)
    {
        int ret = glfwInit();
        assert(ret == GLFW_TRUE);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window_ = glfwCreateWindow(width, height, window_title.c_str(), nullptr, nullptr);

        driver_ = std::make_shared<VkDriver>();
        #ifdef NDEBUG
        driver_->init(window_title, false);
        #else
        driver_->init(window_title, true);
        #endif
    }

    WindowApp::~WindowApp()
    {
        glfwDestroyWindow(window_);
        glfwTerminate();
    }

    void WindowApp::setApp(const std::shared_ptr<AppBase> &app)
    {
        app_ = app;
        // TODO connect resize or mouse action
    }

    void WindowApp::run()
    {
        while(!glfwWindowShouldClose(window_))
        {
            glfwPollEvents();
            app_->tick(0.016f); //60fps
        }
    }
}
