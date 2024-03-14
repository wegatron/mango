file(GLOB_RECURSE headers CONFIGURE_DEPENDS *.h *.hpp)
file(GLOB_RECURSE srcs CONFIGURE_DEPENDS *.cpp)

set(IMGUI_HEADERS
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imconfig.h
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui.h
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_internal.h
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imstb_rectpack.h
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imstb_textedit.h
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imstb_truetype.h
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/backends/imgui_impl_glfw.h
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/backends/imgui_impl_vulkan.h
)
set(IMGUI_SRC
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/backends/imgui_impl_vulkan.cpp
)
add_library(vk_engine STATIC ${headers} ${srcs} ${IMGUI_HEADERS} ${IMGUI_SRC})
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