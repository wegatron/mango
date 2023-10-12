#pragma once

#include <memory>

namespace vk_engine
{
    class VkDriver;
    class StagePool;
    class ResourceCache;
    class GPUAssetManager;
    class CommandPool;
    class DescriptorPool;
    class RenderTarget;
    
    struct FrameData
    {
        std::shared_ptr<CommandPool> command_pool;
        std::shared_ptr<RenderTarget> render_tgt;
    };

    struct AppContext
    {
        std::shared_ptr<VkDriver> driver;
        std::shared_ptr<DescriptorPool> descriptor_pool;
        // std::shared_ptr<CommandPool> command_pool; frame data
        std::shared_ptr<StagePool> stage_pool;
        std::shared_ptr<GPUAssetManager> gpu_asset_manager;
        std::shared_ptr<ResourceCache> resource_cache;
        std::vector<FrameData> frames_data;
    };

    const AppContext &getDefaultAppContext();
}