#include <framework/utils/app_context.h>

namespace vk_engine
{
    static AppContext g_app_context;

    const AppContext &getDefaultAppContext() { return g_app_context; }

    
    bool initAppContext(const std::shared_ptr<VkDriver> &driver, const std::vector<std::shared_ptr<RenderTarget>> &rts)
    {
        g_app_context.resource_cache = std::make_shared<ResourceCache>();
        g_app_context.driver = driver;
        if (g_app_context.resource_cache->getPipelineCache() == nullptr) {
            auto pcw = std::make_unique<VkPipelineCacheWraper>(driver->getDevice());
            g_app_context.resource_cache->setPipelineCache(std::move(pcw));
        }
        g_app_context.stage_pool = std::make_shared<StagePool>(driver);

        // gpu asset manager
        g_app_context.gpu_asset_manager = std::make_shared<GPUAssetManager>();

        // descriptor pool

        // global param set
        // frames data
        auto cmd_queue = driver->getGraphicsQueue();  
        auto &frames_data = g_app_context.frames_data;
        frames_data.resize(rts.size());  
        render_output_syncs_.resize(rts.size());
        for(auto i=0; i<rts.size(); ++i)
        {
            frames_data[i].command_pool = std::make_unique<CommandPool>(
            driver, cmd_queue->getFamilyIndex(),
            CommandPool::CmbResetMode::ResetPool);
            frames_data[i].render_tgt = rts[i];
            auto &sync = render_output_syncs_[i];
            sync.render_fence = std::make_shared<Fence>(driver, true);
            sync.render_semaphore = std::make_shared<Semaphore>(driver);
            sync.present_semaphore = std::make_shared<Semaphore>(driver);    
        }    
        return true;
    }
}