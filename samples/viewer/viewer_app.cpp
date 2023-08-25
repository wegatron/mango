#include "viewer_app.h"

namespace vk_engine {
void ViewerApp::init(const std::shared_ptr<VkDriver> &driver,
                     const std::vector<std::shared_ptr<RenderTarget>> &rts) {
  driver_ = driver;
  rts_ = rts;
  if (resource_cache_->getPipelineCache() == nullptr) {
    auto pcw = std::make_unique<VkPipelineCacheWraper>(driver_->getDevice());
    resource_cache_->setPipelineCache(std::move(pcw));
  }
}

void ViewerApp::setScene(const std::string &path)
{
  loadScene(path, *scene_);
}

void ViewerApp::tick(const float seconds, const uint32_t rt_index, const uint32_t frame_index)
{
    scene_->prepare();

    // todo
}
} // namespace vk_engine