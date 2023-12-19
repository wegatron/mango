#include <framework/scene/asset_manager.hpp>

#include <stb_image.h>

#include <framework/utils/app_context.h>
#include <framework/vk/image.h>
#include <framework/vk/vk_driver.h>
#include <cassert>

namespace vk_engine
{
    static constexpr uint32_t ASSET_TIME_BEFORE_EVICTION = 100;   

    template <>
    std::shared_ptr<ImageView> load(const std::string &path, const std::shared_ptr<CommandBuffer> &cmd_buf) {
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
        image->updateByStaging(img_data, getDefaultAppContext().stage_pool, cmd_buf);
        stbi_image_free(img_data);

        // image view
        return std::make_shared<ImageView>(
            image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8_SRGB,
            VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1, 1);
    }

    template <>
    std::shared_ptr<ImageView> load(const uint8_t *data, const size_t size, const std::shared_ptr<CommandBuffer> &cmd_buf)
    {
        int width = 0;
        int height = 0;
        int channel = 0;
        stbi_uc *img_data = stbi_load_from_memory(data, size, &width, &height, &channel, 0);
        VkExtent3D extent{static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
        assert(channel == 4);
        auto driver = getDefaultAppContext().driver;
        auto image = std::make_shared<Image>(
            driver, 0, VK_FORMAT_R8G8B8_SRGB, extent, VK_SAMPLE_COUNT_1_BIT,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
        image->updateByStaging(img_data, getDefaultAppContext().stage_pool, cmd_buf);
        stbi_image_free(img_data);

        // image view
        return std::make_shared<ImageView>(
            image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8_SRGB,
            VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1, 1);             
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