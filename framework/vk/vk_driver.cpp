
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
constexpr uint32_t vulkan_version = VK_API_VERSION_1_0;

const std::vector<const char *> request_validation_layers = {
    "VK_LAYER_KHRONOS_validation",
};

const std::vector<RequestedDeviceExtension> request_device_extensions = {
    {VK_KHR_SWAPCHAIN_EXTENSION_NAME, true},
    {VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME, false},
    {VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME, false},
    {VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, false},
    {VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, false},
    {VK_KHR_DEVICE_GROUP_EXTENSION_NAME, false},
    {VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME, false},
};


bool checkValidationLayerSupport() {
  uint32_t layer_count = 0;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

  for (const auto &name : request_validation_layers) {
    bool is_find = false;
    for (const auto &l : available_layers) {
      if (strcmp(l.layerName, name) == 0) {
        is_find = true;
        break;
      }
    }

    if (!is_find) {
      std::string error_msg = std::string("validation layer ") +
                              std::string(name) +
                              " requested, but not available!";
      throw std::runtime_error(error_msg);
      return false;
    }
  }
  return true;
}

std::pair<bool, uint32_t>
VkDriver::selectPhysicalDevice(const std::vector<RequestedDeviceExtension> &request_extensions) {
  auto physical_devices = PhysicalDevice::getPhysicalDevices(instance_);
  for (const auto &pd : physical_devices) {
    physical_device_ = pd.getHandle();
    uint32_t graphics_queue_family_index = pd.getGraphicsQueueFamilyIndex();
    if (graphics_queue_family_index == 0XFFFFFFFF)
      continue;
    VkBool32 surface_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device_,
                                         graphics_queue_family_index, surface_,
                                         &surface_support);
    if (!surface_support ||
        (pd.getProperties().deviceType !=
         VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) ||
        !(pd.getFeatures().geometryShader))
      continue;

    const auto &device_extensions = pd.getExtensionProperties();
    bool extension_support = true;
    for (const auto &req_ext : request_extensions) {
      bool is_find = false;
      for (const auto &ext : device_extensions) {
        if (strcmp(ext.extensionName, req_ext.name) == 0) {
          is_find = true;
          break;
        }
      }
      if(is_find) {
        enabled_device_extensions_.push_back(req_ext.name);
        continue;
      }
      if (req_ext.required) {
        extension_support = false;
        break;
      }
    }
    if (!extension_support)
      continue;
    return std::make_pair(true, graphics_queue_family_index);
  }

  return std::make_pair(false, -1);
}

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
  app_info.apiVersion = vulkan_version;          // vulkan api version

  VkInstanceCreateInfo instance_info{};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;

  // glfw extension
  uint32_t glfw_extension_count = 0;
  const char **glfw_extensions =
      glfwGetRequiredInstanceExtensions(&glfw_extension_count);
  instance_info.enabledExtensionCount = glfw_extension_count;
  instance_info.ppEnabledExtensionNames = glfw_extensions;

  // validation layer
  instance_info.enabledLayerCount = 0;

  // install vulkan sdk for validation support
  if (enable_vk_validation_) {
    if (!checkValidationLayerSupport())
      return;
    instance_info.enabledLayerCount =
        static_cast<uint32_t>(request_validation_layers.size());
    instance_info.ppEnabledLayerNames = request_validation_layers.data();
  }

  if (VK_SUCCESS != vkCreateInstance(&instance_info, nullptr, &instance_))
    throw std::runtime_error("failed to create instance");

  volkLoadInstanceOnly(instance_);
}

void VkDriver::initDevice() {
  auto select_ret = selectPhysicalDevice(request_device_extensions);
  if (!select_ret.first) {
    throw std::runtime_error("failed to select suitable physical device!");
  }

  // for print out memory infos
  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(physical_device_, &memory_properties);  
  for(uint32_t i = 0; i < memory_properties.memoryHeapCount; ++i) {
    std::cout << " heap size " << std::dec << memory_properties.memoryHeaps[i].size / 1000000 << "MB"
              << " flags " << std::hex << memory_properties.memoryHeaps[i].flags << std::endl;
  }
  for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i) {
    std::cout << " heap index " << memory_properties.memoryTypes[i].heapIndex 
              << " memory property flags ";
    if(memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
      std::cout << "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ";
    if(memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
      std::cout << "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ";
    if(memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
      std::cout << "VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ";
    if(memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
      std::cout << "VK_MEMORY_PROPERTY_HOST_CACHED_BIT ";
    if(memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
      std::cout << "VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT ";
    std::cout << std::endl;
  }

  // queue info
  VkDeviceQueueCreateInfo queue_info{};
  float queue_priority = 1.0f;
  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.queueFamilyIndex = select_ret.second;
  queue_info.queueCount = 1;
  queue_info.pQueuePriorities = &queue_priority;

  // logical device
  VkDeviceCreateInfo device_info{};
  VkPhysicalDeviceFeatures
      device_features{}; // no need to set, all false, when use features to
                         // enable all supported features may crash
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.pQueueCreateInfos = &queue_info;
  device_info.queueCreateInfoCount = 1;
  device_info.pEnabledFeatures = &device_features;
  device_info.enabledExtensionCount = enabled_device_extensions_.size();
  device_info.ppEnabledExtensionNames = enabled_device_extensions_.data();

  device_features.geometryShader = VK_TRUE;
  if (enable_vk_validation_) {
    device_info.enabledLayerCount = request_validation_layers.size();
    device_info.ppEnabledLayerNames = request_validation_layers.data();
  } else
    device_info.enabledLayerCount = 0;

  if (VK_SUCCESS !=
      vkCreateDevice(physical_device_, &device_info, nullptr, &device_)) {
    throw std::runtime_error("failed to create vulkan device!");
  }

  volkLoadDevice(device_);

  vkGetDeviceQueue(device_, select_ret.second, 0, &graphics_queue_);
}

void VkDriver::init(const std::string &app_name, const bool enable_validation,
                    GLFWwindow *window) {
  enable_vk_validation_ = enable_validation;
  initInstance();

  assert(window != nullptr);
  if (VK_SUCCESS !=
      glfwCreateWindowSurface(instance_, window, nullptr, &surface_)) {
    throw std::runtime_error("failed to create window surface!");
  }

  initDevice();

  checkSwapchainAbility();

  initSwapchain(window);

  initAllocator();
}

bool VkDriver::isDeviceExtensionEnabled(const char *extension_name)
{
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

void VkDriver::initSwapchain(GLFWwindow *window) {
  VkSurfaceCapabilitiesKHR surface_capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_,
                                            &surface_capabilities);
  VkSurfaceFormatKHR surface_format{VK_FORMAT_B8G8R8A8_SRGB,
                                    VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

  // real resolution for highdpi
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  VkExtent2D extent = {static_cast<uint32_t>(width),
                       static_cast<uint32_t>(height)};
  extent.width =
      std::clamp(extent.width, surface_capabilities.minImageExtent.width,
                 surface_capabilities.maxImageExtent.width);
  extent.height =
      std::clamp(extent.height, surface_capabilities.minImageExtent.height,
                 surface_capabilities.maxImageExtent.height);

  uint32_t image_count = surface_capabilities.minImageCount + 1;
  if (surface_capabilities.maxImageCount > 0 &&
      image_count > surface_capabilities.maxImageCount)
    image_count = surface_capabilities.maxImageCount;

  // create swapchain
  VkSwapchainCreateInfoKHR swapchain_info{};
  swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchain_info.surface = surface_;
  swapchain_info.imageExtent = extent;
  swapchain_info.minImageCount = image_count;

  swapchain_info.imageFormat = surface_format.format;
  swapchain_info.imageColorSpace = surface_format.colorSpace;
  swapchain_info.imageArrayLayers = 1;
  swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchain_info.preTransform = surface_capabilities.currentTransform;
  swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchain_info.clipped = VK_TRUE;
  swapchain_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
  swapchain_info.oldSwapchain = swapchain_;

  // vkCreateSwapchainKHR maybe null if not enable VK_KHR_swapchain
  // make sure eanbleExtensionCount is correct when create logical device
  // refer to:
  // https://stackoverflow.com/questions/55131406/why-would-vkcreateswapchainkhr-result-in-an-access-violation-at-0
  VK_CHECK(
      vkCreateSwapchainKHR(device_, &swapchain_info, nullptr, &swapchain_));

  // retrieve swapchain images
  if (swapchain_info.oldSwapchain != VK_NULL_HANDLE) {
    throw std::runtime_error("recreate swapchain is not implemented!");
    for (auto &image_view : swapchain_image_views_) {
      vkDestroyImageView(device_, image_view, nullptr);
    }
    vkDestroySwapchainKHR(device_, swapchain_info.oldSwapchain, nullptr);
  }

  VK_CHECK(vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, nullptr));
  swapchain_images_.resize(image_count);
  vkGetSwapchainImagesKHR(device_, swapchain_, &image_count,
                          swapchain_images_.data());
  swapchain_extent_ = extent;
  swapchain_image_format_ = surface_format.format;

  // image views
  swapchain_image_views_.resize(swapchain_images_.size());
  VkImageViewCreateInfo image_view_info{};
  image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  image_view_info.format = swapchain_image_format_;
  image_view_info.subresourceRange.baseMipLevel = 0;
  image_view_info.subresourceRange.levelCount = 1;
  image_view_info.subresourceRange.baseArrayLayer = 0;
  image_view_info.subresourceRange.layerCount = 1;
  image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  image_view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  image_view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  image_view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  image_view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

  for (size_t i = 0; i < swapchain_images_.size(); ++i) {
    image_view_info.image = swapchain_images_[i];
    VK_CHECK(vkCreateImageView(device_, &image_view_info, nullptr,
                               &swapchain_image_views_[i]));
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

  bool can_get_memory_requirements = isDeviceExtensionEnabled(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
  bool has_dedicated_allocation = isDeviceExtensionEnabled(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
  bool can_get_buffer_device_address = isDeviceExtensionEnabled(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);

	if (can_get_memory_requirements && has_dedicated_allocation)
	{
		allocator_info.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
		vma_vulkan_func.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
		vma_vulkan_func.vkGetImageMemoryRequirements2KHR  = vkGetImageMemoryRequirements2KHR;
	}

  if(can_get_buffer_device_address)
  {
    allocator_info.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
  }

  auto result = vmaCreateAllocator(&allocator_info, &allocator_);
  if(result != VK_SUCCESS)
  {
    throw VulkanException(result, "failed to create vma allocator");
  }
}

VkDriver::~VkDriver() {
  for (auto &image_view : swapchain_image_views_) {
    vkDestroyImageView(device_, image_view, nullptr);
  }
  vkDestroySwapchainKHR(device_, swapchain_, nullptr);
  vkDestroyDevice(device_, nullptr);
  vkDestroySurfaceKHR(instance_, surface_, nullptr);
  vkDestroyInstance(instance_, nullptr);
}
} // namespace vk_engine
