#pragma once

#include <framework/vk/resource_cache.h>
#include <framework/vk/frame_buffer.h>
#include <memory>
#include <string>

namespace vk_engine {

class VkDriver;
class Fence;
class Semaphore;

struct RenderOutputSync
{
  std::shared_ptr<Fence> render_fence;
  std::shared_ptr<Semaphore> render_semaphore;
  std::shared_ptr<Semaphore> present_semaphore;
};

class AppBase {
public:
  AppBase(const std::string &name,
          const std::shared_ptr<ResourceCache> &resource_cache)
      : name_(name), resource_cache_(resource_cache) {
    if (resource_cache_ == nullptr)
      resource_cache_ = std::make_shared<ResourceCache>();
  }

  virtual ~AppBase() = default;

  AppBase(const AppBase &) = delete;
  AppBase &operator=(const AppBase &) = delete;

  virtual void tick(const float seconds, const uint32_t render_target_index, const uint32_t frame_index) = 0;

  virtual void init(const std::shared_ptr<VkDriver> &driver,
    const std::vector<std::shared_ptr<RenderTarget>> &rts) = 0;

  const RenderOutputSync &getRenderOutputSync(const uint32_t index) const {
    return render_output_syncs_[index];
  }

protected:
  const std::string name_;
  std::shared_ptr<VkDriver> driver_;
  std::shared_ptr<ResourceCache> resource_cache_;
  std::vector<RenderOutputSync> render_output_syncs_;
};

} // namespace vk_engine