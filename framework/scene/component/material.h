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

class ImageView;

struct MaterialUboParam {
  uint32_t stride{0}; // for array element in uniform buffer
  const std::type_info &tinfo;
  const uint8_t ub_offset;
  std::string name;
};

struct MaterialUbo {
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
  Material(const std::shared_ptr<VkDriver> &driver)
      : driver_(driver) {}

  virtual ~Material() = default;

  template <typename T>
  void setUboParamValue(const std::string &name, const T value,
                        uint32_t index = 0) {
    for (auto &info : ubos_info_) {
      for (auto &param : info.params) {
        if (param.name == name) {
          // do the job
          assert(param.tinfo == typeid(T));
          uint32_t param_offset = index * param.stride + param.ub_offset;
          if ((index != 0 && param.stride == 0) ||
              param_offset + sizeof(T) >= info.size)
            throw std::runtime_error("invalid ubo param index");
          memcpy(info.data.data() + param_offset, &value, sizeof(T));
          info.dirty = true;
          return;
        }
      }
    }
    throw std::runtime_error("invalid ubo param name or type");
  }

  // void setTextureParamValue(const std::string &name,
  //                           const std::string &img_file_path) {
  //   auto itr = std::find_if(texture_params_.begin(), texture_params_.end(),
  //                           [&name](const MaterialTextureParam &param) {
  //                             return param.name == name;
  //                           });
  //   if (itr == texture_params_.end())
  //     throw std::runtime_error("invalid texture param name");
  //   itr->img_file_path = img_file_path;
  //   itr->dirty = true;
  // }

  //////// helper functions
  //const std::vector<std::shared_ptr<Buffer>> &getMaterialUniformBuffers();

  bool updateParams();

  /**
   * \brief update the information(vs,fs, multisample, subpass index) to
   * pipeline state
   */
  virtual void setPipelineState(PipelineState &pipeline_state) = 0;

  std::shared_ptr<DescriptorSet> getDescriptorSet() const noexcept { return desc_set_; }

  virtual void compile() = 0;

protected:
  std::shared_ptr<ShaderModule> vs_;
  std::shared_ptr<ShaderModule> fs_;

  // std::vector<ShaderResource> shader_resources_;

  std::vector<MaterialUbo> ubos_info_;
  std::vector<std::shared_ptr<Buffer>> ubos_;

  std::shared_ptr<DescriptorSet> desc_set_;

  //std::vector<MaterialTextureParam> textures_info_;
  std::shared_ptr<VkDriver> driver_;

  //uint32_t variance_; // material variance bit flags, check by value
};

class PbrMaterial : public Material {
public:
  PbrMaterial(const std::shared_ptr<VkDriver> &driver);

  ~PbrMaterial() override = default;

  void setPipelineState(PipelineState &pipeline_state) override;

  void compile() override;
};

struct GlobalMVP {
  glm::mat4 model;
  glm::mat4 view_proj;
  glm::vec3 camera_position;
};

// set 0, binding 0
MaterialUbo globalMVPUbo();

} // namespace vk_engine