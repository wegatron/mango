cmake_minimum_required(VERSION 3.10)
project(vk_engine)

set(CMAKE_CXX_STANDARD 17)

# 设置编译输出的路径
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(WIN32)
    set(GLFW_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/install/lib/glfw3.lib)
else()
    set(GLFW_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/install/lib/libglfw3.a)
endif()

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/install/lib/cmake/glslang)

find_package(glslang REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/install/include
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Vulkan-Hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/eigen3.4
)

if (NOT APPLE AND ${CMAKE_CXX_COMPILER} MATCHES "(C|c?)lang")
    add_link_options(-stdlib=libc++)
    add_compile_options(-stdlib=libc++)
endif()

add_definitions(-DVK_NO_PROTOTYPES)
if (WIN32)
   set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
elseif()
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_XLIB_KHR)
endif()
add_subdirectory(thirdparty/volk)

file(GLOB src *.cpp framework/*.cpp)
file(GLOB headers *.h framework/*.h)
add_executable(vk_triangle
    ${src}
    ${headers})

if (NOT WIN32)
set(THREAD "-lpthread")
endif()

target_link_libraries(vk_triangle 
    PRIVATE 
    volk
    ${GLFW_LIBRARY}
    ${THREAD} 
    glslang::glslang    
    glslang::glslang-default-resource-limits
    glslang::SPIRV
)