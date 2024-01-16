#pragma once

#include <Eigen/Dense>

namespace vk_engine {

enum class LightType : uint32_t {
  DIRECTIONAL,
  POINT,
  SPOT,
  AREA
};

struct alignas(16) LightUb {
  LightType light_type;
  float angle;
  float blend;
  float falloff;
  //float preserved;
  alignas(16) Eigen::Vector3f position;
  alignas(16) Eigen::Vector3f direction;
  Eigen::Vector3f color;
  float intensity;
};

// filament/src/details/View.cpp --> prepare

class Light {
public:
  
  Light() = default;

  void setType(const LightType light_type) noexcept
  {
    light_ub_.light_type = light_type;
    dirty_ = true;
  }

  void setAngle(const float angle) noexcept
  {
    light_ub_.angle  = angle;
    dirty_ = true;
  }

  void setBlend(const float blend) noexcept
  {
    assert(blend<=1.0f && blend>=0.0f);
    light_ub_.blend = blend;
    dirty_ = true;
  }

  void setFallof(const float falloff) noexcept
  {
    light_ub_.falloff = falloff;
    dirty_ = true;
  }

  void setPosition(const Eigen::Vector3f &position)
  {
    light_ub_.position = position;
    dirty_ = true;
  }

  void setDirection(const Eigen::Vector3f &direction)
  {
    light_ub_.direction = direction;
    dirty_ = true;
  }

  void setColor(const Eigen::Vector3f &color)
  {
    light_ub_.color = color;
    dirty_ = true;
  }

  void setIntensity(const float intensity)
  {
    light_ub_.intensity = intensity;
    dirty_ = true;
  }
  
private:
  LightUb light_ub_;
  bool dirty_{false};
};
} // namespace vk_engine