#pragma once

#ifdef _WIN32
#include <windows.h>

namespace refii {
namespace kernel {

struct Mutex : CRITICAL_SECTION
{
    Mutex()
    {
        InitializeCriticalSection(this);
    }
    ~Mutex()
    {
        DeleteCriticalSection(this);
    }

    void lock()
    {
        EnterCriticalSection(this);
    }

    void unlock()
    {
        LeaveCriticalSection(this);
    }
};

} // namespace kernel
} // namespace refii

#else

#include <mutex>

namespace refii {
namespace kernel {

using Mutex = std::mutex;

} // namespace kernel
} // namespace refii

#endif
