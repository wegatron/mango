#include "framework/window_app.h"
#include <cassert>
#include <iostream>

namespace vk_engine
{
    WindowApp::~WindowApp()
    {
        glfwDestroyWindow(window_);
        glfwTerminate();
    }

    bool WindowApp::init()
    {
        int ret = glfwInit();
        assert(ret == GLFW_TRUE);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window_ = glfwCreateWindow(width_, height_, window_title_.c_str(), nullptr, nullptr);        
        driver_ = std::make_shared<VkDriver>();
        bool ret = false;
        try {
            #ifdef NDEBUG
            ret = driver_->init(window_title_, false, window_);
            #else
            ret = driver_->init(window_title_, true, window_);
            #endif
        } catch (const std::runtime_error &e) {
            std::cerr << e.what() << std::endl;
            return false;
        }
        return ret;
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
