#pragma once
#include <Eigen/Dense>
#include <framework/platform/input_events.h>

namespace vk_engine {
class Camera;
class Trackball : public EventHandler {
public:
  Trackball(Camera *camera) : camera_(camera) {}

  void apply(const std::shared_ptr<MouseInputEvent> &mouse_event) override;

private:

  void rotate(const Eigen::Vector2f &prev_mouse_pos, const Eigen::Vector2f &cur_mouse_pos);
  
  void pan(const Eigen::Vector2f &prev_mouse_pos, const Eigen::Vector2f &cur_mouse_pos);
  
  void zoom(const float delta_y);  
  
  enum UpdateMode { INACTIVE = 0, ROTATE, PAN, ZOOM };
  
  uint8_t mouse_button_status_{0};
  Eigen::Vector2f prev_mouse_pos_;
  Camera *camera_;
};
} // namespace vk_engine