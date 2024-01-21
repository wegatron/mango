#pragma once

#include <Eigen/Dense>
#include <framework/vk/vk_constants.h>

namespace vk_engine {

// for compatability with assimp
enum class LightType : uint32_t {  
  UNDEFINED = 0x0,
  DIRECTIONAL = 0x1,
  POINT = 0x2,
  SPOT = 0x3,
  AMBIENT = 0x4,
  AREA = 0x5
};

struct alignas(16) Light {
  LightType light_type;
  float inner_angle;
  float outer_angle;
  float intensity;
  //float preserved;
  alignas(16) Eigen::Vector3f position;
  alignas(16) Eigen::Vector3f direction;
  alignas(16) Eigen::Vector3f color;
  float falloff;  
};

struct Lights {  
  Light l[MAX_LIGHTS_COUNT];
  uint32_t lights_count{0}; 
};
// filament/src/details/View.cpp --> prepare

} // namespace vk_engine