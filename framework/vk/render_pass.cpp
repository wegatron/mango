#include <framework/vk/render_pass.h>

namespace vk_engine {

bool is_depth_only_format(VkFormat format)
{
	return format == VK_FORMAT_D16_UNORM ||
	       format == VK_FORMAT_D32_SFLOAT;
}

bool is_depth_stencil_format(VkFormat format)
{
	return format == VK_FORMAT_D16_UNORM_S8_UINT ||
	       format == VK_FORMAT_D24_UNORM_S8_UINT ||
	       format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
	       is_depth_only_format(format);
}

void fill_input_attachment_refs(
    const std::vector<uint32_t> &attachment_inds,
    const std::vector<VkAttachmentDescription> &attachment_descriptions,
    size_t &ref_ind,
    std::vector<VkAttachmentReference> &attachment_refs)
{
    for (size_t i = 0; i < attachment_inds.size(); ++i) {
        attachment_refs[ref_ind].attachment = attachment_inds[i];
        attachment_refs[ref_ind].layout = is_depth_stencil_format(attachment_descriptions[attachment_inds[i]].format) 
            ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        ++ref_ind;
    }
}

void fill_output_attachment_refs(
    const std::vector<uint32_t> &attachment_inds,
    const std::vector<VkAttachmentDescription> &attachment_descriptions,
    size_t &ref_ind,
    std::vector<VkAttachmentReference> &attachment_refs)
{
    for (size_t i = 0; i < attachment_inds.size(); ++i) {
        attachment_refs[ref_ind].attachment = attachment_inds[i];
        attachment_refs[ref_ind].layout = attachment_descriptions[attachment_inds[i]].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED
            ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : attachment_descriptions[attachment_inds[i]].initialLayout;
        ++ref_ind;
    }
}

RenderPass::RenderPass(const std::shared_ptr<VkDriver> &driver,
           std::vector<Attachment> attachments,
           std::vector<LoadStoreInfo> load_store_infos,
           std::vector<SubpassInfo> subpasses) 
{
    std::vector<VkAttachmentDescription> attachment_descriptions(attachments.size());
    for (size_t i = 0; i < attachments.size(); ++i) {
        attachment_descriptions[i].flags = 0;
        attachment_descriptions[i].format = attachments[i].format;
        attachment_descriptions[i].samples = attachments[i].samples;
        attachment_descriptions[i].loadOp = load_store_infos[i].load_op;
        attachment_descriptions[i].storeOp = load_store_infos[i].store_op;
        attachment_descriptions[i].stencilLoadOp = load_store_infos[i].load_op;
        attachment_descriptions[i].stencilStoreOp = load_store_infos[i].store_op;
        attachment_descriptions[i].initialLayout = attachments[i].initial_layout;
        attachment_descriptions[i].finalLayout = is_depth_stencil_format(attachments[i].format) ?
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;;
    }

    size_t attachment_num = 0;
    for(size_t i=0; i<subpasses.size(); ++i) {
        attachment_num += subpasses[i].input_attachments.size();
        attachment_num += subpasses[i].output_attachments.size();
        attachment_num += subpasses[i].color_resolve_attachments.size();
        if(subpasses[i].depth_stencil_attachment != 0xFFFFFFFF) ++attachment_num;
        if(subpasses[i].depth_stencil_resolve_attachment != 0xFFFFFFFF) ++attachment_num;
    }
    size_t ref_ind = 0;
    std::vector<VkAttachmentReference> attachment_refs(attachment_num);
    std::vector<VkSubpassDescription> subpass_descriptions(subpasses.size());
    for(size_t i=0; i<subpasses.size(); ++i)
    {
        subpass_descriptions[i].flags = 0;
        subpass_descriptions[i].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_descriptions[i].inputAttachmentCount = subpasses[i].input_attachments.size();
        subpass_descriptions[i].pInputAttachments = attachment_refs.data() + ref_ind;
        fill_input_attachment_refs(subpasses[i].input_attachments, attachment_descriptions, ref_ind, attachment_refs);

        subpass_descriptions[i].colorAttachmentCount = subpasses[i].output_attachments.size();        
        subpass_descriptions[i].pColorAttachments = attachment_refs.data() + ref_ind;
        fill_output_attachment_refs(subpasses[i].output_attachments, attachment_descriptions, ref_ind, attachment_refs);

        subpass_descriptions[i].pResolveAttachments = nullptr;
        if(subpasses[i].color_resolve_attachments.size() > 0)
        {
            subpass_descriptions[i].pResolveAttachments = attachment_refs.data() + ref_ind;
            fill_output_attachment_refs(subpasses[i].color_resolve_attachments, attachment_descriptions, ref_ind, attachment_refs);
        }

        subpass_descriptions[i].pDepthStencilAttachment = nullptr;
        if(subpasses[i].depth_stencil_attachment != 0xFFFFFFFF)
        {
            subpass_descriptions[i].pDepthStencilAttachment = attachment_refs.data() + ref_ind;
            attachment_refs[ref_ind].attachment = subpasses[i].depth_stencil_attachment;
            attachment_refs[ref_ind].layout = 
                (attachment_descriptions[subpasses[i].depth_stencil_attachment].initialLayout==VK_IMAGE_LAYOUT_UNDEFINED) 
                ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : attachment_descriptions[subpasses[i].depth_stencil_attachment].initialLayout;
            ++ref_ind;


            // if(subpasses[i].depth_stencil_resolve_attachment != 0xFFFFFFFF)
            // {
            //     subpass_descriptions[i].pDepthStencilResolveAttachment = attachment_refs.data() + ref_ind;
            //     attachment_refs[ref_ind].attachment = subpasses[i].depth_stencil_resolve_attachment;
            //     attachment_refs[ref_ind].layout = 
            //         (attachment_descriptions[subpasses[i].depth_stencil_resolve_attachment].initialLayout==VK_IMAGE_LAYOUT_UNDEFINED) 
            //         ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : attachment_descriptions[subpasses[i].depth_stencil_resolve_attachment].initialLayout;
            // }            
        }
    }
}

}