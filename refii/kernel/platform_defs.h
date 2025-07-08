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

    inline void OutputDebugStringA(const char* str) {
        // On Linux, just print to stderr or discard
        std::fprintf(stderr, "[DebugString] %s\n", str ? str : "(null)");
    }

    // ==================== File Attribute Constants ====================
    #define FILE_ATTRIBUTE_DIRECTORY        0x10
    #define FILE_ATTRIBUTE_NORMAL           0x80
    #define INVALID_FILE_ATTRIBUTES         0xFFFFFFFF

    // ==================== Access Mode Constants ====================
    #define GENERIC_READ                    0x80000000
    #define GENERIC_WRITE                   0x40000000
    #define FILE_READ_DATA                  0x00000001
    #define INVALID_FILE_SIZE               0xFFFFFFFF

    // ==================== File Creation / Attribute Flags ====================
    #define FILE_FLAG_WRITE_THROUGH         0x80000000
    #define FILE_FLAG_OVERLAPPED            0x40000000
    #define FILE_FLAG_NO_BUFFERING          0x20000000
    #define FILE_FLAG_RANDOM_ACCESS         0x10000000
    #define FILE_FLAG_SEQUENTIAL_SCAN       0x08000000
    #define FILE_FLAG_DELETE_ON_CLOSE       0x04000000
    #define FILE_FLAG_BACKUP_SEMANTICS      0x02000000
    #define FILE_FLAG_POSIX_SEMANTICS       0x01000000
    #define FILE_FLAG_OPEN_REPARSE_POINT    0x00200000
    #define FILE_FLAG_OPEN_NO_RECALL        0x00100000
    #define FILE_FLAG_FIRST_PIPE_INSTANCE   0x00080000

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

    // ==================== Wait Return Codes ====================
    #define WAIT_OBJECT_0                   0x00000000
    #define WAIT_ABANDONED                  0x00000080
    #define WAIT_TIMEOUT                    0x00000102
    #define WAIT_FAILED                     0xFFFFFFFF
    #define WAIT_IO_COMPLETION              0x000000C0

    // ==================== HRESULT & Return Codes ====================
    #define S_FALSE                         0x00000001

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
    #define ERROR_INVALID_ADDRESS           0x0000001E
    #define ERROR_SEEK                      0x00000084  
    #define ERROR_HANDLE_EOF                0x00000026  
    #define ERROR_READ_FAULT                0x0000001E
    #define ERROR_INVALID_NAME              0x0000007B
    #define ERROR_NOT_SUPPORTED             0x00000032
    #define ERROR_FILE_EXISTS               0x00000050
    #define ERROR_CANNOT_MAKE               0x00000052
    #define ERROR_DIR_NOT_EMPTY             0x00000091
    #define ERROR_NOT_READY                 0x00000015
    #define ERROR_DISK_FULL                 0x00000070
    #define ERROR_FILE_CORRUPT              0x0000066A
    #define ERROR_NOACCESS                  0x0000003E

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
constexpr uint32_t STATUS_INVALID_HANDLE            = 0xC0000008;
constexpr uint32_t STATUS_OBJECT_TYPE_MISMATCH      = 0xC0000024;
constexpr uint32_t STATUS_THREAD_IS_TERMINATING     = 0xC000004B;
constexpr uint32_t STATUS_INVALID_PAGE_PROTECTION   = 0xC0000045;
constexpr uint32_t STATUS_ALREADY_COMMITTED         = 0xC0000021;
constexpr uint32_t STATUS_CONFLICTING_ADDRESSES     = 0xC0000018;
constexpr uint32_t STATUS_INVALID_SYSTEM_SERVICE    = 0xC000001C;
constexpr uint32_t STATUS_INVALID_INFO_CLASS        = 0xC0000003;
constexpr uint32_t STATUS_END_OF_FILE               = 0xC0000011;
constexpr uint32_t STATUS_NOT_SUPPORTED             = 0xC00000BB;

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
