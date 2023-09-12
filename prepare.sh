!/bin/bash
# apt install vulkan-tools libvulkan-dev vulkan-validationlayers-dev spirv-tools

mkdir thirdparty
git clone https://github.com/g-truc/glm.git thirdparty/glm
git clone https://github.com/zeux/volk.git thirdparty/volk
git clone https://gitlab.com/libeigen/eigen.git -b 3.4.1 thirdparty/eigen

git clone https://github.com/glfw/glfw.git thirdparty/glfw
cmake thirdparty/glfw -DCMAKE_INSTALL_PREFIX="./install" -B thirdparty/glfw/build -DCMAKE_BUILD_TYPE=Debug
cmake --build thirdparty/glfw/build
cmake --install thirdparty/glfw/build --config Debug

git clone https://github.com/KhronosGroup/Vulkan-Headers thirdparty/Vulkan-Headers
cmake thirdparty/Vulkan-Headers -DCMAKE_INSTALL_PREFIX="./install" -B thirdparty/Vulkan-Headers/build -DCMAKE_BUILD_TYPE=Debug
cmake --build thirdparty/Vulkan-Headers/build
cmake --install thirdparty/Vulkan-Headers/build --config Debug

git clone https://github.com/gabime/spdlog.git thirdparty/spdlog
cmake thirdparty/spdlog -DCMAKE_INSTALL_PREFIX="./install" -B thirdparty/spdlog/build -DCMAKE_BUILD_TYPE=Debug
cmake --build thirdparty/spdlog/build
cmake --install thirdparty/spdlog/build --config Debug

git clone https://github.com/KhronosGroup/glslang thirdparty/glslang
cmake thirdparty/glslang -DCMAKE_INSTALL_PREFIX="./install" -B thirdparty/glslang/build -DCMAKE_BUILD_TYPE=Debug
cmake --build thirdparty/glslang/build
cmake --install thirdparty/glslang/build --config Debug

git clone https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git thirdparty/vma
cmake thirdparty/vma -DCMAKE_INSTALL_PREFIX="./install" -B thirdparty/vma/build -DCMAKE_BUILD_TYPE=Debug
cmake --build thirdparty/vma/build
cmake --install thirdparty/vma/build --config Debug

git clone https://github.com/KhronosGroup/SPIRV-Cross.git thirdparty/spirv-cross
cmake thirdparty/spirv-cross -DCMAKE_INSTALL_PREFIX="./install" -B thirdparty/spirv-cross/build -DCMAKE_BUILD_TYPE=Debug
cmake --build thirdparty/spirv-cross/build
cmake --install thirdparty/spirv-cross/build --config Debug

git clone https://github.com/assimp/assimp.git thirdparty/assimp
cd thirdparty/assimp
git apply -p1 < ../../assimp_patch.patch
cd ../../
cmake thirdparty/assimp -DCMAKE_INSTALL_PREFIX="./install" -B thirdparty/assimp/build -DCMAKE_BUILD_TYPE=Debug
cmake --build thirdparty/assimp/build
cmake --install thirdparty/assimp/build --config Debug
mkdir -p install/include/contrib/utf8cpp
cp -r thirdparty/assimp/contrib/utf8cpp install/include/contrib

#git clone https://github.com/vblanco20-1/decs.git thirdparty/decs

git clone https://github.com/skypjack/entt.git thirdparty/entt
cmake thirdparty/entt -DCMAKE_INSTALL_PREFIX="./install" -B thirdparty/entt/build -DCMAKE_BUILD_TYPE=Debug
cmake --build thirdparty/entt/build
cmake --install thirdparty/entt/build --config Debug

#download data from https://sketchfab.com/3d-models/buster-drone-294e79652f494130ad2ab00a13fdbafd#download
