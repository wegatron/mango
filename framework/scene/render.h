#pragma once

#include <vector>
#include <framework/scene/rpass.h>
#include <framework/vk/syncs.h>

namespace vk_engine {
class CommandBuffer;
class Scene;
class FrameBuffer;
class Gui;
class Render {
public:
  Render(VkFormat color_format, VkFormat ds_format);

  void beginFrame(const float time_elapse, const uint32_t frame_index,
                  const uint32_t rt_index);

  void render(Scene *scene, Gui *gui);

  void initRts();

  void endFrame();

private:
  uint32_t cur_frame_index_{0};
  uint32_t cur_rt_index_{0};
  float cur_time_{0.0};
  std::shared_ptr<CommandBuffer> cmd_buf_;
  RPass rpass_;
  std::vector<std::unique_ptr<FrameBuffer>> frame_buffers_;
};
} // namespace vk_engine