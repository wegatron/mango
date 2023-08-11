#include <framework/utils/logging.h>
#include <iostream>

#include <cassert>
#include <volk.h>
#include <vulkan/vulkan.h>

#include <framework/utils/window_app.h>
#include "triangle_app.h"
#include <memory_resource>

  // class LinearAllocator
  // {
  // public:
  //     template <typename T>
  //     T * allocate(std::size_t n)
  //     {
  //         return static_cast<T *>(resource.allocate(n * sizeof(T), alignof(T)));
  //     }


      
  // private:
  //     std::pmr::monotonic_buffer_resource resource_;
  // };

int main(int argc, char const *argv[]) {
  std::byte stack_buffer[100];
  std::pmr::monotonic_buffer_resource resource_{stack_buffer, sizeof(stack_buffer)};

  int * a = reinterpret_cast<int*>(resource_.allocate(10*sizeof(int), alignof(int)));

  std::cout << "a: " << a << std::endl;
  //resource_.release();

  int * b = reinterpret_cast<int*>(resource_.allocate(10*sizeof(int), alignof(int)));

  std::cout << "b: " << b << std::endl;

  // #if !NDEBUG
  // spdlog::set_level(spdlog::level::debug);
  // #endif
  // auto app = std::make_shared<vk_engine::TriangleApp>("triangle", nullptr);
  // auto window_app =
  //     std::make_shared<vk_engine::WindowApp>("triangle", 800, 600);

  // window_app->setApp(std::move(app));
  // if (!window_app->init()) {
  //   LOGE("Failed to init window app");
  //   return -1;
  // }
  // window_app->run();
  return 0;
}
