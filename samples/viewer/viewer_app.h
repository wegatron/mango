#pragma once

#include <framework/scene/scene.h>
#include <framework/scene/render.h>
#include <framework/utils/app_base.h>
#include <framework/vk/commands.h>
#include <framework/vk/descriptor_set.h>


namespace vk_engine {
class CommandBuffer;
class StagePool;
class ViewerApp : public AppBase {
public:
  ViewerApp(const std::string &name, VkFormat color_format, VkFormat ds_format)
      : AppBase(name), scene_(std::make_unique<Scene>()), render_(render_output_syncs_, color_format, ds_format) {}

  ~ViewerApp() override = default;

  void init(const std::shared_ptr<VkDriver> &driver,
            const std::vector<std::shared_ptr<RenderTarget>> &rts) override;

  void setScene(const std::string &path);

  void tick(const float seconds, const uint32_t rt_index,
            const uint32_t frame_index) override;

private:
  std::string scene_path_;
  Render render_;
  std::unique_ptr<Scene> scene_;
};
} // namespace vk_engine