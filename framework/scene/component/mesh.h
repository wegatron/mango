#pragma once

#include <framework/vk/buffer.h>

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
    uint32_t index_count; // number of vertices
    VkIndexType data_type;
    VkPrimitiveTopology type;
};

struct StaticMesh {  
  VertexBuffer vertices;
  VertexBuffer normals;
  VertexBuffer texture_coords;
  IndexBuffer faces;
};

} // namespace vk_engine