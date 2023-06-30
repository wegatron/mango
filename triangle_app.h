#pragma once

#include <framework/utils/app_base.h>
#include <framework/vk/buffer.h>
#include <framework/vk/image.h>

namespace vk_engine {
class TriangleApp : public AppBase {
public:
  TriangleApp(const std::string &name, const std::shared_ptr<ResourceCache> &resource_cache) 
    : AppBase(name, resource_cache) {}

  void init(const std::shared_ptr<VkDriver> &driver, VkFormat color_format, VkFormat ds_format) override;

  void tick(const float seconds) override;

private:  

  void setupRenderPipeline();

  void buildCommandBuffers();
  
  std::shared_ptr<Buffer> vertex_buffer_;
  std::shared_ptr<RenderPass> render_pass_;
  std::shared_ptr<Pipeline> pipeline_;
};
} // namespace vk_engine