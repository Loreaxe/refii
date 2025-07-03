#pragma once

#include <cstdint>
#include <cstring>

#ifdef _WIN32
    #include <Windows.h>
#else
    #include <errno.h>  // For errno in GetLastError fallback

    // ==================== Simulated Win32 Structures ====================

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

    union LARGE_INTEGER {
        struct {
            uint32_t LowPart;
            int32_t  HighPart;
        };
        int64_t QuadPart;
    };

    // ==================== File Attribute Constants ====================
    #define FILE_ATTRIBUTE_DIRECTORY        0x10
    #define FILE_ATTRIBUTE_NORMAL           0x80
    #define INVALID_FILE_ATTRIBUTES         0xFFFFFFFF

    // ==================== Access Mode Constants ====================
    #define GENERIC_READ                    0x80000000
    #define GENERIC_WRITE                   0x40000000
    #define FILE_READ_DATA                  0x00000001
    #define INVALID_FILE_SIZE               0xFFFFFFFF

    // ==================== File Pointer Move Methods ====================
    #define FILE_BEGIN                      0
    #define FILE_CURRENT                    1
    #define FILE_END                        2
    #define INVALID_SET_FILE_POINTER        0xFFFFFFFF

    // ==================== Share Mode Constants ====================
    #define FILE_SHARE_READ                 0x00000001
    #define FILE_SHARE_WRITE                0x00000002

    // ==================== Creation Disposition Constants ====================
    #define CREATE_NEW                      1
    #define CREATE_ALWAYS                   2
    #define OPEN_EXISTING                   3
    #define OPEN_ALWAYS                     4
    #define TRUNCATE_EXISTING               5

    // ==================== Error Code Emulation ====================
    #define ERROR_SUCCESS                   0x00000000
    #define ERROR_INVALID_FUNCTION          0x00000001
    #define ERROR_FILE_NOT_FOUND            0x00000002
    #define ERROR_PATH_NOT_FOUND            0x00000003
    #define ERROR_ACCESS_DENIED             0x00000005
    #define ERROR_INVALID_HANDLE            0x00000006
    #define ERROR_NOT_ENOUGH_MEMORY         0x00000008
    #define ERROR_INVALID_DATA              0x0000000D
    #define ERROR_OUTOFMEMORY               0x0000000E
    #define ERROR_INVALID_DRIVE             0x0000000F
    #define ERROR_NO_MORE_FILES             0x00000012
    #define ERROR_SHARING_VIOLATION         0x00000020
    #define ERROR_LOCK_VIOLATION            0x00000021
    #define ERROR_INVALID_PARAMETER         0x00000057
    #define ERROR_INSUFFICIENT_BUFFER       0x0000007A
    #define ERROR_BAD_ARGUMENTS             0x000000A0
    #define ERROR_ALREADY_EXISTS            0x000000B7
    #define ERROR_DEVICE_NOT_CONNECTED      0x0000048F
    #define ERROR_NOT_FOUND                 0x00000490
    #define ERROR_DIRECTORY                 0x00000010
    #define ERROR_NO_SUCH_USER              0x00000525

    // ==================== Fallback GetLastError ====================
    inline int GetLastError() {
        return errno;
    }

#endif

// ==================== Shared Constants (Cross-Platform) ====================

constexpr uint32_t INFINITE                         = 0xFFFFFFFF;
constexpr uint32_t S_OK                             = 0x00000000;
constexpr uint32_t STATUS_SUCCESS                   = 0x00000000;
constexpr uint32_t STATUS_TIMEOUT                   = 0x00000102;
constexpr uint32_t STATUS_WAIT_0                    = 0x00000000;
constexpr uint32_t STATUS_USER_APC                  = 0x000000C0;
constexpr uint32_t STATUS_FAIL_CHECK                = 0xC000022D;
constexpr uint32_t STATUS_ABANDONED_WAIT_0          = 0x00000080;
constexpr uint32_t STATUS_MUTANT_NOT_OWNED          = 0xC0000046;
constexpr uint32_t STATUS_PENDING                   = 0x00000103;
constexpr uint32_t STATUS_NOT_IMPLEMENTED           = 0xC0000002;
constexpr uint32_t STATUS_INVALID_PARAMETER         = 0xC000000D;
constexpr uint32_t STATUS_ACCESS_DENIED             = 0xC0000022;
constexpr uint32_t STATUS_OBJECT_NAME_NOT_FOUND     = 0xC0000034;
constexpr uint32_t STATUS_OBJECT_PATH_NOT_FOUND     = 0xC000003A;
constexpr uint32_t STATUS_NO_MEMORY                 = 0xC0000017;
constexpr uint32_t STATUS_BUFFER_TOO_SMALL          = 0xC0000023;

// ==================== Memory Protection Flags ====================

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

// ==================== Memory Allocation Flags ====================

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

// ==================== Boolean Constants ====================

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
