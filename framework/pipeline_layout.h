#pragma once

#include <volk.h>
#include <map>
#include <framework/vk_driver.h>
#include <framework/shader_module.h>
#include <framework/descriptor_set_layout.h>

namespace vk_engine {
class PipelineLayout final {
public:
  PipelineLayout(std::shared_ptr<VkDriver> &driver,
    const std::vector<ShaderSource *> &shader_modules);

  ~PipelineLayout() = default;

  PipelineLayout(const PipelineLayout &) = delete;
  PipelineLayout &operator=(const PipelineLayout &) = delete;

  PipelineLayout(PipelineLayout &&) = delete;
  PipelineLayout &operator=(PipelineLayout &&) = delete;

  VkPipelineLayout getHandle() const { return handle_; }

  const DescriptorSetLayout &getDescriptorSetLayout(const uint32_t set_index) const;

private:
  std::shared_ptr<VkDriver> driver_;
  
  // all resources statics
  std::map<std::string, ShaderResource> resources_;
	
  // A map of each set and the resources it owns used by the pipeline layout
	std::map<uint32_t, std::vector<ShaderResource>> set_resources_;

  std::vector<DescriptorSetLayout *> descriptor_set_layouts_;

  VkPipelineLayout handle_{VK_NULL_HANDLE};
};
} // namespace vk_engine