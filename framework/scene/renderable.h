#pragma once
#include <cstddef>
#include <entt/entt.hpp>
#include <framework/scene/component/mesh.h>

namespace vk_engine {
struct RenderableBuilderInfo;
class RenderableBuilder final {
public:
  RenderableBuilder();
  ~RenderableBuilder();

  // add geometry component to renderable
  RenderableBuilder &mesh(const Mesh &m) noexcept;
 
  // add material component to renderable
  RenderableBuilder &material(size_t index, Material &mat) noexcept;

  RenderableBuilder& boundingBox(const Box& axisAlignedBoundingBox) noexcept;
  
  entt::entity build();
private:
  RenderableBuilderInfo *info_;
};
} // namespace vk_engine