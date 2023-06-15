#pragma once

#include <memory>
#include <string>
#include <framework/vk/resource_cache.h>

namespace vk_engine {
class VkDriver;
class AppBase {
public:
  AppBase(const std::string &name) : name_(name) {}
  virtual ~AppBase() = default;

  AppBase(const AppBase &) = delete;
  AppBase &operator=(const AppBase &) = delete;

  virtual void tick(const float seconds) = 0;
  virtual void init(const std::shared_ptr<VkDriver> &driver) = 0;

protected:
  const std::string name_;
  std::shared_ptr<VkDriver> driver_;
  std::shared_ptr<ResourceCache> resource_cache_;
};
} // namespace vk_engine