#pragma once

#include <framework/resource_cache.h>

namespace vk_engine {
std::shared_ptr<Shader>
ResourceCache::requestShader(const std::shared_ptr<VkDriver> &driver,
                             VkShaderStageFlagBits stage,
                             const std::string &glsl_source) {
  auto hash_code = ShaderModule::hash(glsl_source, stage);
  auto shader_iter = state_.shaders.find(hash_code);
  if (shader_iter != state_.shaders.end())
    return shader_iter->second;

  auto sm_itr = state_.shader_modules.find(hash_code);
  std::shared_ptr<ShaderModule> shader_module = nullptr;
  if (sm_itr != state_.shader_modules.end())
    shader_module = sm_itr->second;
  else {
    shader_module = std::make_shared<ShaderModule>();
    shader_module->setGlsl(glsl_source, stage);
    state_.shader_modules[hash_code] = shader_module;
  }
  auto shader = std::make_shared<Shader>(driver, shader_module);
  state_.shaders[hash_code] = shader;
  return shader;
}

std::shared_ptr<Shader>
ResourceCache::requestShader(const std::shared_ptr<VkDriver> &driver,
                             const std::string &file_path) {
  VkShaderStageFlagBits stage{};
  std::string glsl_code;
  ShaderModule::readGlsl(file_path, stage, glsl_code);
  return requestShader(driver, stage, glsl_code);
}
} // namespace vk_engine
