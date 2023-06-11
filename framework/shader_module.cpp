#include "shader_module.h"
#include <cassert>
#include <fstream>
#include <functional>
#include <glslang/SPIRV/GLSL.std.450.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <glm/gtx/hash.hpp>

#include <volk.h>
#include <framework/logging.h>
#include <framework/spirv_reflection.h>

namespace vk_engine {
void ShaderModule::load(const std::string &file_path) {
  readGlsl(file_path, stage_, glsl_code_);
  setGlsl(glsl_code_, stage_);
}

void ShaderModule::setGlsl(const std::string &glsl_code,
                           VkShaderStageFlagBits stage) {
  glsl_code_ = glsl_code;
  stage_ = stage;

  compile2spirv(glsl_code_, stage_, spirv_code_);

  // update shader resources
  SPIRVReflection spirv_reflection;

  // Reflect all shader resouces
  if (!spirv_reflection.reflect_shader_resources(stage_, spirv_code_,
                                                 resources_)) {
    throw std::runtime_error("Failed to reflect shader resources");
  }

  // update hash code
  hash_code_ = hash(glsl_code_, stage_);
}

EShLanguage findShaderLanguage(VkShaderStageFlagBits stage);

void ShaderModule::compile2spirv(
  const std::string &glsl_code,
  VkShaderStageFlagBits stage,
  std::vector<uint32_t> &spirv_code) {
  // Initialize glslang library.
  glslang::InitializeProcess();

  // TODO add support for shader varient
  EShLanguage lang = findShaderLanguage(stage);
  glslang::TShader shader(lang);
  const char *file_name_list[1] = {""};
  const char *shader_source = glsl_code.data();
  shader.setStringsWithLengthsAndNames(&shader_source, nullptr, file_name_list,
                                       1);
  shader.setEntryPoint("main");
  shader.setSourceEntryPoint("main");
  EShMessages messages = static_cast<EShMessages>(
      EShMsgDefault | EShMsgVulkanRules | EShMsgSpvRules);

  if (!shader.parse(GetDefaultResources(), 100, false, messages)) {
    auto error_msg = std::string(shader.getInfoLog()) + "\n" +
                     std::string(shader.getInfoDebugLog());
    throw std::runtime_error("compile glsl to spirv error: " + error_msg);
  }
  // Add shader to new program object.
  glslang::TProgram program;
  program.addShader(&shader);

  if (!program.link(messages)) {
    auto error_msg = std::string(program.getInfoLog()) + "\n" +
                     std::string(program.getInfoDebugLog());
    throw std::runtime_error("link program error: " + error_msg);
  }

  auto shader_log = shader.getInfoLog();
  if (shader_log)
    LOGI(shader_log);
  auto program_log = program.getInfoLog();
  if (program_log)
    LOGI(program_log);

  glslang::TIntermediate *intermediate = program.getIntermediate(lang);
  if (!intermediate) {
    throw std::runtime_error("failed to get shader intermediate code");
  }

  spv::SpvBuildLogger logger;
  glslang::GlslangToSpv(*intermediate, spirv_code, &logger);
  LOGI(logger.getAllMessages());
  glslang::FinalizeProcess();
}


void ShaderModule::readGlsl(const std::string &file_path, VkShaderStageFlagBits &stage, std::string &glsl_code)
{
  auto len = file_path.length();
  if(len <= 4)
    throw std::runtime_error("invalid shader file path");
  
  if(file_path.compare(len-4, 4, ".vert"))
  {
    stage = VK_SHADER_STAGE_VERTEX_BIT;
  }
  else if(file_path.compare(len-4, 4, ".frag"))
  {
    stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  }
  else if(file_path.compare(len-4, 4, ".comp"))
  {
    stage = VK_SHADER_STAGE_COMPUTE_BIT;
  } else {
    throw std::runtime_error("invalid shader file path post fix, only support .vert, .frag, .comp");
  }

  // read glsl code
  std::ifstream ifs(file_path, std::ifstream::binary);
  if (!ifs)
    throw std::runtime_error("can't open file " + file_path);
  ifs.seekg(0, std::ios::end);
  size_t size = ifs.tellg();
  ifs.seekg(0, std::ios::beg);
  glsl_code.resize(size);
  ifs.read(reinterpret_cast<char *>(glsl_code.data()), size);
  ifs.close();  
}

size_t ShaderModule::hash(const std::string &glsl_code,
                    VkShaderStageFlagBits stage) noexcept
{
  auto hash_code = std::hash<VkShaderStageFlagBits>{}(stage);
  auto value = std::hash<std::string>{}(glsl_code);
  glm::detail::hash_combine(hash_code, value);
  return hash_code;
}


Shader::Shader(const std::shared_ptr<VkDriver> &driver, const std::shared_ptr<ShaderModule> &shader_module)
{
  driver_ = driver;
  shader_module_ = shader_module;

  VkShaderModuleCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = shader_module_->getSpirv().size() * sizeof(uint32_t);
  create_info.pCode = shader_module_->getSpirv().data();

  if (vkCreateShaderModule(driver_->getDevice(), &create_info, nullptr, &handle_) != VK_SUCCESS) {
    throw std::runtime_error("failed to create shader module!");
  }
}


// helper functions
EShLanguage findShaderLanguage(VkShaderStageFlagBits stage) {
  switch (stage) {
  case VK_SHADER_STAGE_VERTEX_BIT:
    return EShLangVertex;

  case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
    return EShLangTessControl;

  case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
    return EShLangTessEvaluation;

  case VK_SHADER_STAGE_GEOMETRY_BIT:
    return EShLangGeometry;

  case VK_SHADER_STAGE_FRAGMENT_BIT:
    return EShLangFragment;

  case VK_SHADER_STAGE_COMPUTE_BIT:
    return EShLangCompute;

  case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
    return EShLangRayGen;

  case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
    return EShLangAnyHit;

  case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
    return EShLangClosestHit;

  case VK_SHADER_STAGE_MISS_BIT_KHR:
    return EShLangMiss;

  case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
    return EShLangIntersect;

  case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
    return EShLangCallable;

  default:
    return EShLangVertex;
  }
}
} // namespace vk_engine