#pragma once

#include <cassert>
#include <framework/pipeline_state.h>

namespace vk_engine
{
    class ResourceCache;
    class Pipeline
    {
    public:
        Pipeline(const Pipeline &) = delete;
        Pipeline &operator=(const Pipeline &) = delete;

        Pipeline() = default;
        virtual ~Pipeline() = default;

        const std::shared_ptr<PipelineState> getPipelineState() const { return pipeline_state_; }

        VkPipeline getHandle() const { return pipeline_; }

        void cleanDirtyFlag() { pipeline_state_->dirty_ = false; }

    protected:
        VkPipeline pipeline_{VK_NULL_HANDLE};        
        std::shared_ptr<PipelineState> pipeline_state_;
    };

    class GraphicsPipeline : public Pipeline
    {
    public:
        GraphicsPipeline(
            const std::shared_ptr<VkDriver> &driver,
            const std::shared_ptr<ResourceCache> &cache,
            std::unique_ptr<PipelineState> &&pipeline_state);

        PipelineState & getPipelineState() const {
            assert(pipeline_state_ != nullptr);
            return *pipeline_state_;
        }
        
        ~GraphicsPipeline() override = default;

    private:        
        std::shared_ptr<VkDriver> driver_;
        std::shared_ptr<PipelineLayout> pipeline_layout_;
        std::unique_ptr<PipelineState> pipeline_state_;        
    };
}