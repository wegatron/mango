#pragma once

#include <entt/entt.hpp>
#include <framework/scene/component/basic.h>
#include <framework/scene/component/material.h>
#include <framework/scene/component/mesh.h>

namespace vk_engine {
class Scene final {
public:
  Scene() = default;

  ~Scene() = default;

  void update(const float seconds, const std::shared_ptr<CommandBuffer> &cmd_buffer);

  entt::registry &camera_manager() { return camera_manager_; }
  entt::registry &light_manager() { return light_manager_; }

  entt::registry &renderableManager() { return renderable_manager_; }
  entt::entity
  createRenderableEntity(const std::string &name,
                         const std::shared_ptr<TransformRelationship> tr,
                         const std::shared_ptr<Material> material,
                         const std::shared_ptr<StaticMesh> mesh);

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

} // namespace vk_engine