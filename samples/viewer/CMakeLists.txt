include_directories(${CMAKE_SOURCE_DIR}/thirdparty/imgui)
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
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/backends/imgui_impl_vulkan.cpp
)

add_executable(viewer main.cpp viewer_app.cpp viewer_app.h ${IMGUI_SRC} ${IMGUI_HEADERS})

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
    assimp::assimp
)