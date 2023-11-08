#pragma once

#include <cstddef>
#include <cstdint>

namespace vk_engine {
enum class EventSource : uint8_t { Keyboard, Mouse, Touchscreen };

class InputEvent {
public:
  InputEvent(EventSource source) : source(source) {}

  EventSource get_source() const { return source; }

private:
  EventSource source;
};

enum class MouseButton : uint8_t { Left, Right, Middle, Back, Forward, Unknown };

enum class MouseAction : uint8_t { Down, Up, Move, Unknown };

class MouseButtonInputEvent : public InputEvent {
public:
  MouseButtonInputEvent(MouseButton button, MouseAction action, float pos_x,
                        float pos_y) : InputEvent(EventSource::Mouse),
                                       button(button), action(action),
                                       pos_x(pos_x), pos_y(pos_y) {}

  MouseButton get_button() const { return button; }

  MouseAction get_action() const { return action; }

  float get_pos_x() const { return pos_x; }

  float get_pos_y() const  { return pos_y; }

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
                  float pos_x, float pos_y) : InputEvent(EventSource::Touchscreen),
                                              pointer_id(pointer_id),
                                              touch_points(pointer_count),
                                              action(action), pos_x(pos_x),
                                              pos_y(pos_y) {}

  TouchAction get_action() const { return action; }

  int32_t get_pointer_id() const { return pointer_id; }

  size_t get_touch_points() const { return touch_points; }

  float get_pos_x() const { return pos_x; }

  float get_pos_y() const { return pos_y; }

private:
  TouchAction action;

  int32_t pointer_id;

  size_t touch_points;

  float pos_x;

  float pos_y;
};
} // namespace vk_engine