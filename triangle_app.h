#pragma once

#include "app_base.h"

namespace vk_engine
{
    class TriangleApp: public AppBase
    {
    public:
        TriangleApp(const std::string &name): AppBase(name) {}
        void tick(const float seconds) override;
    };
}