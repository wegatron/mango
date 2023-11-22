#include <framework/scene/component/camera.h>
#include <framework/utils/logging.h>
#include <framework/utils/trackball.h>
#include <cmath>


namespace vk_engine {

Eigen::Vector3f tbc(const Eigen::Vector2f &mouse_pos) 
{
  Eigen::Vector2f v = mouse_pos * 2.0f - Eigen::Vector2f(1.0f, 1.0f);
  float l = v.norm();
  if (l < 1.0f) {
    float h = 0.5f + cos(l * M_PI) * 0.5f;
    return Eigen::Vector3f(v.x(), -v.y(), h);
  }
  return Eigen::Vector3f(v.x(), -v.y(), 0);
}

Trackball::Trackball(Camera *camera) {}

void Trackball::apply(const std::shared_ptr<MouseInputEvent> &mouse_event) {
  LOGI("mouse_event! {} {} {}", static_cast<uint8_t>(mouse_event->action),
       static_cast<uint8_t>(mouse_event->button), mouse_event->pos);
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
      Eigen::Vector3f xp = cur_tbc.cross(prev_tbc);
      float xp_len = xp.norm();
      if (xp_len > 0.0f) {
        float angle = asin(xp_len);
        Eigen::Vector3f axis = xp / xp_len;
        Eigen::AngleAxisf rotate(angle, axis);
        // update camera
        Eigen::Matrix3f r = camera_->getViewMatrix().block<3, 3>(0, 0);
        camera_->setRotation(r * rotate);
      }
    } else if (mouse_button_status_ >>
                   static_cast<uint8_t>(MouseButton::Middle) &
               1) {
      // todo calc move
    }
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