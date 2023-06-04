@REM mkdir thirdparty
@REM git clone https://github.com/g-truc/glm.git thirdparty/glm
@REM git clone https://github.com/glfw/glfw.git thirdparty/glfw
@REM git clone https://github.com/zeux/volk.git thirdparty/volk
@REM git clone https://github.com/KhronosGroup/Vulkan-Headers thirdparty/Vulkan-Headers
@REM git clone https://github.com/gabime/spdlog.git thirdparty/spdlog
@REM git clone https://gitlab.com/libeigen/eigen.git -b 3.4 thirdparty/eigen3.4

@REM cmake thirdparty/glfw -DCMAKE_INSTALL_PREFIX="./install" -B thirdparty/glfw/build
@REM cmake --build thirdparty/glfw/build
@REM cmake --install thirdparty/glfw/build --config Debug

@REM cmake thirdparty/Vulkan-Headers -DCMAKE_INSTALL_PREFIX="./install" -B thirdparty/Vulkan-Headers/build
@REM cmake --build thirdparty/Vulkan-Headers/build
@REM cmake --install thirdparty/Vulkan-Headers/build --config Debug

@REM cmake thirdparty/spdlog -DCMAKE_INSTALL_PREFIX="./install" -B thirdparty/spdlog/build -DCMAKE_BUILD_TYPE=Debug
@REM cmake --build thirdparty/spdlog/build
@REM cmake --install thirdparty/spdlog/build --config Debug

@REM git clone https://github.com/KhronosGroup/glslang thirdparty/glslang
@REM cmake thirdparty/glslang -DCMAKE_INSTALL_PREFIX="./install" -B thirdparty/glslang/build -DCMAKE_BUILD_TYPE=Debug
@REM cmake --build thirdparty/glslang/build
@REM cmake --install thirdparty/glslang/build --config Debug

@REM git clone https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git thirdparty/vma
cmake thirdparty/vma -DCMAKE_INSTALL_PREFIX="./install" -B thirdparty/vma/build -DCMAKE_BUILD_TYPE=Debug -DVMA_STATIC_VULKAN_FUNCTIONS=OFF
cmake --build thirdparty/vma/build
cmake --install thirdparty/vma/build --config Debug

@REM git clone https://github.com/KhronosGroup/SPIRV-Cross.git thirdparty/spirv-cross
@REM cmake thirdparty/spirv-cross -DCMAKE_INSTALL_PREFIX="./install" -B thirdparty/spirv-cross/build -DCMAKE_BUILD_TYPE=Debug
@REM cmake --build thirdparty/spirv-cross/build
@REM cmake --install thirdparty/spirv-cross/build --config Debug