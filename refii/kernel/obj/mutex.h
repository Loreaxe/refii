#pragma once

#ifdef _WIN32
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

#else

using Mutex = std::mutex;

#endif
}
}
