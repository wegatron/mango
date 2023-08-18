#include <iostream>
#include "material.h"

namespace vk_engine {
Material::Material(const std::shared_ptr<ShaderModule> &vs,
                   const std::shared_ptr<ShaderModule> &fs) {
  auto shader_resources = parseShaderResources({vs, fs});
  for (const auto &resource : shader_resources) {
    switch (resource.type) {
    case ShaderResourceType::BufferUniform:
      std::cout << "ubo" << std::endl;
      // parse ubo param
      // resource.set
      break;
    case ShaderResourceType::ImageSampler:
    case ShaderResourceType::Image:
        for (uint32_t i = 0; i < resource.array_size; ++i)
            texture_params_.emplace_back(MaterialTextureParam{resource.set, resource.binding, i,
                                         resource.name, "", false});
      break;
    default:
      break;
    }
  }
}

void Material::update2PipelineState(PipelineState &pipeline_state)
{

}

void Material::updateParam2DescriptorSet(std::vector<VkWriteDescriptorSet> &write_desc_sets)
{

}

} // namespace vk_engine