#include <limits>

namespace vk_engine {
// Adds padding to multi-channel interleaved data by inserting dummy values, or
// discards trailing channels. This is useful for platforms that only accept
// 4-component data, since users often wish to submit (or receive) 3-component
// data.
template <typename componentType>
void reshapeImageData(void *dest, const void *src, size_t srcChannelCount,
          size_t dstChannelCount, size_t numSrcBytes) {
  const componentType maxValue = std::numeric_limits<componentType>::max();
  const componentType *in = (const componentType *)src;
  componentType *out = (componentType *)dest;
  const size_t pixels_count = (numSrcBytes / sizeof(componentType)) / srcChannelCount;
  const int minChannelCount = (srcChannelCount < dstChannelCount) ? srcChannelCount : dstChannelCount;
  for (size_t index = 0; index < pixels_count; ++index) {
    for (size_t channel = 0; channel < minChannelCount; ++channel) {
      out[channel] = in[channel];
    }
    for (size_t channel = srcChannelCount; channel < dstChannelCount;
         ++channel) {
      out[channel] = maxValue;
    }
    in += srcChannelCount;
    out += dstChannelCount;
  }
}

} // namespace vk_engine