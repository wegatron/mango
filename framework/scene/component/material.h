#pragma once
#include <stdexcept>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <framework/vk/buffer.h>
#include <framework/vk/descriptor_set.h>
#include <framework/vk/pipeline_state.h>
#include <framework/vk/shader_module.h>
#include <framework/vk/vk_driver.h>

/**
 * set-0 for engine-global resource,
 * set-1 for per pass resource,
 * set-2 for material resource, for material only store this set
 * set-3 for per-object resource.
 */
constexpr uint32_t GLOBAL_SET_INDEX = 0;
constexpr uint32_t PER_PASS_SET_INDEX = 1;
constexpr uint32_t MATERIAL_SET_INDEX = 2;
constexpr uint32_t PER_OBJECT_SET_INDEX = 3;

namespace vk_engine {

class GPUAssetManager;

struct MaterialUboParam {
  uint32_t stride{0}; // for array element in uniform buffer
  const std::type_info &type_info;
  const uint8_t ub_offset;
  std::string name;
};

struct MaterialUbo {
  uint32_t set;
  uint32_t binding;
  uint32_t size;
  bool dirty{false};
  std::byte *data{nullptr};
  std::vector<MaterialUboParam> params;
};

struct MaterialTextureParam {
  uint32_t set;
  uint32_t binding;
  uint32_t index; // for array texture
  std::string name;
  std::string img_file_path;
  bool dirty;
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
  Material(const std::shared_ptr<VkDriver> &driver,
           const std::shared_ptr<GPUAssetManager> &gpu_asset_manager)
      : driver_(driver), gpu_asset_manager_(gpu_asset_manager) {}

  virtual ~Material() = default;

  template <typename T>
  void setUboParamValue(const std::string &name, const T value,
                        uint32_t index = 0) {
    for (auto &ubo : ubos_) {
      for (auto &param : ubo.params) {
        if (param.name == name) {
          // do the job
          uint32_t param_offset = index * param.stride + param.ub_offset;
          if ((index != 0 && param.stride == 0) ||
              param_offset + sizeof(T) >= ubo.size)
            throw std::runtime_error("invalid ubo param index");
          memcpy(ubo.data + param_offset, &value, sizeof(T));
          ubo.dirty = true;
          return;
        }
      }
    }
    throw std::runtime_error("invalid ubo param name or type");
  }

  void setTextureParamValue(const std::string &name,
                            const std::string &img_file_path) {
    auto itr = std::find_if(texture_params_.begin(), texture_params_.end(),
                            [&name](const MaterialTextureParam &param) {
                              return param.name == name;
                            });
    if (itr == texture_params_.end())
      throw std::runtime_error("invalid texture param name");
    itr->img_file_path = img_file_path;
    itr->dirty = true;
  }

  //////// helper functions
  std::vector<std::shared_ptr<Buffer>> createMaterialUniformBuffers();

  /**
   * \brief update the information(vs,fs, multisample, subpass index) to
   * pipeline state
   */
  virtual void update2PipelineState(PipelineState &pipeline_state) = 0;

  /**
   * \brief update the material descriptor sets which index is
   * MATERIAL_SET_INDEX
   */
  void
  updateDescriptorSets(VkDescriptorSet descriptor_set,
                       std::vector<std::shared_ptr<Buffer>> &material_ubos);

protected:
  std::shared_ptr<ShaderModule> vs_;
  std::shared_ptr<ShaderModule> fs_;

  std::vector<ShaderResource> shader_resources_;

  std::vector<MaterialUbo> ubos_;
  std::vector<std::byte> ubo_data_;

  std::vector<MaterialTextureParam> texture_params_;
  std::shared_ptr<VkDriver> driver_;
  std::shared_ptr<GPUAssetManager> gpu_asset_manager_;
};

class PbrMaterial : public Material {
public:
  PbrMaterial(const std::shared_ptr<VkDriver> &driver,
              const std::shared_ptr<GPUAssetManager> &gpu_asset_manager);

  ~PbrMaterial() override = default;

  void update2PipelineState(PipelineState &pipeline_state) override;
};

struct GlobalMVP {
  glm::mat4 model;
  glm::mat4 view_proj;
  glm::vec3 camera_position;
};

// set 0, binding 0
MaterialUbo globalMVPUbo();

} // namespace vk_engine