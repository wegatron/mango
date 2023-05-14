#include "framework/window_app.h"
#include "framework/logging.h"
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
        #if defined(VK_USE_PLATFORM_XLIB_KHR)
	        glfwInitHint(GLFW_X11_XCB_VULKAN_SURFACE, false);
        #endif
        if(GLFW_TRUE != glfwInit())
        {
            std::cerr << "Failed to init glfw" << std::endl;
            return false;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window_ = glfwCreateWindow(width_, height_, window_title_.c_str(), nullptr, nullptr);        
        driver_ = std::make_shared<VkDriver>();
        bool ret = false;
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
