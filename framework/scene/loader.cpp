#include <framework/scene/loader.h>

#include <Eigen/Dense>
#include <queue>

#include <framework/scene/asset_manager.hpp>
#include <framework/utils/logging.h>
#include <framework/vk/commands.h>
#include <iostream>

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
  return cur_tr;
}

void AssimpLoader::loadScene(const std::string &path, Scene &scene,
                             const std::shared_ptr<CommandBuffer> &cmd_buf) {
  Assimp::Importer importer;
  const aiScene *a_scene =
      importer.ReadFile(path, aiProcessPreset_TargetRealtime_Quality);

  if (!a_scene) {
    throw std::runtime_error("Assimp import error:" +
                             std::string(importer.GetErrorString()));
  }
  file_directory_ = path.substr(0, path.find_last_of('/'));
  // add materials and meshes to scene
  std::vector<std::shared_ptr<StaticMesh>> meshes =
      processMeshs(a_scene, scene, cmd_buf);
  std::vector<std::shared_ptr<Material>> materials =
      processMaterials(a_scene, scene);

  // process root node's mesh
  processNode(nullptr, a_scene->mRootNode, a_scene, scene, meshes, materials);

  std::queue<std::pair<std::shared_ptr<TransformRelationship>, aiNode *>>
      process_queue;
  process_queue.push(std::make_pair(nullptr, a_scene->mRootNode));
  while (!process_queue.empty()) {
    auto e = process_queue.front();
    process_queue.pop();

    auto parent_tr = e.first;
    auto pnode = e.second;
    std::shared_ptr<TransformRelationship> pre_tr_re = nullptr;
    for (auto i = 0; i < pnode->mNumChildren; ++i) {
      // process children's mesh
      auto cur_tr_re = processNode(parent_tr, pnode->mChildren[i], a_scene,
                                   scene, meshes, materials);

      if (i == 0)
        parent_tr->child = cur_tr_re;
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
AssimpLoader::processMeshs(const aiScene *a_scene, Scene &scene,
                           const std::shared_ptr<CommandBuffer> &cmd_buf) {
  auto a_meshes = a_scene->mMeshes;
  std::vector<std::shared_ptr<StaticMesh>> ret_meshes(a_scene->mNumMeshes);
  for (auto i = 0; i < a_scene->mNumMeshes; ++i) {
    auto tmp_a_mesh = a_meshes[i];
    ret_meshes[i] = std::make_shared<StaticMesh>();

    // mesh data to static mesh data
    // vertices data: 3f_pos | 3f_normal | 2f_uv
    auto nv = tmp_a_mesh->mNumVertices;
    auto driver = getDefaultAppContext().driver;
    auto vb = std::make_shared<Buffer>(driver, 0, nv * 8 * sizeof(float),
                                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                           VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                       0, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    std::vector<float> data(nv * 8);
    static_assert(std::is_same<ai_real, float>::value,
                  "Type should be same while using memory copy.");

    memcpy(data.data(), &tmp_a_mesh->mVertices,
           nv * 3 * sizeof(float)); // vertices
    memcpy(data.data() + nv * 3, &tmp_a_mesh->mNormals,
           nv * 3 * sizeof(float)); // normals
    for (auto vi = 0; vi < nv; ++vi) {
      data[6 * nv + vi * 2] = tmp_a_mesh->mTextureCoords[0][vi].x;
      data[6 * nv + vi * 2 + 1] = tmp_a_mesh->mTextureCoords[0][vi].y;
    }

    auto stage_pool = getDefaultAppContext().stage_pool;
    // upload to gpu
    vb->updateByStaging(data.data(), data.size() * sizeof(float), 0, stage_pool,
                        cmd_buf);
    const auto stride = sizeof(float) * 8;
    ret_meshes[i]->vertices = {vb, 0, stride, nv, VK_FORMAT_R32G32B32_SFLOAT};
    ret_meshes[i]->normals = {vb, sizeof(float) * 3, stride, nv,
                              VK_FORMAT_R32G32B32_SFLOAT};
    ret_meshes[i]->texture_coords = {vb, sizeof(float) * 6, stride, nv,
                                     VK_FORMAT_R32G32_SFLOAT};

    // faces
    auto nf = tmp_a_mesh->mNumFaces;
    std::vector<uint32_t> tri_v_inds;
    tri_v_inds.reserve(nf * 3);
    for (auto j = 0; j < nf; ++j) {
      assert(tmp_a_mesh->mFaces[j].mNumIndices == 3);
      tri_v_inds.emplace_back(tmp_a_mesh->mFaces[j].mIndices[0]);
      tri_v_inds.emplace_back(tmp_a_mesh->mFaces[j].mIndices[1]);
      tri_v_inds.emplace_back(tmp_a_mesh->mFaces[j].mIndices[2]);
    }

    // buffer: indices data triangle faces
    auto ib = std::make_shared<Buffer>(
        driver, 0, tri_v_inds.size() * sizeof(uint32_t),
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 0,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
    // upload data to buffer
    ib->updateByStaging(tri_v_inds.data(), tri_v_inds.size() * sizeof(uint32_t),
                        0, stage_pool, cmd_buf);
    ret_meshes[i]->faces = {ib, 0, static_cast<uint32_t>(tri_v_inds.size()),
                            VK_INDEX_TYPE_UINT32,
                            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
  }

  // add barrier to make sure transfer is complete before rendering
  VkMemoryBarrier memoryBarrier = {};
  memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
  memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  memoryBarrier.dstAccessMask =
      VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT;

  vkCmdPipelineBarrier(cmd_buf->getHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 1, &memoryBarrier,
                       0, nullptr, 0, nullptr);

  return ret_meshes;
}

void AssimpLoader::loadAndSet(aiMaterial *a_mat, aiTextureType ttype,
                              const char *pKey, unsigned int vtype,
                              unsigned int idx, const char *shader_texture_name,
                              const char *shader_color_name,
                              std::shared_ptr<PbrMaterial> &mat) {
  aiString texture_path;
  if (AI_SUCCESS == a_mat->GetTexture(ttype, 0, &texture_path)) {
    // mat->setTextureParamValue(shader_texture_name,
    //                           file_directory_ +
    //                               std::string(texture_path.C_Str()));
    // getDefaultAppContext().gpu_asset_manager->requestImage<vk_engine::Image>(file_directory_
    // + std::string(texture_path.C_Str()));
  } else {
    aiColor3D diffuse_color(0.0f, 0.0f, 0.0f);
    a_mat->Get(pKey, vtype, idx, diffuse_color);
    mat->setUboParamValue(
        shader_color_name,
        glm::vec4(diffuse_color.r, diffuse_color.g, diffuse_color.b, 1.0));
  }
}

std::vector<std::shared_ptr<Material>>
AssimpLoader::processMaterials(const aiScene *a_scene, Scene &) {
  auto num_materials = a_scene->mNumMaterials;
  std::vector<std::shared_ptr<Material>> ret_mats(num_materials);

  auto driver = getDefaultAppContext().driver;
  auto gpu_asset_manager = getDefaultAppContext().gpu_asset_manager;

  for (auto i = 0; i < num_materials; ++i) {
    auto a_mat = a_scene->mMaterials[i];
    auto cur_mat = std::make_shared<PbrMaterial>(driver);
    ret_mats.emplace_back(cur_mat);

    aiString texturePath;
    if (a_mat->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
        std::cout << "Diffuse texture path: " << texturePath.C_Str() << std::endl;
        // Now, you can load and use the texture with your graphics library (e.g., OpenGL or DirectX).
    }    

    // diffuse, base color
    loadAndSet(a_mat, aiTextureType_DIFFUSE, AI_MATKEY_COLOR_DIFFUSE,
               "base_color_texture", "pbr_mat.base_color", cur_mat);
/*     loadAndSet(a_mat, aiTextureType_SPECULAR, AI_MATKEY_COLOR_SPECULAR,
              "specular_color_texture", "pbr_mat.specular_color", cur_mat);
    loadAndSet(a_mat, aiTextureType_DIFFUSE_ROUGHNESS, AI_MATKEY_ROUGHNESS_FACTOR,
              "roughness_texture", "pbr_mat.roughness", cur_mat);
    loadAndSet(a_mat, aiTextureType_METALNESS, AI_MATKEY_METALLIC_FACTOR,
              "metallic_texture", "pbr_mat.metallic", cur_mat); */
  }
  return ret_mats;
}

} // namespace vk_engine