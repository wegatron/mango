#include <framework/scene/rpass.h>
#include <framework/vk/buffer.h>
#include <framework/vk/commands.h>
#include <framework/scene/component/material.h>
#include <framework/vk/pipeline.h>
#include <framework/scene/component/mesh.h>

namespace vk_engine
{
    MeshParamsSet * MeshParamsPool::requestMeshParamsSet()
    {
        if(!free_mesh_params_set_.empty())
        {
            auto ret = free_mesh_params_set_.front();
            free_mesh_params_set_.pop_front();
            used_mesh_params_set_.emplace_back(ret);
            return ret;
        }
        // create new one
        auto ret = new MeshParamsSet;
        ret->ubo = std::make_unique<Buffer>(
        driver_, 0, MESH_UBO_SIZE, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_ALLOCATION_CREATE_MAPPED_BIT |
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_HOST);
        // do we need to recreate every frame?
        ret->desc_set = getDefaultAppContext().descriptor_pool->requestDescriptorSet();
    }

    MeshParamsPool::~MeshParamsPool()
    {

    }

    void MeshParamsPool::gc()
    {

    }

    void RPass::draw(const std::shared_ptr<Material> &mat, const Eigen::Matrix4f &rt,
        const std::shared_ptr<StaticMesh> &mesh, const std::shared_ptr<CommandBuffer> &cmd_buf)
    {
        // bind ubo set3
        auto mesh_params_set = mesh_params_pool_.requestMeshParamsSet();
        mesh_params_set->ubo->update(rt.data(), sizeof(rt));

        // todo: fixme pipeline's vertex input state
        // using VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE
        auto gp = mat_pipeline_pool_.requestGraphicsPipeline(mat);
        auto mat_desc_set = mat->getDescriptorSet();
        cmd_buf->bindPipelineWithDescriptorSets(gp, {mat_desc_set, mesh_params_set->desc_set}, {}, 0);
        
        // bind mesh vertices
        cmd_buf->bindVertexBuffer({mesh->vertices.buffer, mesh->normals.buffer},
            {mesh->vertices.offset, mesh->normals.offset}, 0);
        cmd_buf->bindIndexBuffer(mesh->faces.buffer, mesh->faces.offset, mesh->faces.data_type);
        cmd_buf->drawIndexed(mesh->faces.index_count, 1, 0, 0, 0);
    }
}