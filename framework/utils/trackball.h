#pragma once
#include <Eigen/Dense>
#include <framework/platform/input_events.h>

namespace vk_engine {
class Camera;
class Trackball : public EventHandler {
public:
  Trackball(Camera *camera);

  void apply(const std::shared_ptr<MouseInputEvent> &mouse_event) override;

  void rotate(double angle, const Eigen::Vector3f &axis);

  void zoom(double ratio);

  void pan(const Eigen::Vector3f &delta);

private:
  
  enum UpdateMode { INACTIVE = 0, ROTATE, PAN, ZOOM };
  
  uint8_t mouse_button_status_{0};
  Eigen::Vector2f prev_mouse_pos_;
  Camera *camera_;
};
} // namespace vk_engine