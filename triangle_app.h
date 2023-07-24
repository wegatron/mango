#pragma once

#include <framework/utils/app_base.h>
#include <framework/vk/buffer.h>
#include <framework/vk/image.h>
#include <framework/vk/pipeline.h>
#include <framework/vk/descriptor_set.h>

#include <Eigen/Dense>

namespace vk_engine {
class CommandBuffer;
class TriangleApp : public AppBase {
public:
  TriangleApp(const std::string &name, const std::shared_ptr<ResourceCache> &resource_cache) 
    : AppBase(name, resource_cache) {}

  void init(const std::shared_ptr<VkDriver> &driver, const std::vector<std::shared_ptr<RenderTarget>> &rts) override;

  void tick(const float seconds, const uint32_t render_target_index, const uint32_t frame_index) override;

private:

  struct FrameData
  {
    Eigen::Matrix4f model;
    Eigen::Matrix4f view_proj;
    std::shared_ptr<DescriptorSet> descriptor_set;
    std::unique_ptr<Buffer> uniform_buffer; // dynaic data
    std::shared_ptr<CommandBuffer> cmd_buffer;
    
    FrameData() {
      model.setIdentity();
      view_proj.setIdentity();
    }
  };

  void setupScene();

  void setupRender(VkFormat color_format, VkFormat ds_format);

  void buildCommandBuffers();
  
  uint32_t frames_inflight_{0};
  std::shared_ptr<Buffer> vertex_buffer_; //!< static vertex buffer
  std::shared_ptr<RenderPass> render_pass_;
  std::shared_ptr<GraphicsPipeline> pipeline_;
  std::shared_ptr<DescriptorPool> descriptor_pool_;
  std::vector<FrameData> frame_data_;
};
} // namespace vk_engine