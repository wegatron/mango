#include <framework/scene/loader.h>

#include <Eigen/Dense>
#include <queue>

#include <framework/utils/logging.h>

namespace vk_engine {

std::shared_ptr<TransformRelationship>
AssimpLoader::processNode(const std::shared_ptr<TransformRelationship> &parent,
                          aiNode *node, const aiScene *a_scene, Scene &scene,
                          std::vector<std::shared_ptr<StaticMesh>> &meshes,
                          std::vector<std::shared_ptr<Material>> &materials) {
  auto cur_tr = std::make_shared<TransformRelationship>();
  cur_tr->parent = parent;
  memcpy(cur_tr->transform.data(), &node->mTransformation,
         sizeof(Eigen::Matrix4f));
  cur_tr->transform.transposeInPlace();

  // process all the node's meshes (if any)
  for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
    aiMesh *a_mesh = a_scene->mMeshes[node->mMeshes[i]];
    assert(materials.size() > a_mesh->mMaterialIndex);
    assert(meshes.size() > node->mMeshes[i]);
    auto renderable_entt = scene.createRenderableEntity(
        a_mesh->mName.C_Str(), cur_tr, materials[a_mesh->mMaterialIndex],
        meshes[node->mMeshes[i]]);
  }
}

void AssimpLoader::loadScene(const std::string &path, Scene &scene) {
  Assimp::Importer importer;
  const aiScene *a_scene =
      importer.ReadFile(path, aiProcessPreset_TargetRealtime_Quality);

  if (!a_scene) {
    throw std::runtime_error("Assimp import error:" +
                             std::string(importer.GetErrorString()));
  }

  // add materials and meshes to scene
  std::vector<std::shared_ptr<StaticMesh>> meshes =
      processMeshs(a_scene, scene);
  std::vector<std::shared_ptr<Material>> materials =
      processMaterials(a_scene, scene);

  // for mesh
  processNode(nullptr, a_scene->mRootNode, a_scene, scene, meshes, materials);

  std::queue<std::pair<std::shared_ptr<TransformRelationship>, aiNode *>>
      process_queue;
  process_queue.push(std::make_pair(nullptr, a_scene->mRootNode));
  while (!process_queue.empty()) {
    auto e = process_queue.front();
    process_queue.pop();

    auto parent = e.first;
    auto pnode = e.second;
    std::shared_ptr<TransformRelationship> pre_tr_re = nullptr;
    for (auto i = 0; i < pnode->mNumChildren; ++i) {
      auto cur_tr_re = processNode(parent, pnode->mChildren[i], a_scene, scene,
                                   meshes, materials);

      if (i == 0)
        parent->child = cur_tr_re;
      else if (i != 0)
        pre_tr_re->sibling = cur_tr_re;

      pre_tr_re = cur_tr_re;
      process_queue.push(std::make_pair(cur_tr_re, pnode->mChildren[i]));
    }
  }

  // load the default camera if have
  LOGI("load scene: %s", path.c_str());
}

std::vector<std::shared_ptr<StaticMesh>>
AssimpLoader::processMeshs(const aiScene *a_scene, Scene &scene) {
  auto a_meshes = a_scene->mMeshes;
  std::vector<std::shared_ptr<StaticMesh>> ret_meshes(a_scene->mNumMeshes);
  for (auto i = 0; i < a_scene->mNumMeshes; ++i) {
    auto tmp_a_mesh = a_meshes[i];
    ret_meshes[i] = std::make_shared<StaticMesh>();

    // mesh data to static mesh data
    // vertices data: 3f_pos | 3f_normal | 2f_uv
    auto nv = tmp_a_mesh->mNumVertices;
    auto nv_data_size = nv * 3 * sizeof(float);
    auto uv_data_size = nv * 2 * sizeof(float);

    auto vb =
        std::make_shared<Buffer>(driver_, 0, nv_data_size * 2 + uv_data_size,
                                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 0,
                                 VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

    std::vector<std::byte> data(nv * (3 + 3 + 2) * sizeof(float));
    memcpy(data.data(), &tmp_a_mesh->mVertices, nv_data_size); // vertices
    memcpy(data.data() + nv_data_size, &tmp_a_mesh->mNormals,
           nv_data_size); // normals
    memcpy(data.data() + 2 * nv_data_size, &tmp_a_mesh->mTextureCoords,
           uv_data_size); // uv

    // TODO upload to gpu
    const auto stride = sizeof(float) * 8;
    ret_meshes[i]->vertices = {vb, 0, stride, nv, VK_FORMAT_R32G32B32_SFLOAT};
    ret_meshes[i]->normals = {vb, sizeof(float) * 3, stride, nv,
                              VK_FORMAT_R32G32B32_SFLOAT};
    ret_meshes[i]->texture_coords = {vb, sizeof(float) * 6, stride, nv,
                                     VK_FORMAT_R32G32_SFLOAT};

    // faces
    auto nf = tmp_a_mesh->mNumFaces;
    std::vector<uint32_t> tri_faces;
    tri_faces.reserve(nf * 3);
    for (auto j = 0; j < nf; ++j) {
      assert(tmp_a_mesh->mFaces[j].mNumIndices == 3);
      tri_faces.emplace_back(tmp_a_mesh->mFaces[j].mIndices[0]);
      tri_faces.emplace_back(tmp_a_mesh->mFaces[j].mIndices[1]);
      tri_faces.emplace_back(tmp_a_mesh->mFaces[j].mIndices[2]);
    }

    // buffer: indices data triangle faces
    auto ib = std::make_shared<Buffer>(driver_, 0, tri_faces.size()*sizeof(uint32_t),
                                 VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 0,
                                 VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    // TODO upload data to buffer
    ret_meshes[i]->faces = {
      ib, 0, static_cast<uint32_t>(tri_faces.size()), VK_INDEX_TYPE_UINT32, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
    };
  }

  return ret_meshes;
}

std::vector<std::shared_ptr<Material>>
AssimpLoader::processMaterials(const aiScene *a_scene, Scene &) {
  return {};
}

} // namespace vk_engine