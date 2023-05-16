#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <volk.h>


namespace vk_engine
{
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

        void setSource(const std::string &code);

    private:

        void compile2spirv();

        size_t hash_code_{0};
        std::string glsl_code_;
        std::vector<uint32_t> spirv_code_;
    };
}