#pragma once
#include <unordered_map>
#include <framework/shader_module.h>
#include <framework/descriptor_set_layout.h>
#include <framework/pipeline_layout.h>

namespace vk_engine
{
    struct ResourceCacheState
    {
        std::unordered_map<uint32_t, std::shared_ptr<ShaderResource>> resources;
        std::unordered_map<uint32_t, std::shared_ptr<DescriptorSetLayout>> descriptor_set_layouts;
        std::unordered_map<uint32_t, std::shared_ptr<PipelineLayout>> pipeline_layouts;
    };

    class ResourceCache final
    {
    public:
        ResourceCache() = default;
        ~ResourceCache() = default;

        ResourceCache(const ResourceCache &) = delete;
        ResourceCache &operator=(const ResourceCache &) = delete;

        ResourceCache(ResourceCache &&) = delete;
        ResourceCache &operator=(ResourceCache &&) = delete;

        std::shared_ptr<ShaderResource> requestShaderModule(const uint32_t hash_code);

        std::shared_ptr<DescriptorSetLayout> requestDescriptorSetLayout(const uint32_t hash_code);

        std::shared_ptr<PipelineLayout> requestPipelineLayout(const uint32_t hash_code);

        void clear();
    private:

        ResourceCacheState state_;
    };
}