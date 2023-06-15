#pragma once
#include <framework/vk/descriptor_set_layout.h>
#include <framework/vk/pipeline_layout.h>
#include <framework/vk/shader_module.h>
#include <unordered_map>
#include <mutex>

namespace vk_engine {
struct ResourceCacheState {
  std::mutex shader_modules_mtx;
  std::unordered_map<size_t, std::shared_ptr<ShaderModule>>
      shader_modules; //!< hash code of shader module(stage, glsl_code) -->
                      //!< ShaderModule

  std::mutex shaders_mtx;                      
  std::unordered_map<size_t, std::shared_ptr<Shader>>
      shaders; //!< hash code of shader module --> Shader
  
  std::mutex descriptor_set_layouts_mtx;
  std::unordered_map<size_t, std::shared_ptr<DescriptorSetLayout>>
      descriptor_set_layouts;

  std::mutex pipeline_layouts_mtx;
  std::unordered_map<size_t, std::shared_ptr<PipelineLayout>> pipeline_layouts;

  VkPipelineCache pipeline_cache{VK_NULL_HANDLE};
};

class ResourceCache final {
public:
  ResourceCache() = default;
  ~ResourceCache() = default;

  ResourceCache(const ResourceCache &) = delete;
  ResourceCache &operator=(const ResourceCache &) = delete;

  ResourceCache(ResourceCache &&) = delete;
  ResourceCache &operator=(ResourceCache &&) = delete;

  std::shared_ptr<ShaderModule>
  requestShaderModule(VkShaderStageFlagBits stage,
                      const std::string &glsl_source);

  std::shared_ptr<ShaderModule>
  requestShaderModule(const std::string &file_path);

  std::shared_ptr<Shader>
  requestShader(const std::shared_ptr<VkDriver> &driver,
                const std::shared_ptr<ShaderModule> &shader_module);

  std::shared_ptr<DescriptorSetLayout>
  requestDescriptorSetLayout(const std::shared_ptr<VkDriver> &driver,
                             const size_t set_index,
                             const std::vector<ShaderResource> &resources);

  std::shared_ptr<PipelineLayout> requestPipelineLayout(
      const std::shared_ptr<VkDriver> &driver,
      const std::vector<std::shared_ptr<ShaderModule>> &shader_modules);

  VkPipelineCache getPipelineCache() const
  {
    return state_.pipeline_cache;
  }
  
  void clear();

private:
  ResourceCacheState state_;
};
} // namespace vk_engine