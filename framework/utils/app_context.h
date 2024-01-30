#pragma once

#include <vector>
#include <memory>
#include <Eigen/Dense>
#include <framework/vk/syncs.h>
#include <framework/vk/buffer.h>
#include <framework/vk/vk_constants.h>
#include <framework/scene/component/light.h>

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


    struct GlobalUb{
        // camera
        float ev100; // camera exposure setting value in 100 ISO
        alignas(16) Eigen::Matrix4f view; // 16 + 64
        Eigen::Matrix4f proj; // 16 + 128

        // lights
        Light lights[MAX_LIGHTS_COUNT]; // 16 + 128 + 64 * MAX_LIGHTS_COUNT
        int lights_count;  // 16 + 128 + 64 * MAX_LIGHTS_COUNT + 16
        float reserve[3];  // reserve
    };

    constexpr uint32_t GLOBAL_UBO_CAMERA_SIZE = sizeof(float) * (32+4);
    constexpr uint32_t GLOBAL_UBO_SIZE = GLOBAL_UBO_CAMERA_SIZE + MAX_LIGHTS_COUNT * 64 + 16;    
    class GlobalParamSet final
    {
    public:
        GlobalParamSet();
        ~GlobalParamSet() = default;

        void setCameraParam(const float ev100, const Eigen::Matrix4f &view, const Eigen::Matrix4f &proj);

        void setLights(const Lights &lights);

        void update();

        std::shared_ptr<DescriptorSet> getDescSet() const { return desc_set_; }
    private:
        GlobalUb ub_data_;
        std::unique_ptr<Buffer> ubo_;        
        std::shared_ptr<DescriptorSet> desc_set_;
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
        std::vector<RenderOutputSync> render_output_syncs;

        void destroy() {
            resource_cache.reset();
            stage_pool.reset();
            gpu_asset_manager.reset();
            global_param_set.reset();
            descriptor_pool.reset();            
            frames_data.clear();
            render_output_syncs.clear();
        }
    };

    void updateRtsInContext(const std::vector<std::shared_ptr<RenderTarget>> &rts);
    
    bool initAppContext(const std::shared_ptr<VkDriver> &driver, const std::vector<std::shared_ptr<RenderTarget>> &rts);
    
    const AppContext &getDefaultAppContext();
    
    void destroyDefaultAppContext();
}