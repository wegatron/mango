#include <cmath>
#include <framework/scene/component/camera.h>
#include <framework/utils/logging.h>
#include <framework/utils/trackball.h>

namespace vk_engine {

// according to https://www.khronos.org/opengl/wiki/Object_Mouse_Trackball
Eigen::Vector3f tbc(const Eigen::Vector2f &mouse_pos) {
  Eigen::Vector2f v =
      mouse_pos * 2.0f - Eigen::Vector2f(1.0f, 1.0f); // normalize to [-1,1]
  // r = 1.0f
  float l = v.norm();
  float z = l < 0.5f ? sqrt(1.0f - l * l) : 0.5f / l;
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
  case MouseAction::Move: {
    if (mouse_button_status_ >> static_cast<uint8_t>(MouseButton::Left) & 1) {
      rotate(prev_mouse_pos_, mouse_event->pos);
    } else if (mouse_button_status_ >>
                   static_cast<uint8_t>(MouseButton::Middle) &
               1) {
      pan(prev_mouse_pos_, mouse_event->pos);
    }
    prev_mouse_pos_ = mouse_event->pos;
  } break;
  case MouseAction::Scroll: {
    zoom(mouse_event->pos.y());
    break;
  }
  default:
    break;
  }
}

void Trackball::rotate(const Eigen::Vector2f &prev_mouse_pos,
                       const Eigen::Vector2f &cur_mouse_pos)
{
      Eigen::Vector3f prev_tbc = tbc(prev_mouse_pos_);
      Eigen::Vector3f cur_tbc = tbc(cur_mouse_pos);
      Eigen::Vector3f xp = prev_tbc.cross(cur_tbc);
      float xp_len = xp.norm();
      if (xp_len > 0.0f) {
        float angle = asin(xp_len);
        Eigen::Vector3f axis = xp / xp_len;
        Eigen::AngleAxisf rotate(angle, axis);
        // update camera
        Eigen::Matrix3f r = camera_->getViewMatrix().block<3, 3>(0, 0);
        camera_->setRotation(rotate * r);
        // LOGI("prev_mouse_pos {}, cur_mouse_pos {}, angle {} axis {}\n camera "
        //      "view mat:\n{}",
        //      prev_mouse_pos_.transpose(), mouse_event->pos.transpose(), angle,
        //      axis.transpose(), camera_->getViewMatrix());
      }  
}

void Trackball::pan(const Eigen::Vector2f &prev_mouse_pos,
                    const Eigen::Vector2f &cur_mouse_pos)
{
      // set camera translation
      float fx = camera_->getProjMatrix()(0, 0);
      float fy = camera_->getProjMatrix()(1, 1);
      Eigen::Matrix3f rotation = camera_->getViewMatrix().block<3, 3>(0, 0);
      // fy * (y + translate_y)/(-z) = proj_y + mouse_delta_y
      float z = camera_->getDis();
      float dx = (cur_mouse_pos.x() - prev_mouse_pos_.x()) / fx * z;
      float dy = (cur_mouse_pos.y() - prev_mouse_pos_.y()) / fy * z;
      Eigen::Vector3f delta(dx, dy, 0.0f);
      camera_->getViewMatrix().block<3, 1>(0, 3) += delta;  
}

void Trackball::zoom(const float delta_y) {
    // set camera translation
    float z = camera_->getDis();
    if (delta_y > 0.5f) {
      camera_->getViewMatrix()(2, 3) += 0.08 * z;
    } else if (delta_y < -0.5f) {
      camera_->getViewMatrix()(2, 3) -= 0.08 * z;
    }  
}

} // namespace vk_engine