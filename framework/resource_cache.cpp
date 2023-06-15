#pragma once

#include <framework/resource_cache.h>
#include <glm/gtx/hash.hpp>

namespace vk_engine {

std::shared_ptr<ShaderModule>
ResourceCache::requestShaderModule(VkShaderStageFlagBits stage,
                                   const std::string &glsl_source) {
  std::unique_lock<std::mutex> lock(state_.shader_modules_mtx);                                    
  auto hash_code = ShaderModule::hash(glsl_source, stage);
  auto iter = state_.shader_modules.find(hash_code);
  if (iter != state_.shader_modules.end())
    return iter->second;

  auto shader_module = std::make_shared<ShaderModule>();
  shader_module->setGlsl(glsl_source, stage);
  state_.shader_modules[hash_code] = shader_module;
  return shader_module;
}

std::shared_ptr<ShaderModule>
ResourceCache::requestShaderModule(const std::string &file_path) {
  VkShaderStageFlagBits stage{};
  std::string glsl_code;
  ShaderModule::readGlsl(file_path, stage, glsl_code);
  return requestShaderModule(stage, glsl_code);
}

std::shared_ptr<Shader>
ResourceCache::requestShader(const std::shared_ptr<VkDriver> &driver,
                             const std::shared_ptr<ShaderModule> &shader_module) {
  std::unique_lock<std::mutex> lock(state_.shaders_mtx);                           
  auto hash_code = shader_module->getHash();
  auto iter = state_.shaders.find(hash_code);
  if (iter != state_.shaders.end())
    return iter->second;
  auto shader = std::make_shared<Shader>(driver, shader_module);
  state_.shaders[hash_code] = shader;
  return shader;
}

std::shared_ptr<DescriptorSetLayout> ResourceCache::requestDescriptorSetLayout(
    const std::shared_ptr<VkDriver> &driver, const size_t set_index,
    const std::vector<ShaderResource> &resources) {
  size_t hash_code = 0;
  for (const auto &rs : resources) {
    assert(rs.set == set_index || rs.set == 0XFFFFFFFF);
    auto tmp_hash_code = ShaderResource::hash(rs);
    glm::detail::hash_combine(hash_code, tmp_hash_code);
  }
  std::unique_lock<std::mutex> lock(state_.descriptor_set_layouts_mtx);
  auto itr = state_.descriptor_set_layouts.find(hash_code);
  if (itr != state_.descriptor_set_layouts.end())
    return itr->second;

  auto descriptor_set_layout =
      std::make_shared<DescriptorSetLayout>(driver, set_index, resources);
  state_.descriptor_set_layouts[hash_code] = descriptor_set_layout;
  return descriptor_set_layout;
}

std::shared_ptr<PipelineLayout> ResourceCache::requestPipelineLayout(
    const std::shared_ptr<VkDriver> &driver,
    const std::vector<std::shared_ptr<ShaderModule>> &shader_modules) {
  size_t hash_code = 0;
  for (const auto &shader_module : shader_modules) {
    glm::detail::hash_combine(hash_code, shader_module->getHash());
  }
  std::unique_lock<std::mutex> lock(state_.pipeline_layouts_mtx);
  auto itr = state_.pipeline_layouts.find(hash_code);
  if (itr != state_.pipeline_layouts.end())
    return itr->second;

  auto pipeline_layout =
      std::make_shared<PipelineLayout>(driver, shader_modules);
  state_.pipeline_layouts[hash_code] = pipeline_layout;
  return pipeline_layout;
}

void ResourceCache::clear() {
  std::unique_lock<std::mutex> lock(state_.shaders_mtx);
  state_.shaders.clear();

  std::unique_lock<std::mutex> lock2(state_.shader_modules_mtx);
  state_.shader_modules.clear();

  std::unique_lock<std::mutex> lock3(state_.descriptor_set_layouts_mtx);
  state_.descriptor_set_layouts.clear();

  std::unique_lock<std::mutex> lock4(state_.pipeline_layouts_mtx);  
  state_.pipeline_layouts.clear();
}
} // namespace vk_engine
