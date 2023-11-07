#pragma once

#include <cstddef>
#include <cstdint>

namespace vk_engine {
enum class EventSource { Keyboard, Mouse, Touchscreen };

class InputEvent {
public:
  InputEvent(EventSource source);

  EventSource get_source() const;

private:
  EventSource source;
};

enum class MouseButton { Left, Right, Middle, Back, Forward, Unknown };

enum class MouseAction { Down, Up, Move, Unknown };

class MouseButtonInputEvent : public InputEvent {
public:
  MouseButtonInputEvent(MouseButton button, MouseAction action, float pos_x,
                        float pos_y);

  MouseButton get_button() const;

  MouseAction get_action() const;

  float get_pos_x() const;

  float get_pos_y() const;

private:
  MouseButton button;

  MouseAction action;

  float pos_x;

  float pos_y;
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

class TouchInputEvent : public InputEvent {
public:
  TouchInputEvent(int32_t pointer_id, size_t pointer_count, TouchAction action,
                  float pos_x, float pos_y);

  TouchAction get_action() const;

  int32_t get_pointer_id() const;

  size_t get_touch_points() const;

  float get_pos_x() const;

  float get_pos_y() const;

private:
  TouchAction action;

  int32_t pointer_id;

  size_t touch_points;

  float pos_x;

  float pos_y;
};
} // namespace vk_engine