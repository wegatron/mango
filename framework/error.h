#pragma once

#include "framework/logging.h"
#include "framework/strings.h"

/// @brief Helper macro to test the result of Vulkan calls which can return an
/// error.
#define VK_CHECK(x)                                                            \
  do {                                                                         \
    VkResult err = x;                                                          \
    if (err) {                                                                 \
      LOGE("Detected Vulkan error: {}", vkb::to_string(err));                  \
      abort();                                                                 \
    }                                                                          \
  } while (0)

#define ASSERT_VK_HANDLE(handle)                                               \
  do {                                                                         \
    if ((handle) == VK_NULL_HANDLE) {                                          \
      LOGE("Handle is NULL");                                                  \
      abort();                                                                 \
    }                                                                          \
  } while (0)
