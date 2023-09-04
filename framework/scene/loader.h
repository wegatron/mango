#pragma once

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <framework/scene/scene.h>
#include <framework/vk/vk_driver.h>

namespace vk_engine {

class CommandBuffer;
class StagePool;

// todo Static Mesh, Material TransformRelationship memory management
class AssimpLoader final {
public:
  AssimpLoader(std::shared_ptr<VkDriver> &driver) : driver_(driver) {}

  void loadScene(const std::string &path, Scene &scene,
                 const std::shared_ptr<CommandBuffer> &cmd_buf,
                 const std::shared_ptr<StagePool> &stage_pool);

private:
  std::shared_ptr<TransformRelationship>
  processNode(const std::shared_ptr<TransformRelationship> &parent,
              aiNode *node, const aiScene *a_scene, Scene &scene,
              std::vector<std::shared_ptr<StaticMesh>> &meshes,
              std::vector<std::shared_ptr<Material>> &materials);

  std::vector<std::shared_ptr<StaticMesh>>
  processMeshs(const aiScene *a_scene, Scene &,
               const std::shared_ptr<CommandBuffer> &cmd_buf,
               const std::shared_ptr<StagePool> &stage_pool);
  std::vector<std::shared_ptr<Material>>
  processMaterials(const aiScene *a_scene, Scene &);

  std::shared_ptr<VkDriver> &driver_;
};
} // namespace vk_engine