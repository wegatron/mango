#include <framework/scene/component/camera.h>
#include <framework/utils/logging.h>
#include <framework/utils/trackball.h>
#include <cmath>


namespace vk_engine {

// according to https://www.khronos.org/opengl/wiki/Object_Mouse_Trackball
Eigen::Vector3f tbc(const Eigen::Vector2f &mouse_pos) 
{
  Eigen::Vector2f v = mouse_pos * 2.0f - Eigen::Vector2f(1.0f, 1.0f); // normalize to [-1,1]
  // r = 1.0f
  float l = v.norm();
  float z = l<0.5f ? sqrt(1.0f - l * l) : 0.5f/l;
  return Eigen::Vector3f(v.x(), v.y(), z);
}

void Trackball::apply(const std::shared_ptr<MouseInputEvent> &mouse_event) {
  // LOGI("mouse_event! {} {} {}", static_cast<uint8_t>(mouse_event->action),
  //      static_cast<uint8_t>(mouse_event->button), mouse_event->pos);
  switch (mouse_event->action) {
  case MouseAction::Down:
    mouse_button_status_ |= (1 << static_cast<uint8_t>(mouse_event->button));
    prev_mouse_pos_ = mouse_event->pos;
    break;
  case MouseAction::Up:
    mouse_button_status_ &= ~(1 << static_cast<uint8_t>(mouse_event->button));
    break;
  case MouseAction::Move:
    if (mouse_button_status_ >> static_cast<uint8_t>(MouseButton::Left) & 1) {
      Eigen::Vector3f prev_tbc = tbc(prev_mouse_pos_);
      Eigen::Vector3f cur_tbc = tbc(Eigen::Vector2f(mouse_event->pos));
      Eigen::Vector3f xp = prev_tbc.cross(cur_tbc);
      float xp_len = xp.norm();
      if (xp_len > 0.0f) {
        float angle = asin(xp_len);
        Eigen::Vector3f axis = xp / xp_len;
        Eigen::AngleAxisf rotate(angle, axis);
        // update camera
        Eigen::Matrix3f r = camera_->getViewMatrix().block<3, 3>(0, 0);
        camera_->setRotation(rotate * r);
        LOGI("prev_mouse_pos {}, cur_mouse_pos {}, angle {} axis {}\n camera view mat:\n{}",
            prev_mouse_pos_.transpose(), mouse_event->pos.transpose(), angle,
            axis.transpose(), camera_->getViewMatrix());
      }
    } else if (mouse_button_status_ >>
                   static_cast<uint8_t>(MouseButton::Middle) &
               1) {
      // todo calc move
    } // strol
    prev_mouse_pos_ = mouse_event->pos;
    break;
  default:
    break;
  }
}

void Trackball::rotate(double angle, const Eigen::Vector3f &axis) {}

void Trackball::zoom(double ratio) {}

void Trackball::pan(const Eigen::Vector3f &delta) {}

} // namespace vk_engine