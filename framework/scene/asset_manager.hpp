#pragma once

#include <framework/utils/app_context.h>
#include <framework/vk/image.h>
#include <framework/vk/vk_driver.h>
#include <stb_image.h>
#include <stdexcept>

namespace vk_engine {

struct Asset {
  std::shared_ptr<void> data_ptr;
  mutable uint64_t last_accessed;
};

template <typename T> std::shared_ptr<T> load(const std::string &path) {
  throw std::logic_error("load type unsupported!");
}

template <> std::shared_ptr<Image> load(const std::string &path);

/**
 * \brief GPUAssertManager is used to manage the GPU assert.
 * The assert is load from file, and will not change.
 */
class GPUAssetManager final {
public:
  GPUAssetManager() = default;

  ~GPUAssetManager() = default;

  template <typename T> std::shared_ptr<T> request(const std::string &path) {
    auto itr = assets_.find(path);
    if (itr != assets_.end()) {

      return std::static_pointer_cast<T>(itr->second);
    }

    auto ret = load<T>(path);
    assets_.emplace(path, ret);
    return ret;
  }

  void gc();

  void reset();

private:
  std::map<std::string, Asset> assets_;
  uint64_t current_frame_{0};
};
} // namespace vk_engine