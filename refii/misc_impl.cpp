#include "stdafx.h"
#include <kernel/function.h>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <cstdio>

#ifndef _WIN32
// Define missing Windows types and constants on Linux
using LARGE_INTEGER = struct {
    int64_t QuadPart;
};

#define TRUE 1

// Dummy implementation of OutputDebugStringA for Linux
void OutputDebugStringA(const char* str) {
    std::puts(str);
}
#endif

uint32_t QueryPerformanceCounterImpl(LARGE_INTEGER* lpPerformanceCount)
{
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    lpPerformanceCount->QuadPart = ByteSwap(std::chrono::duration_cast<std::chrono::nanoseconds>(now).count());
    return TRUE;
}

uint32_t QueryPerformanceFrequencyImpl(LARGE_INTEGER* lpFrequency)
{
    constexpr int64_t Frequency = std::chrono::steady_clock::period::den / std::chrono::steady_clock::period::num;
    lpFrequency->QuadPart = ByteSwap(Frequency);
    return TRUE;
}

uint32_t GetTickCountImpl()
{
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    return uint32_t(std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
}

void GlobalMemoryStatusImpl(XLPMEMORYSTATUS lpMemoryStatus)
{
    lpMemoryStatus->dwLength = sizeof(XMEMORYSTATUS);
    lpMemoryStatus->dwMemoryLoad = 0;
    lpMemoryStatus->dwTotalPhys = 0x20000000;
    lpMemoryStatus->dwAvailPhys = 0x20000000;
    lpMemoryStatus->dwTotalPageFile = 0x20000000;
    lpMemoryStatus->dwAvailPageFile = 0x20000000;
    lpMemoryStatus->dwTotalVirtual = 0x20000000;
    lpMemoryStatus->dwAvailVirtual = 0x20000000;
}

//GUEST_FUNCTION_HOOK(sub_82274140, GuestThread::SetThreadName);

//#ifdef _WIN32
GUEST_FUNCTION_HOOK(sub_82170010, OutputDebugStringA);

GUEST_FUNCTION_HOOK(sub_8221EA28, QueryPerformanceCounterImpl);
GUEST_FUNCTION_HOOK(sub_82CC6E78, QueryPerformanceFrequencyImpl);
GUEST_FUNCTION_HOOK(sub_822660B0, GetTickCountImpl);

GUEST_FUNCTION_HOOK(sub_82CC2440, GlobalMemoryStatusImpl);
//
//// sprintf
//PPC_FUNC(sub_82BD4AE8)
//{
//    sub_831B1630(ctx, base);
//}

void GameLogMsg(char* msg)
{
    LOG_UTILITY(msg);
}

GUEST_FUNCTION_HOOK(sub_82173070, GameLogMsg);


