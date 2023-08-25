#pragma once

#include <entt/entt.hpp>
namespace vk_engine {
class Scene final {
public:
  Scene() = default;
  ~Scene() = default;

  void prepare();

  entt::registry &camera_manager() { return camera_manager_; }
  entt::registry &light_manager() { return light_manager_; }
  entt::registry &renderable_manager() { return renderable_manager_; }

  // disable copy/move
  Scene(const Scene &) = delete;
  Scene(Scene &&) = delete;
  Scene &operator=(const Scene &) = delete;
  Scene &operator=(Scene &&) = delete;

private:
  entt::registry camera_manager_;
  entt::registry light_manager_;
  entt::registry renderable_manager_;
};

void loadScene(const std::string &path, Scene &scene);

} // namespace sg