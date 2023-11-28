#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <cassert>
#include <vector>
#include <Eigen/Dense>

namespace vk_engine {
enum class EventSource : uint8_t { Keyboard, Mouse, Touchscreen };

enum class MouseButton : uint8_t { Left, Right, Middle, Back, Forward, Unknown };

enum class MouseAction : uint8_t { Down, Up, Move, Scroll, Unknown };

struct MouseInputEvent {
  MouseInputEvent(bool h, MouseButton b, MouseAction a, float px, float py)
    : handled(h), button(b), action(a), pos(px, py)
  {}
  
  bool handled;

  MouseButton button;

  MouseAction action;

  Eigen::Vector2f pos;
};

enum class TouchAction {
  Down,
  Up,
  Move,
  Cancel,
  PointerDown,
  PointerUp,
  Unknown
};

struct TouchInputEvent {

  bool handled{false};

  TouchAction action;

  int32_t pointer_id;

  size_t touch_points;

  Eigen::Vector2f pos;
};

class EventHandler {
public:
  virtual void apply(const std::shared_ptr<MouseInputEvent> &mouse_event) {}  
};

class EventManager {
public:
  EventManager() = default;
  void registHandler(const std::shared_ptr<EventHandler> &event_handler)
  {
    assert(event_handler != nullptr);
    event_handlers_.emplace_back(event_handler);
  }

  void handle(const std::shared_ptr<MouseInputEvent> &event) {
    for(auto h : event_handlers_)
    {
      h->apply(event);
      if(event->handled)
        break;
    }
  }
private:
  std::vector<std::shared_ptr<EventHandler>> event_handlers_;
};
} // namespace vk_engine