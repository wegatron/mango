#pragma once

#include <memory>
#include <vector>
#include <framework/shader_module.h>
#include <framework/vk_driver.h>

namespace vk_engine
{
    class DescriptorSetLayout final
    {
    public:
        DescriptorSetLayout(
            std::shared_ptr<VkDriver> &driver,
            const uint32_t set_index,
            const std::vector<ShaderResource> &resource_set);

        DescriptorSetLayout(const DescriptorSetLayout &) = delete;
        DescriptorSetLayout(DescriptorSetLayout &&) = delete;

        DescriptorSetLayout& operator=(const DescriptorSetLayout &) = delete;
        DescriptorSetLayout& operator=(DescriptorSetLayout &&) = delete;
        
        ~DescriptorSetLayout();

        VkDescriptorSetLayout getHandle() const { return handle_; }
    private:
        VkDescriptorSetLayout handle_{VK_NULL_HANDLE};
        std::vector<VkDescriptorBindingFlagsEXT> binding_flags_;
        std::vector<VkDescriptorSetLayoutBinding> bindings_;
    };
}