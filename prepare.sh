#!/bin/bash
#apt install vulkan-tools libvulkan-dev vulkan-validationlayers-dev spirv-tools

mkdir thirdparty
git clone https://github.com/g-truc/glm.git thirdparty/glm
git clone https://github.com/glfw/glfw.git thirdparty/glfw
git clone https://github.com/zeux/volk.git thirdparty/volk
git clone https://github.com/KhronosGroup/Vulkan-Headers thirdparty/Vulkan-Headers
git clone https://github.com/KhronosGroup/Vulkan-Hpp.git thirdparty/Vulkan-Hpp

cmake thirdparty/glfw -DCMAKE_INSTALL_PREFIX="./install" -B thirdparty/glfw/build
cmake --build thirdparty/glfw/build
cmake --install thirdparty/glfw/build --config Debug

cmake thirdparty/Vulkan-Headers -DCMAKE_INSTALL_PREFIX="./install" -B thirdparty/Vulkan-Headers/build
cmake --build thirdparty/Vulkan-Headers/build
cmake --install thirdparty/Vulkan-Headers/build --config Debug
