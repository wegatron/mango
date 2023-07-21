
#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <volk.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <framework/utils/error.h>
#include <framework/vk/physical_device.h>
#include <framework/vk/vk_driver.h>

namespace vk_engine {
constexpr uint32_t vulkan_version = VK_API_VERSION_1_1;

void VkDriver::initInstance() {
  if (VK_SUCCESS != volkInitialize()) {
    throw std::runtime_error("failed to initialize volk!");
  }

  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "vk_engine";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "No Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0); // 以上这些意义不大
  app_info.apiVersion = vulkan_version;              // vulkan api version

  VkInstanceCreateInfo instance_info{};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;
  config_->checkAndUpdate(instance_info);

  if (VK_SUCCESS != vkCreateInstance(&instance_info, nullptr, &instance_))
    throw std::runtime_error("failed to create instance");

  volkLoadInstanceOnly(instance_);
}

void VkDriver::initDevice() {
  auto physical_devices = PhysicalDevice::getPhysicalDevices(instance_);
  VkDeviceCreateInfo device_info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
  const uint32_t physical_device_index =
      config_->checkSelectAndUpdate(physical_devices, device_info, surface_);

  if (physical_device_index == -1) {
    throw std::runtime_error("failed to select physical device!");
  }

  physical_device_ = physical_devices[physical_device_index].getHandle();
  const uint32_t queue_index =
      physical_devices[physical_device_index].getGraphicsQueueFamilyIndex();

// for print out memory infos
#if !defined(NDEBUG)
  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(physical_device_, &memory_properties);
  for (uint32_t i = 0; i < memory_properties.memoryHeapCount; ++i) {
    std::cout << " heap size " << std::dec
              << memory_properties.memoryHeaps[i].size / 1000000 << "MB"
              << " flags " << std::hex << memory_properties.memoryHeaps[i].flags
              << std::endl;
  }
  for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i) {
    std::cout << " heap index " << memory_properties.memoryTypes[i].heapIndex
              << " memory property flags ";
    if (memory_properties.memoryTypes[i].propertyFlags &
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
      std::cout << "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ";
    if (memory_properties.memoryTypes[i].propertyFlags &
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
      std::cout << "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ";
    if (memory_properties.memoryTypes[i].propertyFlags &
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
      std::cout << "VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ";
    if (memory_properties.memoryTypes[i].propertyFlags &
        VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
      std::cout << "VK_MEMORY_PROPERTY_HOST_CACHED_BIT ";
    if (memory_properties.memoryTypes[i].propertyFlags &
        VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
      std::cout << "VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT ";
    std::cout << std::endl;
  }
#endif

  // queue info
  VkDeviceQueueCreateInfo queue_info{};
  float queue_priority = 1.0f;
  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.queueFamilyIndex = queue_index;
  queue_info.queueCount = 1;
  queue_info.pQueuePriorities = &queue_priority;

  // logical device
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.pQueueCreateInfos = &queue_info;
  device_info.queueCreateInfoCount = 1;

  VK_THROW_IF_ERROR(
      vkCreateDevice(physical_device_, &device_info, nullptr, &device_),
      "failed to create vulkan device!");

  volkLoadDevice(device_);

  vkGetDeviceQueue(device_, queue_index, 0, &graphics_queue_);
}

void VkDriver::init(const std::string &app_name,
                    const std::shared_ptr<VkConfig> &config,
                    GLFWwindow *window) {
  config_ = config;
  initInstance();

  assert(window != nullptr);
  VK_THROW_IF_ERROR(glfwCreateWindowSurface(instance_, window, nullptr,
                                            &surface_),
                    "failed to create window surface!");

  initDevice();

  initAllocator();
}

bool VkDriver::isDeviceExtensionEnabled(const char *extension_name) {
  for (const auto &ext : enabled_device_extensions_) {
    if (strcmp(ext, extension_name) == 0)
      return true;
  }
  return false;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
              void *pUserData) {
  std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

  return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void VkDriver::setupDebugMessenger() {

  VkDebugUtilsMessengerCreateInfoEXT createInfo;
  createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = debugCallback;

  if (CreateDebugUtilsMessengerEXT(instance_, &createInfo, nullptr,
                                   &debug_messenger_) != VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug messenger!");
  }
}

void VkDriver::checkSwapchainAbility() {
  VkSurfaceFormatKHR surface_format{VK_FORMAT_B8G8R8A8_SRGB,
                                    VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
  uint32_t format_count = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_,
                                       &format_count, nullptr);
  std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_,
                                       &format_count, surface_formats.data());
  auto format_itr =
      std::find_if(surface_formats.begin(), surface_formats.end(),
                   [&surface_format](const VkSurfaceFormatKHR sf) {
                     return sf.format == surface_format.format &&
                            sf.colorSpace == surface_format.colorSpace;
                   });
  if (format_itr == surface_formats.end()) {
    throw std::runtime_error(
        "B8G8R8_SNORM format, COLOR_SPACE_SRGB_NONLINEAR_KHR is not "
        "supported!");
  }

  uint32_t present_mode_count = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_, surface_,
                                            &present_mode_count, nullptr);
  std::vector<VkPresentModeKHR> present_modes(present_mode_count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      physical_device_, surface_, &present_mode_count, present_modes.data());
  auto present_itr =
      std::find_if(present_modes.begin(), present_modes.end(),
                   [](const VkPresentModeKHR present_mode) {
                     return present_mode == VK_PRESENT_MODE_FIFO_KHR;
                   });
  if (present_itr == present_modes.end()) {
    throw std::runtime_error("FIFO present mode is not supported!");
  }
}

void VkDriver::initAllocator() {
  VmaVulkanFunctions vma_vulkan_func{};
  vma_vulkan_func.vkAllocateMemory = vkAllocateMemory;
  vma_vulkan_func.vkBindBufferMemory = vkBindBufferMemory;
  vma_vulkan_func.vkBindImageMemory = vkBindImageMemory;
  vma_vulkan_func.vkCreateBuffer = vkCreateBuffer;
  vma_vulkan_func.vkCreateImage = vkCreateImage;
  vma_vulkan_func.vkDestroyBuffer = vkDestroyBuffer;
  vma_vulkan_func.vkDestroyImage = vkDestroyImage;
  vma_vulkan_func.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
  vma_vulkan_func.vkFreeMemory = vkFreeMemory;
  vma_vulkan_func.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
  vma_vulkan_func.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
  vma_vulkan_func.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;
  vma_vulkan_func.vkGetPhysicalDeviceMemoryProperties =
      vkGetPhysicalDeviceMemoryProperties;
  vma_vulkan_func.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
  vma_vulkan_func.vkInvalidateMappedMemoryRanges =
      vkInvalidateMappedMemoryRanges;
  vma_vulkan_func.vkMapMemory = vkMapMemory;
  vma_vulkan_func.vkUnmapMemory = vkUnmapMemory;
  vma_vulkan_func.vkCmdCopyBuffer = vkCmdCopyBuffer;

  VmaAllocatorCreateInfo allocator_info{};
  allocator_info.physicalDevice = physical_device_;
  allocator_info.device = device_;
  allocator_info.instance = instance_;
  allocator_info.vulkanApiVersion = vulkan_version;
  allocator_info.pVulkanFunctions = &vma_vulkan_func;

  bool can_get_memory_requirements =
      isDeviceExtensionEnabled(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
  bool has_dedicated_allocation =
      isDeviceExtensionEnabled(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
  bool can_get_buffer_device_address =
      isDeviceExtensionEnabled(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);

  if (can_get_memory_requirements && has_dedicated_allocation) {
    allocator_info.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
    vma_vulkan_func.vkGetBufferMemoryRequirements2KHR =
        vkGetBufferMemoryRequirements2KHR;
    vma_vulkan_func.vkGetImageMemoryRequirements2KHR =
        vkGetImageMemoryRequirements2KHR;
  }

  if (can_get_buffer_device_address) {
    allocator_info.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
  }

  auto result = vmaCreateAllocator(&allocator_info, &allocator_);
  if (result != VK_SUCCESS) {
    throw VulkanException(result, "failed to create vma allocator");
  }
}

VkDriver::~VkDriver() {
  vkDestroyDevice(device_, nullptr);
  vkDestroySurfaceKHR(instance_, surface_, nullptr);
  vkDestroyInstance(instance_, nullptr);
}
} // namespace vk_engine
