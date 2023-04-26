cmake_minimum_required(VERSION 3.10)
project(vk_engine)

set(CMAKE_CXX_STANDARD 17)

# 设置编译输出的路径
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${CMAKE_CURRENT_SOURCE_DIR}/install/lib/cmake/glfw3")
find_package(glfw3 REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/install/include
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glm)

add_definitions(-DVK_NO_PROTOTYPES)
if (WIN32)
   set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
elseif()
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_XLIB_KHR)
endif()
add_subdirectory(thirdparty/volk)

file(GLOB src *.cpp)
file(GLOB headers *.h)
add_executable(vk_triangle
    ${src}
    ${headers})

target_link_libraries(vk_triangle PRIVATE volk glfw)