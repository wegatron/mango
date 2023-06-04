#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <volk.h>


namespace vk_engine
{

    /// A bitmask of qualifiers applied to a resource
    struct ShaderResourceQualifiers
    {
        enum : uint32_t
        {
            None        = 0,
            NonReadable = 1,
            NonWritable = 2,
        };
    };

    /// Types of shader resources
    enum class ShaderResourceType
    {
        Input,
        InputAttachment,
        Output,
        Image,
        ImageSampler,
        ImageStorage,
        Sampler,
        BufferUniform,
        BufferStorage,
        PushConstant,
        SpecializationConstant,
        All
    };

    /// This determines the type and method of how descriptor set should be created and bound
    enum class ShaderResourceMode
    {
        Static,
        Dynamic,
        UpdateAfterBind
    };

    struct ShaderResource
    {
        VkShaderStageFlags stages;

        ShaderResourceType type;

        ShaderResourceMode mode;

        uint32_t set;

        uint32_t binding;

        uint32_t location;

        uint32_t input_attachment_index;

        uint32_t vec_size;

        uint32_t columns;

        uint32_t array_size;

        uint32_t offset;

        uint32_t size;

        uint32_t constant_id;

        uint32_t qualifiers;

        std::string name;
    };

    class ShaderSource
    {
    public:
        ShaderSource() = default;

        /**
         * @brief load shader file
         * .vert for vertex shader glsl
         * .frag for fragment shader glsl
         * .comp for computer shader glsl
         * .vert.spv for vertex shader spirv
         * .frag.spv for fragment shader spirv
         * .comp.spv for computer shader spirv
         * @param file_path 
         */
        void load(const std::string &file_path);

        /**
         * @brief Set the Glsl object and do precompile         
         */
        void setGlsl(const std::string &glsl_code, VkShaderStageFlagBits stage);

        const std::string &getGlsl() const noexcept { return glsl_code_; }

        const std::vector<uint32_t> &getSpirv() const noexcept { return spirv_code_; }

        size_t getHash() const noexcept { return hash_code_; }

        const std::vector<ShaderResource> &getResources() const noexcept { return resources_; }

    private:

        void compile2spirv();
        
        size_t hash_code_{0};
        VkShaderStageFlagBits stage_;
        std::string glsl_code_;
        std::vector<uint32_t> spirv_code_;
        std::vector<ShaderResource> resources_;
    };
}