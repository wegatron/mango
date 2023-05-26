#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace vk_engine
{
    struct VertexInputState
    {
        std::vector<VkVertexInputBindingDescription> bindings;
        std::vector<VkVertexInputAttributeDescription> attributes;
    };

    struct InputAssemblyState
    {
        VkPrimitiveTopology topology{VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
        VkBool32 primitive_restart_enable{false};
    };

    struct RasterizationState
    {
        VkBool32 depth_clamp_enable{VK_FALSE};
        VkBool32 rasterizer_discard_enable{VK_FALSE};
        VkPolygonMode polygon_mode{VK_POLYGON_MODE_FILL};
        VkCullModeFlags cull_mode{VK_CULL_MODE_BACK_BIT};
        VkFrontFace front_face{VK_FRONT_FACE_CLOCKWISE};
        VkBool32 depth_bias_enable{VK_FALSE};
    };

    class PipelineState
    {
    public:
        PipelineState();

        ~PipelineState() = default;
    
    private:

        bool dirty_{false};        
    };
}