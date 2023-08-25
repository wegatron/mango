add_executable(viewer main.cpp viewer_app.cpp viewer_app.h)

target_link_libraries(viewer 
    PRIVATE 
    vk_engine
    volk
    ${GLFW_LIBRARY}
    ${THREAD} 
    glslang::glslang    
    glslang::glslang-default-resource-limits
    glslang::SPIRV
    VulkanMemoryAllocator::VulkanMemoryAllocator
    spirv-cross-reflect
    spirv-cross-core
    spirv-cross-glsl
    assimp
)