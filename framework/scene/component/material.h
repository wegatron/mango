#pragma once
#include <map>
#include <stdexcept>
#include <list>
#include <algorithm>
#include <string_view>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <framework/vk/buffer.h>
#include <framework/vk/descriptor_set.h>
#include <framework/vk/pipeline_state.h>
#include <framework/vk/shader_module.h>
#include <framework/vk/vk_constants.h>
#include <framework/vk/vk_driver.h>


namespace vk_engine {

// using uint64_t to define a material type code, the higher 16 bit for Basic Material type, and the lower 16 bits for variant input.
#define PBR_MATERIAL 1u<<16

constexpr char const * BASE_COLOR_NAME = "mat.base_color";
constexpr char const * METALLIC_NAME = "mat.metallic";
constexpr char const * ROUGHNESS_NAME = "mat.roughness";
constexpr char const * NORMAL_NAME = "mat.normal";

constexpr char const * BASE_COLOR_TEXTURE_NAME = "base_color_tex";
constexpr char const * METALLIC_TEXTURE_NAME = "metallic_tex";
constexpr char const * NORMAL_TEXTURE_NAME = "normal_tex";

class GraphicsPipeline;
class ImageView;
class RenderPass;
class Sampler;

struct MaterialUboParam {
  uint32_t stride{0}; // for array element in uniform buffer
  const std::type_info &tinfo;
  const uint8_t ub_offset;
  std::string name;
};

struct MaterialUboInfo {
  uint32_t set;
  uint32_t binding;
  uint32_t size;
  bool dirty{false};
  std::vector<std::byte> data;
  std::vector<MaterialUboParam> params;
};

struct MaterialTextureParam {
  uint32_t set;
  uint32_t binding;
  uint32_t index; // for array texture
  std::string name;
  std::shared_ptr<vk_engine::ImageView> img_view;
  std::shared_ptr<vk_engine::Sampler> sampler;
  bool dirty;
};

struct MatParamsSet {
  uint32_t mat_type_id{0};
  std::unique_ptr<Buffer> ubo;
  std::shared_ptr<DescriptorSet> desc_set;
};

class Material;

/**
 * \brief MatGpuResourcePool is a gpu resource pool for material.
 * It manages GraphicsPipeline, MatParamsSet(uniform buffer + DescriptorSet)
 * 
 * the gc function should be called onece per frame
*/
class MatGpuResourcePool {
public:
  MatGpuResourcePool(VkFormat color_format, VkFormat ds_format);
  
  void gc();

  std::shared_ptr<GraphicsPipeline>
  requestGraphicsPipeline(const std::shared_ptr<Material> &mat);
  
  std::shared_ptr<DescriptorSet>
  requestMatDescriptorSet(const std::shared_ptr<Material> &mat);

private:
  std::shared_ptr<RenderPass> default_render_pass_;
  std::map<uint32_t, std::shared_ptr<GraphicsPipeline>> mat_pipelines_;
  std::unique_ptr<DescriptorPool> desc_pool_;
  std::list<std::shared_ptr<MatParamsSet>> used_mat_params_set_;
  std::list<std::shared_ptr<MatParamsSet>> free_mat_params_set_;
};

/**
 * \brief Material defines the texture of the rendered object,
 * specifying the shaders in the rendering pipeline, as well as its related
 * rendering states, inputs, and so on
 *
 * One can construct material input parameters buffer/push const using Material
 * interface.
 */
class Material {
public:
  Material() = default;

  virtual ~Material() = default;

  template <typename T>
  void setUboParamValue(const std::string &name, const T value,
                        uint32_t index = 0) 
  {
    for (auto &param : ubo_info_.params) {
      if (param.name == name) {
        // do the job
        assert(param.tinfo == typeid(T));
        uint32_t param_offset = index * param.stride + param.ub_offset;
        if ((index != 0 && param.stride == 0) ||
            param_offset + sizeof(T) >= ubo_info_.size)
          throw std::runtime_error("invalid ubo param index");
        memcpy(ubo_info_.data.data() + param_offset, &value, sizeof(T));
        ubo_info_.dirty = true;
        return;
      }
    }
    throw std::runtime_error("invalid ubo param name or type");
  }

  void setTexture(const std::string &name, const std::shared_ptr<ImageView> &img_view, uint32_t index = 0)
  {
    auto itr = std::find_if(texture_params_.begin(), texture_params_.end(),
                 [&name, &index](const MaterialTextureParam &param) {
                   return param.name == name && param.index == index;
                 });
    assert(itr != texture_params_.end());
    itr->img_view = img_view;
    itr->dirty = true;
  }

  std::vector<MaterialTextureParam> &textureParams() {
    return texture_params_;
  }

  void updateParams();

  /**
   * \brief update the information(vs,fs, multisample, subpass index) to
   * pipeline state
   */
  virtual void setPipelineState(PipelineState &pipeline_state) = 0;

  virtual void compile() = 0;

  uint32_t materialTypeId() const { return material_type_id_; }

protected:

  virtual std::shared_ptr<MatParamsSet> createMatParamsSet(
      const std::shared_ptr<VkDriver> &driver,
      DescriptorPool &desc_pool) = 0;
  
  std::shared_ptr<ShaderModule> vs_;
  std::shared_ptr<ShaderModule> fs_;

  // std::shared_ptr<PipelineState> pipeline_state_;
  // std::vector<ShaderResource> shader_resources_;

  MaterialUboInfo ubo_info_; //!< params info(specification、cpu data) of material store in uniform buffer

  std::vector<MaterialTextureParam> texture_params_; //!< texture params of material
  
  std::shared_ptr<MatParamsSet> mat_param_set_;
  std::unique_ptr<DescriptorSetLayout> desc_set_layout_;
    
  uint32_t material_type_id_{0};

  friend class MatGpuResourcePool;  
  // uint32_t variance_; // material variance bit flags, check by value
};

class PbrMaterial : public Material {
public:

  /**
   * \brief Constructor of PBR material defines all possible input parameters/textures during construction.
   * During compilation, the corresponding descriptor layout is set based on the configured inputs.
  */
  PbrMaterial();

  ~PbrMaterial() override = default;

  void setPipelineState(PipelineState &pipeline_state) override;

  void compile() override;

protected:

  /**
   * \brief Create paramset including material's uniform buffer and material's descriptor.
   * 
   * This will invoke when requestDescriptor for rendering.
   * For different variations, we keep the uniform buffer of the material unchanged, even though some parameters may not be used.
   * An image is considered an external input parameter for textures, unlike the uniform buffer, and is not created by the Material.
  */
  std::shared_ptr<MatParamsSet> createMatParamsSet(
        const std::shared_ptr<VkDriver> &driver,
        DescriptorPool &desc_pool) override;
};

} // namespace vk_engine