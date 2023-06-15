#include <stdexcept>

#include <framework/vk/pipeline_layout.h>

namespace vk_engine
{
  PipelineLayout::PipelineLayout(
    const std::shared_ptr<VkDriver> &driver,
    const std::vector<std::shared_ptr<ShaderModule>> &shader_modules)
  {
    // all resources statics
    for(const auto shader_module : shader_modules)
    {
      for(const auto &resource : shader_module->getResources())
      {
        std::string key = resource.name;
        // input output may have same name
        if(resource.type == ShaderResourceType::Input ||
          resource.type == ShaderResourceType::Output) {
            key = std::to_string(resource.stages) + "_" + key;
        }

        auto itr = resources_.find(key);
        if(itr != resources_.end())        
            itr->second.stages |= resource.stages;
        else resources_.emplace(key, resource);
      }
    }

    // resources for each set
    uint32_t max_set_index = 0;
    for(auto itr=resources_.begin(); itr!=resources_.end(); ++itr)
    {
      auto &resource = itr->second;
      auto &set_resources = set_resources_[resource.set];
      set_resources.emplace_back(resource);
      max_set_index = std::max(max_set_index, resource.set);
    }

    // create descriptor set layouts
    descriptor_set_layouts_.resize(max_set_index + 1);
    for(auto itr=set_resources_.begin(); itr!=set_resources_.end(); ++itr)
    {
      auto set_index = itr->first;
      auto &set_resources = itr->second;
      descriptor_set_layouts_[set_index].reset(new DescriptorSetLayout(driver, set_index, set_resources));
    }
  }
  
  PipelineLayout::~PipelineLayout()
  {
    vkDestroyPipelineLayout(driver_->getDevice(), handle_, nullptr);
  }

  const DescriptorSetLayout &PipelineLayout::getDescriptorSetLayout(const uint32_t set_index) const
  {
    auto &ptr = descriptor_set_layouts_[set_index];
    if(ptr == nullptr)
      throw std::runtime_error("invalid set index");
    return *ptr;
  }
}