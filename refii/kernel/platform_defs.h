#pragma once

#include <cstdint>
#include <cstring>

// ==================== Platform-Dependent Win32 Compatibility ====================

#ifdef _WIN32
    #include <Windows.h>
#else
    // Simulated WIN32_FIND_DATAA structure
    struct WIN32_FIND_DATAA {
        uint32_t dwFileAttributes;
        uint64_t ftCreationTime;
        uint64_t ftLastAccessTime;
        uint64_t ftLastWriteTime;
        uint32_t nFileSizeHigh;
        uint32_t nFileSizeLow;
        char     cFileName[260];
        char     cAlternateFileName[14];
    };

    // Simulated LARGE_INTEGER union
    union LARGE_INTEGER {
        struct {
            uint32_t LowPart;
            int32_t  HighPart;
        };
        int64_t QuadPart;
    };

    // File attribute constants
    #define FILE_ATTRIBUTE_DIRECTORY        0x10
    #define FILE_ATTRIBUTE_NORMAL           0x80
    #define INVALID_FILE_ATTRIBUTES         0xFFFFFFFF

    // Access mode constants
    #define GENERIC_READ                    0x80000000
    #define GENERIC_WRITE                   0x40000000
    #define FILE_READ_DATA                  0x00000001
    #define INVALID_FILE_SIZE               0xFFFFFFFF

    // File pointer move methods
    #define FILE_BEGIN                      0
    #define FILE_CURRENT                    1
    #define FILE_END                        2
    #define INVALID_SET_FILE_POINTER        0xFFFFFFFF
#endif

// ==================== Shared Constants (Cross-Platform) ====================

constexpr uint32_t INFINITE                 = 0xFFFFFFFF;
constexpr uint32_t STATUS_SUCCESS           = 0x00000000;
constexpr uint32_t STATUS_TIMEOUT           = 0x00000102;
constexpr uint32_t STATUS_WAIT_0            = 0x00000000;
constexpr uint32_t S_OK                     = 0x00000000;
constexpr uint32_t STATUS_USER_APC          = 0x000000C0;
constexpr uint32_t STATUS_FAIL_CHECK        = 0xC000022D;
constexpr uint32_t ERROR_SUCCESS            = 0x00000000;
constexpr uint32_t ERROR_NO_SUCH_USER       = 0x00000525;
constexpr uint32_t ERROR_NO_MORE_FILES      = 0x00000012;
constexpr uint32_t ERROR_PATH_NOT_FOUND     = 0x00000003;
constexpr uint32_t CREATE_ALWAYS            = 2;
constexpr uint32_t OPEN_EXISTING            = 3;
constexpr uint32_t STATUS_ABANDONED_WAIT_0  = 0x00000080;
constexpr uint32_t STATUS_MUTANT_NOT_OWNED  = 0xC0000046;
constexpr uint32_t ERROR_BAD_ARGUMENTS        = 0x000000A0;
constexpr uint32_t ERROR_DEVICE_NOT_CONNECTED = 0x0000048F;


// Protection constants
#ifndef PAGE_READWRITE
#define PAGE_READWRITE             0x04
#endif

#ifndef PAGE_EXECUTE_READWRITE
#define PAGE_EXECUTE_READWRITE     0x40
#endif

#ifndef PAGE_WRITECOPY
#define PAGE_WRITECOPY             0x08
#endif

#ifndef PAGE_EXECUTE_WRITECOPY
#define PAGE_EXECUTE_WRITECOPY     0x80
#endif

// Allocation type constants
#ifndef MEM_COMMIT
#define MEM_COMMIT                 0x1000
#endif

#ifndef MEM_RESERVE
#define MEM_RESERVE                0x2000
#endif

#ifndef MEM_DECOMMIT
#define MEM_DECOMMIT               0x4000
#endif

#ifndef MEM_RELEASE
#define MEM_RELEASE                0x8000
#endif

#ifndef MEM_PHYSICAL
#define MEM_PHYSICAL               0x00400000
#endif

#ifndef MEM_LARGE_PAGES
#define MEM_LARGE_PAGES            0x20000000
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// ==================== Debug Trap ====================

inline void kDebugTrap()
{
#if defined(_MSC_VER)
    __debugbreak();
#elif defined(__clang__) || defined(__GNUC__)
    __builtin_trap();
#else
    // fallback: do nothing
#endif
}
