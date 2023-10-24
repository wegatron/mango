#pragma once

#include <vector>
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
    class DescriptorSet;
    class Buffer;
    
    struct FrameData
    {
        std::shared_ptr<CommandPool> command_pool;
        std::shared_ptr<RenderTarget> render_tgt;
    };

    constexpr uint32_t GLOBAL_PARAM_UBO_SIZE = sizeof(float) * 16;

    class GlobalParamSet final
    {
    public:
        GlobalParamSet();
        ~GlobalParamSet();
        std::shared_ptr<DescriptorSet> getDescSet() const { return desc_set; }
        void updateParam(const Eigen::Matrix4f &proj);
    private:
        std::unique_ptr<Buffer> ubo;
        std::shared_ptr<DescriptorSet> desc_set;    
    };    

    struct AppContext
    {
        std::shared_ptr<VkDriver> driver;
        std::shared_ptr<DescriptorPool> descriptor_pool;
        std::shared_ptr<StagePool> stage_pool;
        std::shared_ptr<GPUAssetManager> gpu_asset_manager;
        std::shared_ptr<ResourceCache> resource_cache;
        std::vector<FrameData> frames_data;
        std::unique_ptr<GlobalParamSet> global_param_set;
    };

    bool initAppContext(const std::shared_ptr<VkDriver> &driver, const std::vector<std::shared_ptr<RenderTarget>> &rts);
    
    const AppContext &getDefaultAppContext();    
}