#include <framework/scene/rpass.h>
#include <framework/vk/buffer.h>
#include <framework/vk/commands.h>
#include <framework/scene/component/material.h>
#include <framework/vk/pipeline.h>
#include <framework/scene/component/mesh.h>
#include <framework/vk/vk_constants.h>
#include <framework/utils/app_context.h>

namespace vk_engine
{
    MeshParamsPool::MeshParamsPool()
    {
        auto &driver = getDefaultAppContext().driver;
        ShaderResource sr = {
            .stages = VK_SHADER_STAGE_VERTEX_BIT,
            .type = ShaderResourceType::BufferUniform,
            .mode = ShaderResourceMode::Static,
            .set = PER_OBJECT_SET_INDEX,
            .binding = 0,
            .array_size = 1,
            .size = MESH_UBO_SIZE,
        };
        desc_layout_ = std::make_unique<DescriptorSetLayout>(driver, PER_OBJECT_SET_INDEX, &sr, 1);
        VkDescriptorPoolSize pool_size = {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = MAX_MESH_DESC_SET * CONFIG_UNIFORM_BINDING_COUNT
        };        

        desc_pool_ = std::make_unique<DescriptorPool>(driver, 0, &pool_size, 1, MAX_MESH_DESC_SET);
    }

    MeshParamsSet * MeshParamsPool::requestMeshParamsSet()
    {
        if(!free_mesh_params_set_.empty())
        {
            auto ret = free_mesh_params_set_.front();
            free_mesh_params_set_.pop_front();
            used_mesh_params_set_.emplace_back(ret);
            ret->last_access = cur_frame_;
            return ret;
        }
        // create new one
        auto ret = new MeshParamsSet;
        auto driver = getDefaultAppContext().driver;
        ret->ubo = std::make_unique<Buffer>(
        driver, 0, MESH_UBO_SIZE, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_ALLOCATION_CREATE_MAPPED_BIT |
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_HOST);
        
        ret->desc_set = desc_pool_->requestDescriptorSet(*desc_layout_);
        // update descriptor set
        VkDescriptorBufferInfo desc_buffer_info{
            .buffer = ret->ubo->getHandle(),
            .offset = 0,
            .range = MESH_UBO_SIZE,
        };
        driver->update({VkWriteDescriptorSet{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .dstSet = ret->desc_set->getHandle(),
          .dstBinding = 0,
          .descriptorCount = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .pBufferInfo = &desc_buffer_info}});
        used_mesh_params_set_.emplace_front(ret);
        ret->last_access = cur_frame_;
        return ret;
    }

    void MeshParamsPool::gc()
    {
        // If this is one of the first few frames, return early to avoid wrapping unsigned integers.
        if (++cur_frame_ <= TIME_BEFORE_EVICTION) {
            return;
        }
        const uint32_t eviction_time = cur_frame_ - TIME_BEFORE_EVICTION;
        for(auto itr=used_mesh_params_set_.begin(); itr!=used_mesh_params_set_.end();)
        {
            if((*itr)->last_access < eviction_time)
            {
                itr = used_mesh_params_set_.erase(itr);
                free_mesh_params_set_.emplace_front(*itr);
            } else ++itr;
        }
    }

    void MeshParamsPool::reset()
    {
        for(auto param_set : free_mesh_params_set_)
        {
            delete param_set;
        }
        free_mesh_params_set_.clear();

        for(auto param_set : used_mesh_params_set_)
        {
            delete param_set;
        }
        used_mesh_params_set_.clear();
    }

    void RPass::draw(const std::shared_ptr<Material> &mat, const Eigen::Matrix4f &rt,
        const std::shared_ptr<StaticMesh> &mesh, const std::shared_ptr<CommandBuffer> &cmd_buf)
    {
        // bind ubo set3
        auto mesh_params_set = mesh_params_pool_.requestMeshParamsSet();
        mesh_params_set->ubo->update(rt.data(), sizeof(rt));

        // todo: fixme pipeline's vertex input state
        // using VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE
        auto gp = mat_gpu_res_pool_.requestGraphicsPipeline(mat);
        auto mat_desc_set = mat_gpu_res_pool_.requestMatDescriptorSet(mat);
        cmd_buf->bindPipelineWithDescriptorSets(gp, {mat_desc_set, mesh_params_set->desc_set}, {}, 0);
        
        // bind mesh vertices
        cmd_buf->bindVertexBuffer({mesh->vertices.buffer, mesh->normals.buffer},
            {mesh->vertices.offset, mesh->normals.offset}, 0);
        cmd_buf->bindIndexBuffer(mesh->faces.buffer, mesh->faces.offset, mesh->faces.data_type);
        cmd_buf->drawIndexed(mesh->faces.index_count, 1, 0, 0, 0);
    }
}