#pragma once

#include <memory>
#include <string>
#include <vector>

#include <framework/utils/app_context.h>
#include <framework/vk/frame_buffer.h>
#include <framework/vk/resource_cache.h>
#include <framework/vk/syncs.h>

namespace vk_engine {

class VkDriver;
class Fence;
class Semaphore;

class AppBase {
public:
  AppBase(const std::string &name) : name_(name) {}

  virtual ~AppBase() = default;

  AppBase(const AppBase &) = delete;
  AppBase &operator=(const AppBase &) = delete;

  virtual void tick(const float seconds, const uint32_t rt_index,
                    const uint32_t frame_index) = 0;

  virtual void init(const std::shared_ptr<VkDriver> &driver,
                    const std::vector<std::shared_ptr<RenderTarget>> &rts) = 0;

  const RenderOutputSync &getRenderOutputSync(const uint32_t index) const {
    return render_output_syncs_[index];
  }

protected:
  std::string name_;
  std::vector<RenderOutputSync> render_output_syncs_;
};

} // namespace vk_engine