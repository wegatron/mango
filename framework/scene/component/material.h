#pragma once
#include <glm/glm.hpp>
#include <map>
#include <stdexcept>
#include <list>
#include <vulkan/vulkan.h>

#include <framework/vk/buffer.h>
#include <framework/vk/descriptor_set.h>
#include <framework/vk/pipeline_state.h>
#include <framework/vk/shader_module.h>
#include <framework/vk/vk_constants.h>
#include <framework/vk/vk_driver.h>


namespace vk_engine {

class GraphicsPipeline;
class ImageView;

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
  bool dirty;
};

struct MatParamsSet {
  size_t mat_hash_id{0};
  std::unique_ptr<Buffer> ubo;
  // textures
  std::shared_ptr<DescriptorSet> desc_set;
  mutable uint32_t last_access{0};
};

class Material;

class MatGpuResourcePool {
public:
  MatGpuResourcePool() = default;
  
  void gc();

  std::shared_ptr<GraphicsPipeline>
  requestGraphicsPipeline(const std::shared_ptr<Material> &mat);
  
  std::shared_ptr<DescriptorSet>
  requestMatDescriptorSet(const std::shared_ptr<Material> &mat);

private:
  std::map<uint32_t, std::shared_ptr<GraphicsPipeline>> mat_pipelines_;
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

  bool updateParams();

  /**
   * \brief update the information(vs,fs, multisample, subpass index) to
   * pipeline state
   */
  virtual void setPipelineState(PipelineState &pipeline_state) = 0;

  virtual void compile() = 0;

  uint32_t getHashId() const { return hash_id_; }

protected:
  std::shared_ptr<ShaderModule> vs_;
  std::shared_ptr<ShaderModule> fs_;

  // std::vector<ShaderResource> shader_resources_;

  MaterialUboInfo ubo_info_;
  
  std::shared_ptr<MatParamsSet> mat_param_set_;
    
  uint32_t hash_id_{0};

  // uint32_t variance_; // material variance bit flags, check by value
};

class PbrMaterial : public Material {
public:
  PbrMaterial();

  ~PbrMaterial() override = default;

  void setPipelineState(PipelineState &pipeline_state) override;

  void compile() override;
};

// struct GlobalMVP {
//   glm::mat4 model;
//   glm::mat4 view_proj;
//   glm::vec3 camera_position;
// };

// set 0, binding 0
// MaterialUbo globalMVPUbo();

} // namespace vk_engine