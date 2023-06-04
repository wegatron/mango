#pragma once

#include <memory>
#include <map>
#include <framework/shader_module.h>
#include <framework/vk_driver.h>
#include <framework/shader_module.h>


namespace vk_engine
{
    class PipelineLayout final
    {
    public:
        PipelineLayout(const std::shared_ptr<VkDriver> &driver,
            std::vector<ShaderSource *> shader_modules);
        
        ~PipelineLayout();

        PipelineLayout(const PipelineLayout &) = delete;
        PipelineLayout(PipelineLayout &&) = delete;

	    PipelineLayout &operator=(const PipelineLayout &) = delete;
	    PipelineLayout &operator=(PipelineLayout &&) = delete;        

    private:
        VkPipelineLayout handle_{VK_NULL_HANDLE};
	    
        // The shader resources that this pipeline layout uses, indexed by their name
    	std::map<std::string, ShaderResource> shader_resources;
    };
}

