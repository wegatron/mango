#pragma once

#include <Eigen/Dense>

namespace vk_engine {

struct alignas(16) LightUib {
  int32_t type;
  float angle;
  float blend;
  //float preserved;
  alignas(16) Eigen::Vector3f direction;
  Eigen::Vector4f color_intensity;
  Eigen::Vector4f position_falloff;
};

// filament/src/details/View.cpp --> prepare

// class Light {
// public:
// };
} // namespace vk_engine