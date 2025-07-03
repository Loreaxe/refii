#include <stdafx.h>
#include "guest_thread.h"
#include <kernel/obj/guest_memory.h>
#include <kernel/obj/guest_heap.h>
#include <kernel/function.h>
#include "ppc_context.h"
#include <kernel/kernel.h>
#include <kernel/platform_defs.h>

constexpr size_t PCR_SIZE = 0xAB0;
constexpr size_t TLS_SIZE = 0x100;
constexpr size_t TEB_SIZE = 0x2E0;
constexpr size_t STACK_SIZE = 0x40000;
constexpr size_t TOTAL_SIZE = PCR_SIZE + TLS_SIZE + TEB_SIZE + STACK_SIZE;

constexpr size_t TEB_OFFSET = PCR_SIZE + TLS_SIZE;

template<typename T>
constexpr T AlignDown(T value, size_t alignment) {
    static_assert(std::is_integral_v<T>, "AlignDown requires integral type");
    return value & ~(static_cast<T>(alignment) - 1);
}

GuestThreadContext::GuestThreadContext(uint32_t cpuNumber)
{
    assert(thread == nullptr);

    thread = (uint8_t*)refii::kernel::g_userHeap.Alloc(TOTAL_SIZE);
    memset(thread, 0, TOTAL_SIZE);

    *(uint32_t*)thread = ByteSwap(refii::kernel::g_memory.MapVirtual(thread + PCR_SIZE)); // tls pointer
    *(uint32_t*)(thread + 0x100) = ByteSwap(refii::kernel::g_memory.MapVirtual(thread + PCR_SIZE + TLS_SIZE)); // teb pointer
    *(thread + 0x10C) = cpuNumber;

    *(uint32_t*)(thread + PCR_SIZE + 0x10) = 0xFFFFFFFF; // that one TLS entry that felt quirky
    *(uint32_t*)(thread + PCR_SIZE + TLS_SIZE + 0x14C) = ByteSwap(GuestThread::GetCurrentThreadId()); // thread id

    ppcContext.r1.u64 = AlignDown(refii::kernel::g_memory.MapVirtual(thread + PCR_SIZE + TLS_SIZE + TEB_SIZE + STACK_SIZE), 16); // stack pointer
    ppcContext.r13.u64 = refii::kernel::g_memory.MapVirtual(thread);
    ppcContext.fpscr.loadFromHost();

    assert(GetPPCContext() == nullptr);
    SetPPCContext(ppcContext);
}

GuestThreadContext::~GuestThreadContext()
{
    refii::kernel::g_userHeap.Free(thread);
}

static void GuestThreadFunc(GuestThreadHandle* hThread)
{
    hThread->suspended.wait(true);
    GuestThread::Start(hThread->params);
}

GuestThreadHandle::GuestThreadHandle(const GuestThreadParams& params)
    : params(params), suspended((params.flags & 0x1) != 0), thread(GuestThreadFunc, this)
{
}

GuestThreadHandle::~GuestThreadHandle()
{
    if (thread.joinable())
        thread.join();
}

uint32_t GuestThreadHandle::Wait(uint32_t timeout)
{
    assert(timeout == INFINITE);

    if (thread.joinable())
        thread.join();

    return STATUS_WAIT_0;
}

uint32_t GuestThread::Start(const GuestThreadParams& params)
{
    const auto procMask = (uint8_t)(params.flags >> 24);
    const auto cpuNumber = procMask == 0 ? 0 : 7 - std::countl_zero(procMask);

    GuestThreadContext ctx(cpuNumber);
    ctx.ppcContext.r3.u64 = params.value;

    refii::kernel::g_memory.FindFunction(params.function)(ctx.ppcContext, refii::kernel::g_memory.base);

    return ctx.ppcContext.r3.u32;
}

static uint32_t GetThreadId(const std::thread::id& id)
{
    if constexpr (sizeof(id) == 4)
        return *reinterpret_cast<const uint32_t*>(&id);
    else
        return XXH32(&id, sizeof(id), 0);
}

GuestThreadHandle* GuestThread::Start(const GuestThreadParams& params, uint32_t* threadId)
{
    auto hThread = refii::kernel::CreateKernelObject<GuestThreadHandle>(params);

    if (threadId != nullptr)
        *threadId = GetThreadId(hThread->thread.get_id());

    return hThread;
}

uint32_t GuestThread::GetCurrentThreadId()
{
    return GetThreadId(std::this_thread::get_id());
}

void GuestThread::SetLastError(uint32_t error)
{
    auto* thread = (char*)refii::kernel::g_memory.Translate(GetPPCContext()->r13.u32);
    if (*(uint32_t*)(thread + 0x150))
    {
        // Program doesn't want errors
        return;
    }

    // TEB + 0x160 : Win32LastError
    *(uint32_t*)(thread + TEB_OFFSET + 0x160) = ByteSwap(error);
}

#ifdef _WIN32
void GuestThread::SetThreadName(uint32_t threadId, const char* name)
{
#pragma pack(push,8)
    const DWORD MS_VC_EXCEPTION = 0x406D1388;

    typedef struct tagTHREADNAME_INFO
    {
        DWORD dwType; // Must be 0x1000.
        LPCSTR szName; // Pointer to name (in user addr space).
        DWORD dwThreadID; // Thread ID (-1=caller thread).
        DWORD dwFlags; // Reserved for future use, must be zero.
    } THREADNAME_INFO;
#pragma pack(pop)

    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = name;
    info.dwThreadID = threadId;
    info.dwFlags = 0;

    __try
    {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
    }
}
#endif

void SetThreadNameImpl(uint32_t* name)
{
#ifdef _WIN32
    GuestThread::SetThreadName(0xFFFFFFFF, (const char*)name);
#endif
}

int GetThreadPriorityImpl(GuestThreadHandle* hThread)
{
#ifdef _WIN32
    return GetThreadPriority(hThread == refii::kernel::GetKernelObject(CURRENT_THREAD_HANDLE) ? GetCurrentThread() : hThread->thread.native_handle());
#else 
    return 0;
#endif
}

uint32_t SetThreadIdealProcessorImpl(GuestThreadHandle* hThread, uint32_t dwIdealProcessor)
{
    return 0;
}

GUEST_FUNCTION_HOOK(sub_82B3DAE8, SetThreadNameImpl);
//GUEST_FUNCTION_HOOK(sub_82466E00, GetThreadPriorityImpl);
GUEST_FUNCTION_HOOK(sub_82CC2670, SetThreadIdealProcessorImpl);
