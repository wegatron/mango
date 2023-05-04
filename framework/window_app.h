#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <memory>
#include "framework/app_base.h"
#include "framework/vk_driver.h"

namespace vk_engine
{
    class WindowApp final
    {
        public:
            WindowApp(const std::string &window_title, const int width, const int height):
                window_title_(window_title), width_(width), height_(height) {}
            
            ~WindowApp();

            WindowApp(const WindowApp &) = delete;
            WindowApp& operator=(const WindowApp &) = delete;
            
            bool init();
            void setApp(const std::shared_ptr<AppBase> &app);
            void run();
        private:
            std::shared_ptr<VkDriver> driver_;
            std::shared_ptr<AppBase> app_;
            GLFWwindow * window_;
            
            uint32_t width_;
            uint32_t height_;
            std::string window_title_;
    };
}