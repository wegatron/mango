#pragma once

#include <cassert>
#include <framework/vk/pipeline_state.h>

namespace vk_engine
{
    class ResourceCache;
    class RenderPass;
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
            const std::shared_ptr<RenderPass> &render_pass,
            std::unique_ptr<PipelineState> &&pipeline_state);

        PipelineState & getPipelineState() const {
            assert(pipeline_state_ != nullptr);
            return *pipeline_state_;
        }
        
        ~GraphicsPipeline() override = default;

        std::shared_ptr<PipelineLayout> getPipelineLayout() const { return pipeline_layout_; }

    private:        
        std::shared_ptr<VkDriver> driver_;
        std::shared_ptr<PipelineLayout> pipeline_layout_;
        std::unique_ptr<PipelineState> pipeline_state_;        
    };
}