#pragma once
#include <framework/vk/shader_module.h>
#include <framework/vk/pipeline_state.h>

namespace vk_engine
{

struct MaterialUboParam
{    
    const std::type_info type_info;
    const uint8_t ub_offset;
    std::string name;
};

struct MaterialUbo
{
    uint32_t set;
    uint32_t binding;
    bool dirty{false};
    std::byte *data;
    std::vector<MaterialUboParam> params;
};

struct MaterialTextureParam
{
    uint32_t set;
    uint32_t binding;
    uint32_t index; // for array texture
    std::string name;
    std::string img_file_path;
    bool dirty;
};

/**
 * \brief Material defines the texture of the rendered object, 
 * specifying the shaders in the rendering pipeline, as well as its related rendering states, inputs, and so on
 * 
 * One can construct material input parameters buffer/push const using Material interface.
*/
class Material
{
public:

    Material(const std::shared_ptr<ShaderModule> &vs, const std::shared_ptr<ShaderModule> &fs);
    
    template<typename T>
    void setUboParamValue(const std::string &name, const T value)
    {
        // TODO

        // auto itr = std::find_if(ubos_.begin(), ubos_.end(), [&name](const MaterialUbo &ubo) {
        //     return param.name == name;
        // });
        // if(itr == ubo_params_.end()  || itr->type_info != typeid(T))
        //     throw std::runtime_error("invalid ubo param name or type");
        // memcpy(ubo_data_.data() + itr->ub_offset, &value, sizeof(T));
        // ubo_dirty_ = true;
    }

    void setTextureParamValue(const std::string &name, const std::string &img_file_path)
    {
        auto itr = std::find_if(texture_params_.begin(), texture_params_.end(), [&name](const MaterialTextureParam &param) {
            return param.name == name;
        });
        if(itr == texture_params_.end())
            throw std::runtime_error("invalid texture param name");
        itr->img_file_path = img_file_path;
        itr->dirty = true;
    }

    /**
     * \brief update the information(vs,fs,shader resources) to pipeline state
     */
    void update2PipelineState(PipelineState &pipeline_state);

    void updateParam2DescriptorSet(std::vector<VkWriteDescriptorSet> &write_desc_sets);

private:
    std::shared_ptr<ShaderModule> vs_;
    std::shared_ptr<ShaderModule> fs_;
    
    std::vector<MaterialUbo> ubos_;
    std::vector<std::byte> ubo_data_;
    
    std::vector<MaterialTextureParam> texture_params_;
};
}