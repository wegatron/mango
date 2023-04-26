#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <memory>
#include "app_base.h"

namespace vk_engine
{
    class WindowApp final
    {
        public:
            WindowApp(const std::string &window_title, const int width, const int height);
            ~WindowApp();

            WindowApp(const WindowApp &) = delete;
            WindowApp& operator=(const WindowApp &) = delete;

            void setApp(const std::shared_ptr<AppBase> &app);
            void run();
        private:
            std::shared_ptr<AppBase> app_;
            GLFWwindow * window_;
    };
}