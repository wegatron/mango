#include <framework/utils/memory.h>

namespace utils
{
    LinearAllocator::LinearAllocator(const std::size_t size)
        : size_(size), buffer_(new std::byte[size]), resource_(buffer_, size)
    {
    }
}