#pragma once

#include <cstdint>
#include <cassert>

namespace refii {
namespace kernel {

// You can move this to a central header if reused
constexpr uint32_t STATUS_TIMEOUT = 0x00000102; // Common NT-style timeout value

struct KernelObject
{
    virtual ~KernelObject() = default;

    // Returns STATUS_TIMEOUT by default, but asserts to catch misuse during development
    virtual uint32_t Wait(uint32_t timeout)
    {
        assert(false && "❌ KernelObject::Wait() was called but not overridden.");
        return STATUS_TIMEOUT;
    }
};

} // namespace kernel
} // namespace refii
