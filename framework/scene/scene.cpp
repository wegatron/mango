#include <framework/scene/scene.h>

// entt reference: https://skypjack.github.io/entt/md_docs_md_entity.html
// https://github.com/skypjack/entt/wiki/Crash-Course:-core-functionalities#introduction
namespace vk_engine {

entt::entity Scene::createRenderableEntity(const std::string &name,
                                           const std::shared_ptr<TransformRelationship> tr,
                                           const std::shared_ptr<Material> material,
                                           const std::shared_ptr<StaticMesh> mesh) {
  entt::entity entity = renderable_manager_.create();
  renderable_manager_.emplace<std::string>(entity, name); // name
  renderable_manager_.emplace<std::shared_ptr<TransformRelationship>>(entity, tr); // node transform index
  renderable_manager_.emplace<std::shared_ptr<Material>>(entity, material); // material index
  renderable_manager_.emplace<std::shared_ptr<StaticMesh>>(entity, mesh); // mesh index
  return entity;
}

void Scene::prepare() {}

} // namespace vk_engine