#pragma once

#include "framework/app_base.h"

namespace vk_engine
{
    class TriangleApp: public AppBase
    {
    public:
        TriangleApp(const std::string &name): AppBase(name) {
            
        }

        void init(const std::shared_ptr<VkDriver> &driver) override;

        void tick(const float seconds) override;
    };
}