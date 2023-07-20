#pragma once

#include <memory>
#include <framework/vk/vk_driver.h>
#include <framework/vk/descriptor_set_layout.h>

namespace vk_engine
{
    class DescriptorSet;
    class DescriptorPool final
    {
    public:
        DescriptorPool(
            const std::shared_ptr<VkDriver> &driver,
            std::vector<VkDescriptorPoolSize> &pool_sizes,
            uint32_t max_sets);

        DescriptorPool(const DescriptorPool &) = delete;
        DescriptorPool &operator=(const DescriptorPool &) = delete;

        ~DescriptorPool();

        VkDescriptorPool getHandle() const { return descriptor_pool_; }

        void reset();

        std::shared_ptr<DescriptorSet> requestDescriptorSet(const DescriptorSetLayout &layout);

    private:
        std::shared_ptr<VkDriver> driver_;
        VkDescriptorPool descriptor_pool_{VK_NULL_HANDLE};
        std::vector<std::shared_ptr<DescriptorSet>> descriptor_sets_;
    };

    class DescriptorSet
    {
    public:
        DescriptorSet(const DescriptorSet &) = delete;
        DescriptorSet &operator=(const DescriptorSet &) = delete;

        ~DescriptorSet();

        VkDescriptorSet getHandle() const { return descriptor_set_; }

        void update(const std::vector<VkWriteDescriptorSet> &descriptor_writes);
        
    private:
        DescriptorSet(const std::shared_ptr<VkDriver> &driver,
                      DescriptorPool &pool,
                      const DescriptorSetLayout &layout);        
        std::shared_ptr<VkDriver> driver_;
        VkDescriptorPool descriptor_pool_{VK_NULL_HANDLE};
        VkDescriptorSet descriptor_set_{VK_NULL_HANDLE};

        friend class DescriptorPool;
    };
}