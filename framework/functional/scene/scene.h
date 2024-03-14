#pragma once

#include <entt/entt.hpp>
#include <framework/functional/component/basic.h>
#include <framework/functional/component/material.h>
#include <framework/functional/component/mesh.h>
#include <framework/functional/component/camera.h>
#include <framework/functional/component/light.h>

namespace vk_engine {
class Scene final {
public:
  Scene() = default;

  ~Scene() = default;

  void update(const float seconds);

  entt::registry &camera_manager() { return camera_manager_; }
  entt::registry &light_manager() { return light_manager_; }

  entt::registry &renderableManager() { return renderable_manager_; }
  entt::entity
  createRenderableEntity(const std::string &name,
                         const std::shared_ptr<TransformRelationship> &tr,
                         const std::shared_ptr<Material> &material,
                         const std::shared_ptr<StaticMesh> &mesh);
  
  entt::entity
  createCameraEntity(const std::string &name, const std::shared_ptr<TransformRelationship> &tr,
                     const Camera &camera);

  entt::entity
  createLightEntity(const std::string_view &name,
                    const std::shared_ptr<TransformRelationship> &tr,
                    const Light &light);

  void setRootTr(const std::shared_ptr<TransformRelationship> &root_tr) { root_tr_ = root_tr; }

  // disable copy/move
  Scene(const Scene &) = delete;
  Scene(Scene &&) = delete;
  Scene &operator=(const Scene &) = delete;
  Scene &operator=(Scene &&) = delete;

private:
  entt::registry camera_manager_;
  entt::registry light_manager_;
  entt::registry renderable_manager_;
  std::shared_ptr<TransformRelationship> root_tr_; // root transform relationship node
};

} // namespace vk_engine