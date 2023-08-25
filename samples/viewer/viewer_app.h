#pragma once

#include <framework/utils/app_base.h>
#include <framework/vk/descriptor_set.h>
#include <framework/vk/commands.h>
#include <framework/scene/scene.h>


namespace vk_engine {
class CommandBuffer;
class ViewerApp : public AppBase {
public:
  ViewerApp(const std::string &name, const std::shared_ptr<ResourceCache> &resource_cache) 
    : AppBase(name, resource_cache),scene_(std::make_unique<Scene>()) {}

  ~ViewerApp() override = default;

  void init(const std::shared_ptr<VkDriver> &driver, const std::vector<std::shared_ptr<RenderTarget>> &rts) override;

  void setScene(const std::string &path);

  void tick(const float seconds, const uint32_t rt_index, const uint32_t frame_index) override;

private:

  std::shared_ptr<VkDriver> driver_;
  std::shared_ptr<DescriptorPool> descriptor_pool_;
  std::unique_ptr<CommandPool> command_pool_;
  std::vector<std::shared_ptr<RenderTarget>> rts_;
  
  std::unique_ptr<Scene> scene_;
};
} // namespace vk_engine