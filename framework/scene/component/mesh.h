#pragma once

#include <framwork/vk/buffer.h>

namespace vk_engine {

struct VertexBuffer {
    std::shared_ptr<Buffer> buffer;
    uint32_t offset;
    uint32_t stride;
    uint32_t vertex_count;
    VkFormat data_type;
};

struct IndexBuffer {
    std::shared_ptr<Buffer> buffer;
    uint32_t offset;
    uint32_t index_count;
    VkIndexType data_type;
};

struct Mesh {
  VkPrimitiveTopology type;
  VertexBuffer vertices;
  IndexBuffer indices;
  size_t offset;
  size_t count;
};

} // namespace vk_engine