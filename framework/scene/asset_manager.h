#pragma once

#include <framework/vk/vk_driver.h>

namespace vk_engine
{
    class Image;
    class ImageView;
    class StagePool;

    /**
     * \brief GPUAssertManager is used to manage the GPU assert.
     * The assert is load from file, and will not change.
    */
    class GPUAssetManager final
    {
    public:
        GPUAssetManager(std::shared_ptr<VkDriver> driver, const std::shared_ptr<StagePool> &stage_pool) : driver_(driver), stage_pool_(stage_pool) {}
        
        ~GPUAssetManager() = default;

        template<typename T>
        std::shared_ptr<T> requestImage(const std::string &path);

        void gc();

        void reset();

    private:
        std::shared_ptr<VkDriver> driver_;
        std::shared_ptr<StagePool> stage_pool_;
        std::map<std::string, std::shared_ptr<void>> images_;
    };
}