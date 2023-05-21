mkdir thirdparty
git clone https://github.com/g-truc/glm.git thirdparty/glm
git clone https://github.com/glfw/glfw.git thirdparty/glfw
git clone https://github.com/zeux/volk.git thirdparty/volk
git clone https://github.com/KhronosGroup/Vulkan-Headers thirdparty/Vulkan-Headers
git clone https://github.com/gabime/spdlog.git thirdparty/spdlog

cmake thirdparty/glfw -DCMAKE_INSTALL_PREFIX="./install" -B thirdparty/glfw/build
cmake --build thirdparty/glfw/build
cmake --install thirdparty/glfw/build --config Debug

cmake thirdparty/Vulkan-Headers -DCMAKE_INSTALL_PREFIX="./install" -B thirdparty/Vulkan-Headers/build
cmake --build thirdparty/Vulkan-Headers/build
cmake --install thirdparty/Vulkan-Headers/build --config Debug

cmake thirdparty/spdlog -DCMAKE_INSTALL_PREFIX="./install" -B thirdparty/spdlog/build -DCMAKE_BUILD_TYPE=Debug
cmake --build thirdparty/spdlog/build
cmake --install thirdparty/spdlog/build --config Debug

git clone https://github.com/KhronosGroup/glslang thirdparty/glslang
cmake thirdparty/glslang -DCMAKE_INSTALL_PREFIX="./install" -B thirdparty/glslang/build -DCMAKE_BUILD_TYPE=Debug
cmake --build thirdparty/glslang/build
cmake --install thirdparty/glslang/build --config Debug
