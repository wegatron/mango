#pragma once
#include <framework/vk/descriptor_set_layout.h>
#include <framework/vk/pipeline_layout.h>
#include <framework/vk/render_pass.h>
#include <framework/vk/shader_module.h>
#include <mutex>
#include <unordered_map>

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

  std::mutex render_pass_mtx;
  std::unordered_map<size_t, std::shared_ptr<RenderPass>> render_passes;

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

  std::shared_ptr<RenderPass>
  requestRenderPass(const std::shared_ptr<VkDriver> &driver,
                    std::vector<Attachment> attachments,
                    std::vector<LoadStoreInfo> load_store_infos,
                    std::vector<SubpassInfo> subpasses);

  VkPipelineCache getPipelineCache() const { return state_.pipeline_cache; }

  void clear();

private:
  ResourceCacheState state_;
};
} // namespace vk_engine