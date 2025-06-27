#include "stdafx.h"
#include <kernel/function.h>

uint32_t QueryPerformanceCounterImpl(LARGE_INTEGER* lpPerformanceCount)
{
    lpPerformanceCount->QuadPart = ByteSwap(std::chrono::steady_clock::now().time_since_epoch().count());
    return TRUE;
}

uint32_t QueryPerformanceFrequencyImpl(LARGE_INTEGER* lpFrequency)
{
    constexpr auto Frequency = std::chrono::steady_clock::period::den / std::chrono::steady_clock::period::num;
    lpFrequency->QuadPart = ByteSwap(Frequency);
    return TRUE;
}

uint32_t GetTickCountImpl()
{
    return uint32_t(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
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

//GUEST_FUNCTION_HOOK(sub_824665A0, QueryPerformanceCounterImpl);
GUEST_FUNCTION_HOOK(sub_82CC6E78, QueryPerformanceFrequencyImpl);
//GUEST_FUNCTION_HOOK(sub_8248D098, GetTickCountImpl);

GUEST_FUNCTION_HOOK(sub_82CC2440, GlobalMemoryStatusImpl);
//
//// sprintf
//PPC_FUNC(sub_82BD4AE8)
//{
//    sub_831B1630(ctx, base);
//}

