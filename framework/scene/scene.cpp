#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <framework/scene/scene.h>
#include <framework/utils/logging.h>
#include <stack>

namespace vk_engine {
void loadScene(const std::string &path, Scene &scene) {
  Assimp::Importer importer;
  const aiScene *a_scene =
      importer.ReadFile(path, aiProcessPreset_TargetRealtime_Quality);

  if (!a_scene) {
    throw std::runtime_error("Assimp import error:" +
                             std::string(importer.GetErrorString()));
  }

  // for mesh
  std::stack<aiNode *> node_stack;
  node_stack.push(a_scene->mRootNode);
  while(!node_stack.empty())
  {
    aiNode *node = node_stack.top();
    node_stack.pop();

    for(auto i=0; i<node->mNumMeshes; ++i)
    {
      aiMesh *mesh = a_scene->mMeshes[node->mMeshes[i]];
      // todo
    }

    for (uint32_t i = 0; i < node->mNumChildren; ++i) {
      node_stack.push(node->mChildren[i]);
    }
  }

  // load the default camera if have
  LOGI("load scene: %s", path.c_str());
}

void Scene::prepare() {}
} // namespace vk_engine