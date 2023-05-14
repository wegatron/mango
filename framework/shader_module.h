#pragma once
#include <string>

namespace vk_engine
{
    class ShaderSource
    {
    public:
        ShaderSource() = default;
        ShaderSource(const std::string &file_path);

        const std::string &getSource() const noexcept { return code_; }

        size_t getId() const noexcept { return id_; }

        void setSource(const std::string &code);

    private:
        size_t id_;
        std::string code_;
    };

    class ShaderModule
    {
    public:
        ShaderModule();
    };
}