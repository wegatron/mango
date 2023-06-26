#pragma once

#include <framework/utils/app_base.h>

namespace vk_engine {
class TriangleApp : public AppBase {
public:
  TriangleApp(const std::string &name, const std::shared_ptr<ResourceCache> &resource_cache) 
    : AppBase(name, resource_cache) {}

  void init(const std::shared_ptr<VkDriver> &driver) override;

  void tick(const float seconds) override;
};
} // namespace vk_engine