file(GLOB headers scene/*.h utils/*.h vk/*.h)
file(GLOB srcs scene/*.cpp utils/*.cpp vk/*.cpp)

add_library(vk_engine ${headers} ${srcs})
target_link_libraries(vk_engine PRIVATE 
volk
${GLFW_LIBRARY}
${THREAD} 
glslang::glslang
glslang::glslang-default-resource-limits
glslang::SPIRV
VulkanMemoryAllocator::VulkanMemoryAllocator
spirv-cross-reflect
spirv-cross-core
spirv-cross-glsl)