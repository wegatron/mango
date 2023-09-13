#include <framework/scene/asset_manager.hpp>

namespace vk_engine
{
    static constexpr uint32_t ASSET_TIME_BEFORE_EVICTION = 100;
    
    template <>
    std::shared_ptr<Image> load(const std::string &path) {
        int width = 0;
        int height = 0;
        int channel = 0;
        stbi_uc *img_data = stbi_load(path.c_str(), &width, &height, &channel, 0);
        VkExtent3D extent{static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
        assert(channel == 4);
        auto driver = getDefaultAppContext().driver;
        auto image = std::make_shared<Image>(
            driver, 0, VK_FORMAT_R8G8B8_SRGB, extent, VK_SAMPLE_COUNT_1_BIT,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        image->updateByStaging(img_data, width * channel);
        stbi_image_free(img_data);
        return image;
    }    

    void GPUAssetManager::gc()
    {
        if(++current_frame_ < ASSET_TIME_BEFORE_EVICTION) return;
        for(auto itr = assets_.begin(); itr != assets_.end();)
        {
            auto & a = itr->second;
            if(a.data_ptr.use_count() == 1 && a.last_accessed + ASSET_TIME_BEFORE_EVICTION <= current_frame_)
            {
                itr = assets_.erase(itr);
            } else ++itr;
        }
    }

    void GPUAssetManager::reset()
    {        
        assets_.clear();
    }
}