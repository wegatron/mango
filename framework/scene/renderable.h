#pragma once
#include <cstddef>
#include <entt/entt.hpp>

namespace vk_engine {
struct RenderableBuilderInfo;
class RenderableBuilder final {
public:
  RenderableBuilder();
  ~RenderableBuilder();

  // add geometry component to renderable
  RenderableBuilder &geometry(size_t index, PrimitiveType type,
                              VertexBuffer *vertices, IndexBuffer *indices,
                              size_t offset, size_t minIndex, size_t maxIndex,
                              size_t count) noexcept;
 
  // add material component to renderable
  RenderableBuilder &material(size_t index, MaterialInstance const* materialInstance) noexcept;

  RenderableBuilder& boundingBox(const Box& axisAlignedBoundingBox) noexcept;
  
  entt::entity build();
private:
  RenderableBuilderInfo *info_;
};
} // namespace vk_engine