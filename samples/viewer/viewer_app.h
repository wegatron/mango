#pragma once

#include <framework/scene/render.h>
#include <framework/scene/scene.h>
#include <framework/utils/app_base.h>
#include <framework/utils/gui.h>
#include <framework/vk/commands.h>
#include <framework/vk/descriptor_set.h>

namespace vk_engine {
class CommandBuffer;
class StagePool;
class ViewerApp : public AppBase {
public:
  ViewerApp(const std::string &name)
      : AppBase(name), scene_(std::make_unique<Scene>()), gui_(std::make_unique<Gui>()) {}

  ~ViewerApp() override = default;

  void init(Window *window, const std::shared_ptr<VkDriver> &driver,
            const std::vector<std::shared_ptr<RenderTarget>> &rts) override;

  void setScene(const std::string &path);

  void tick(const float seconds, const uint32_t rt_index,
            const uint32_t frame_index) override;

  void
  updateRts(const std::vector<std::shared_ptr<RenderTarget>> &rts) override;

  void inputEvent(const InputEvent &event) override;

private:
  std::string scene_path_;
  std::unique_ptr<Render> render_;
  std::unique_ptr<Scene> scene_;
  std::unique_ptr<Gui> gui_;
};
} // namespace vk_engine