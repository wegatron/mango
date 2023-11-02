#pragma once

#include <memory>
#include <vector>
#include <imgui/imgui.h>


struct GLFWwindow;
namespace vk_engine {

class RenderPass;
class FrameBuffer;

/**
 * @brief Responsible for drawing new elements into the gui
 */
class Drawer {
public:
  Drawer() = default;

  /**
   * @brief Clears the dirty bit set
   */
  void clear();

  /**
   * @brief Returns true if the drawer has been updated
   */
  bool is_dirty();

  /**
   * @brief May be used to force drawer update
   */
  void set_dirty(bool dirty);

  /**
   * @brief Adds a collapsable header item tGLFWwindowo the gui
   * @param caption The text to display
   * @returns True if adding item was successful
   */
  bool header(const char *caption);

  /**
   * @brief Adds a checkbox to the gui
   * @param caption The text to display
   * @param value The boolean value to map the checkbox to
   * @returns True if adding item was successful
   */
  bool checkbox(const char *caption, bool *value);

  /**
   * @brief Adds a checkbox to the gui
   * @param caption The text to display
   * @param value The integer value to map the checkbox to
   * @returns True if adding item was successful
   */
  bool checkbox(const char *caption, int32_t *value);

  /**
   * @brief Adds a number input field to the gui
   * @param caption The text to display
   * @param value The value to map to
   * @param step The step increment
   * @param precision The precision
   * @returns True if adding item was successful
   */
  bool input_float(const char *caption, float *value, float step,
                   uint32_t precision);

  /**
   * @brief Adds a slide bar to the gui for floating points to the gui
   * @param caption The text to display
   * @param value The value to map to
   * @param min The minimum value
   * @param max The maximum value
   * @returns True if adding item was successful
   */
  bool slider_float(const char *caption, float *value, float min, float max);

  /**
   * @brief Adds a slide bar to the gui for integers to the gui
   * @param caption The text to display
   * @param value The value to map to
   * @param min The minimum value
   * @param max The maximum value
   * @returns True if adding item was successful
   */
  bool slider_int(const char *caption, int32_t *value, int32_t min,
                  int32_t max);

  /**
   * @brief Adds a multiple choice drop box to the gui
   * @param caption The text to display
   * @param itemindex The item index to display
   * @param items The items to display in the box
   * @returns True if adding item was successful
   */
  bool combo_box(const char *caption, int32_t *itemindex,
                 std::vector<std::string> items);

  /**
   * @brief Adds a clickable button to the gui
   * @param caption The text to display
   * @returns True if adding item was successful
   */
  bool button(const char *caption);

  /**
   * @brief Adds a label to the gui
   * @param formatstr The format string
   */
  void text(const char *formatstr, ...);

private:
  bool dirty{false};
};

/**
 * @brief Vulkan helper class for Dear ImGui
 */
class Gui final {
public:
  
  Gui() = default;

  void init(GLFWwindow *window);

  /**
   * @brief Destroys the Gui
   */
  ~Gui();

  /**
   * @brief Handles resizing of the window
   * @param width New width of the window
   * @param height New height of the window
   */
  void resize(const uint32_t width, const uint32_t height) const;

  void update(const float delta_time, uint32_t frame_index,
                 uint32_t rt_index);

  //   bool input_event(const InputEvent &input_event);
  
private:
  
  void initRenderStuffs();

  std::shared_ptr<RenderPass> render_pass_;

  std::vector<std::unique_ptr<FrameBuffer>> frame_buffers_;

  Drawer drawer;
};
} // namespace vk_engine
