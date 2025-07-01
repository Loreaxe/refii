#include <stdafx.h>
#include "kernel.h"
#include <locale/locale.h>
#include <user/config.h>
#include "platform_defs.h"

refii::kernel::Mutex refii::kernel::g_kernelLock;

void refii::kernel::DestroyKernelObject(KernelObject* obj)
{
    obj->~KernelObject();
    g_userHeap.Free(obj);
}

void refii::kernel::CloseKernelObject(XDISPATCHER_HEADER& header)
{
    if (header.WaitListHead.Flink != OBJECT_SIGNATURE)
    {
        return;
    }

    DestroyKernelObject(header.WaitListHead.Blink);
}

uint32_t refii::kernel::GetKernelHandle(KernelObject* obj)
{
    assert(obj != GetInvalidKernelObject());
    return g_memory.MapVirtual(obj);
}

void refii::kernel::DestroyKernelObject(uint32_t handle)
{
    DestroyKernelObject(GetKernelObject(handle));
}

bool refii::kernel::IsKernelObject(uint32_t handle)
{
    return (handle & 0x80000000) != 0;
}

bool refii::kernel::IsKernelObject(void* obj)
{
    return IsKernelObject(g_memory.MapVirtual(obj));
}

bool refii::kernel::IsInvalidKernelObject(void* obj)
{
    return obj == GetInvalidKernelObject();
}

uint32_t refii::kernel::GuestTimeoutToMilliseconds(be<int64_t>* timeout)
{
    return timeout ? (*timeout * -1) / 10000 : INFINITE;
}

uint32_t refii::kernel::KeDelayExecutionThread(uint32_t WaitMode, bool Alertable, be<int64_t>* Timeout)
{
    // We don't do async file reads.
    if (Alertable)
        return STATUS_USER_APC;

    uint32_t timeout = GuestTimeoutToMilliseconds(Timeout);

#ifdef _WIN32
    Sleep(timeout);
#else
    if (timeout == 0)
        std::this_thread::yield();
    else
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
#endif

    return STATUS_SUCCESS;
}

void refii::kernel::KeQuerySystemTime(be<uint64_t>* time)
{
    constexpr int64_t FILETIME_EPOCH_DIFFERENCE = 116444736000000000LL;

    auto now = std::chrono::system_clock::now();
    auto timeSinceEpoch = now.time_since_epoch();

    int64_t currentTime100ns = std::chrono::duration_cast<std::chrono::duration<int64_t, std::ratio<1, 10000000>>>(timeSinceEpoch).count();
    currentTime100ns += FILETIME_EPOCH_DIFFERENCE;

    *time = currentTime100ns;
}

uint32_t refii::kernel::KeWaitForMultipleObjects(
    uint32_t Count,
    xpointer<XDISPATCHER_HEADER>* Objects,
    uint32_t WaitType,
    uint32_t WaitReason,
    uint32_t WaitMode,
    uint32_t Alertable,
    be<int64_t>* Timeout)
{
    const uint64_t timeout = GuestTimeoutToMilliseconds(Timeout);

    auto deadline = (timeout == INFINITE)
        ? std::chrono::steady_clock::time_point::max()
        : std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout);

    if (WaitType == 0) // Wait all
    {
        for (size_t i = 0; i < Count; i++)
        {
            uint32_t remaining = INFINITE;  // <-- ✅ missing semicolon was here

            if (timeout != INFINITE)
            {
                auto now = std::chrono::steady_clock::now();
                if (now >= deadline)
                    return STATUS_TIMEOUT;

                remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count();
            }

            uint32_t result = QueryKernelObject<Event>(*Objects[i])->Wait(remaining);
            if (result != STATUS_SUCCESS)
                return result;
        }
    }
    else
    {
        thread_local std::vector<Event*> s_events;
        s_events.resize(Count);

        for (size_t i = 0; i < Count; i++)
            s_events[i] = QueryKernelObject<Event>(*Objects[i]);

        while (true)
        {
            uint32_t generation = g_keSetEventGeneration.load();

            for (size_t i = 0; i < Count; i++)
            {
                if (s_events[i]->Wait(0) == STATUS_SUCCESS)
                {
                    return STATUS_WAIT_0 + i;
                }
            }

            if (timeout != INFINITE && std::chrono::steady_clock::now() >= deadline)
                return STATUS_TIMEOUT;

            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            // Check if generation changed
            if (g_keSetEventGeneration.load() != generation)
                continue;
        }
    }

    return STATUS_SUCCESS;
}

uint32_t refii::kernel::RaiseIrqlToDpcLevel()
{
    return 0;
}

void refii::kernel::KfLowerIrql() {}

uint32_t refii::kernel::KeReleaseSemaphore(XKSEMAPHORE* semaphore, uint32_t increment, uint32_t adjustment, uint32_t wait)
{
    auto* object = QueryKernelObject<Semaphore>(semaphore->Header);
    object->Release(adjustment, nullptr);
    return STATUS_SUCCESS;
}

uint32_t refii::kernel::KeResumeThread(GuestThreadHandle* object)
{
    assert(object != GetKernelObject(CURRENT_THREAD_HANDLE));

    object->suspended = false;
    object->suspended.notify_all();
    return 0;
}

void refii::kernel::KeInitializeSemaphore(XKSEMAPHORE* semaphore, uint32_t count, uint32_t limit)
{
    semaphore->Header.Type = 5;
    semaphore->Header.SignalState = count;
    semaphore->Limit = limit;

    auto* object = QueryKernelObject<Semaphore>(semaphore->Header);
}

uint32_t refii::kernel::KeTryToAcquireSpinLockAtRaisedIrql(uint32_t* spinLock)
{
    std::atomic_ref spinLockRef(*spinLock);
    uint32_t expected = 0;
    return spinLockRef.compare_exchange_weak(expected, g_ppcContext->r13.u32);
}

void refii::kernel::TimeStampBundle()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::DebugMonitorData()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::CertMonitorData()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::KeSetBasePriorityThread(GuestThreadHandle* hThread, int priority)
{
#ifdef _WIN32
    if (priority == 16)
    {
        priority = 15;
    }
    else if (priority == -16)
    {
        priority = -15;
    }

    SetThreadPriority(hThread == GetKernelObject(CURRENT_THREAD_HANDLE) ? GetCurrentThread() : hThread->thread.native_handle(), priority);
#endif
}

void refii::kernel::KeQueryBasePriorityThread()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t refii::kernel::KeSetAffinityThread(uint32_t Thread, uint32_t Affinity, be<uint32_t>* lpPreviousAffinity)
{
    if (lpPreviousAffinity)
        *lpPreviousAffinity = 2;

    return 0;
}

void refii::kernel::KeBugCheckEx()
{
    kDebugTrap();
}

uint32_t refii::kernel::KeGetCurrentProcessType()
{
    return 1;
}

void refii::kernel::ExThreadObjectType()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::KfReleaseSpinLock(uint32_t* spinLock)
{
    std::atomic_ref spinLockRef(*spinLock);
    spinLockRef = 0;
}

void refii::kernel::KfAcquireSpinLock(uint32_t* spinLock)
{
    std::atomic_ref spinLockRef(*spinLock);

    while (true)
    {
        uint32_t expected = 0;
        if (spinLockRef.compare_exchange_weak(expected, g_ppcContext->r13.u32))
            break;

        std::this_thread::yield();
    }
}

uint64_t refii::kernel::KeQueryPerformanceFrequency()
{
    return 49875000;
}

void refii::kernel::KeReleaseSpinLockFromRaisedIrql(uint32_t* spinLock)
{
    std::atomic_ref spinLockRef(*spinLock);
    spinLockRef = 0;
}

void refii::kernel::KeAcquireSpinLockAtRaisedIrql(uint32_t* spinLock)
{
    std::atomic_ref spinLockRef(*spinLock);

    while (true)
    {
        uint32_t expected = 0;
        if (spinLockRef.compare_exchange_weak(expected, g_ppcContext->r13.u32))
            break;

        std::this_thread::yield();
    }
}

uint32_t refii::kernel::KiApcNormalRoutineNop()
{
    return 0;
}

uint32_t refii::kernel::XAudioGetVoiceCategoryVolumeChangeMask(uint32_t Driver, be<uint32_t>* Mask)
{
    *Mask = 0;
    return 0;
}

void refii::kernel::XexGetProcedureAddress()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::XexGetModuleSection()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::XNotifyPositionUI()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::XAudioGetVoiceCategoryVolume()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::KeEnableFpuExceptions()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::XeKeysConsolePrivateKeySign()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::XeKeysConsoleSignatureVerification()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::RtlTimeFieldsToTime()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::RtlInitAnsiString(XANSI_STRING* destination, char* source)
{
    const uint16_t length = source ? (uint16_t)strlen(source) : 0;
    destination->Length = length;
    destination->MaximumLength = length + 1;
    destination->Buffer = source;
}

void refii::kernel::vsprintf_x()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::DbgPrint()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::__C_specific_handler_x()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::RtlNtStatusToDosError()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t refii::kernel::RtlUnicodeToMultiByteN(char* MultiByteString, uint32_t MaxBytesInMultiByteString, be<uint32_t>* BytesInMultiByteString, const be<uint16_t>* UnicodeString, uint32_t BytesInUnicodeString)
{
    const auto reqSize = BytesInUnicodeString / sizeof(uint16_t);

    if (BytesInMultiByteString)
        *BytesInMultiByteString = reqSize;

    if (reqSize > MaxBytesInMultiByteString)
        return STATUS_FAIL_CHECK;

    for (size_t i = 0; i < reqSize; i++)
    {
        const auto c = UnicodeString[i].get();

        MultiByteString[i] = c < 256 ? c : '?';
    }

    return STATUS_SUCCESS;
}

void refii::kernel::ObDereferenceObject()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t refii::kernel::ObReferenceObjectByHandle(uint32_t handle, uint32_t objectType, be<uint32_t>* object)
{
    *object = handle;
    return 0;
}

void refii::kernel::RtlImageXexHeaderField()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::MmFreePhysicalMemory(uint32_t type, uint32_t guestAddress)
{
    if (guestAddress != NULL)
        g_userHeap.Free(g_memory.Translate(guestAddress));
}

void refii::kernel::RtlFillMemoryUlong()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::RtlCompareMemoryUlong()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::RtlRaiseException_x()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::_vsnprintf_x()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::sprintf_x()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::ExRegisterTitleTerminateNotification()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::ObDeleteSymbolicLink()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::ObCreateSymbolicLink()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::_vswprintf_x()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::_vscwprintf_x()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::_swprintf_x()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::_snwprintf_x()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::XeCryptBnQwBeSigVerify()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::XeKeysGetKey()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::XeCryptRotSumSha()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::XeCryptSha()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t refii::kernel::RtlMultiByteToUnicodeN(be<uint16_t>* UnicodeString, uint32_t MaxBytesInUnicodeString, be<uint32_t>* BytesInUnicodeString, const char* MultiByteString, uint32_t BytesInMultiByteString)
{
    uint32_t length = std::min(MaxBytesInUnicodeString / 2, BytesInMultiByteString);

    for (size_t i = 0; i < length; i++)
        UnicodeString[i] = MultiByteString[i];

    if (BytesInUnicodeString != nullptr)
        *BytesInUnicodeString = length * 2;

    return STATUS_SUCCESS;
}

void refii::kernel::DbgBreakPoint()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::RtlCompareStringN()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::_snprintf_x()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::RtlTimeToTimeFields()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::RtlFreeAnsiString()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::RtlUnicodeStringToAnsiString()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::RtlInitUnicodeString()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::XexLoadImageHeaders()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::ObOpenObjectByName()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::ObReferenceObjectByName()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::RtlUpcaseUnicodeChar()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::ObIsTitleObject()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::ObReferenceObject()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::XMsgStartIORequestEx()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::XexGetModuleHandle()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t refii::kernel::XMsgInProcessCall(uint32_t app, uint32_t message, be<uint32_t>* param1, be<uint32_t>* param2)
{
    if (message == 0x7001B)
    {
        uint32_t* ptr = (uint32_t*)g_memory.Translate(param1[1]);
        ptr[0] = 0;
        ptr[1] = 0;
    }

    return 0;
}

void refii::kernel::XexExecutableModuleHandle()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::ExLoadedCommandLine()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t refii::kernel::XGetGameRegion()
{
    if (Config::Language == ELanguage::Japanese)
        return 0x0101;

    return 0x03FF;
}

uint32_t refii::kernel::XMsgStartIORequest(uint32_t App, uint32_t Message, XXOVERLAPPED* lpOverlapped, void* Buffer, uint32_t szBuffer)
{
    return STATUS_SUCCESS;
}

uint32_t refii::kernel::XGetLanguage()
{
    return (uint32_t)::Config::Language.Value;
}

uint32_t refii::kernel::XGetAVPack()
{
    return 0;
}

uint32_t refii::kernel::XexCheckExecutablePrivilege()
{
    return 0;
}

uint32_t refii::kernel::ExGetXConfigSetting(uint16_t Category, uint16_t Setting, void* Buffer, uint16_t SizeOfBuffer, be<uint32_t>* RequiredSize)
{
    uint32_t data[4]{};

    switch (Category)
    {
        // XCONFIG_SECURED_CATEGORY
    case 0x0002:
    {
        switch (Setting)
        {
            // XCONFIG_SECURED_AV_REGION
        case 0x0002:
            data[0] = ByteSwap(0x00001000); // USA/Canada
            break;

        default:
            return 1;
        }
    }

    case 0x0003:
    {
        switch (Setting)
        {
        case 0x0001: // XCONFIG_USER_TIME_ZONE_BIAS
        case 0x0002: // XCONFIG_USER_TIME_ZONE_STD_NAME
        case 0x0003: // XCONFIG_USER_TIME_ZONE_DLT_NAME
        case 0x0004: // XCONFIG_USER_TIME_ZONE_STD_DATE
        case 0x0005: // XCONFIG_USER_TIME_ZONE_DLT_DATE
        case 0x0006: // XCONFIG_USER_TIME_ZONE_STD_BIAS
        case 0x0007: // XCONFIG_USER_TIME_ZONE_DLT_BIAS
            data[0] = 0;
            break;

            // XCONFIG_USER_LANGUAGE
        case 0x0009:
            data[0] = ByteSwap((uint32_t)Config::Language.Value);
            break;

            // XCONFIG_USER_VIDEO_FLAGS
        case 0x000A:
            data[0] = ByteSwap(0x00040000);
            break;

            // XCONFIG_USER_RETAIL_FLAGS
        case 0x000C:
            data[0] = ByteSwap(1);
            break;

            // XCONFIG_USER_COUNTRY
        case 0x000E:
            data[0] = ByteSwap(103);
            break;

        default:
            return 1;
        }
    }
    }

    *RequiredSize = 4;
    memcpy(Buffer, data, std::min((size_t)SizeOfBuffer, sizeof(data)));

    return 0;
}

uint32_t refii::kernel::ExCreateThread(be<uint32_t>* handle, uint32_t stackSize, be<uint32_t>* threadId, uint32_t xApiThreadStartup, uint32_t startAddress, uint32_t startContext, uint32_t creationFlags)
{
    LOGF_UTILITY("0x{:X}, 0x{:X}, 0x{:X}, 0x{:X}, 0x{:X}, 0x{:X}, 0x{:X}",
        (intptr_t)handle, stackSize, (intptr_t)threadId, xApiThreadStartup, startAddress, startContext, creationFlags);

    uint32_t hostThreadId;

    *handle = refii::kernel::GetKernelHandle(GuestThread::Start({ startAddress, startContext, creationFlags }, &hostThreadId));

    if (threadId != nullptr)
        *threadId = hostThreadId;

    return 0;
}

void refii::kernel::ExTerminateThread()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::KeLeaveCriticalRegion()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::RtlLeaveCriticalSection(XRTL_CRITICAL_SECTION* cs)
{
    cs->RecursionCount--;

    if (cs->RecursionCount != 0)
        return;

    std::atomic_ref owningThread(cs->OwningThread);
    owningThread.store(0);
    owningThread.notify_one();
}

void refii::kernel::RtlEnterCriticalSection(XRTL_CRITICAL_SECTION* cs)
{
    uint32_t thisThread = g_ppcContext->r13.u32;
    assert(thisThread != NULL);

    std::atomic_ref owningThread(cs->OwningThread);

    while (true)
    {
        uint32_t previousOwner = 0;

        if (owningThread.compare_exchange_weak(previousOwner, thisThread) || previousOwner == thisThread)
        {
            cs->RecursionCount++;
            return;
        }

        owningThread.wait(previousOwner);
    }
}

uint32_t refii::kernel::RtlInitializeCriticalSection(XRTL_CRITICAL_SECTION* cs)
{
    cs->Header.Absolute = 0;
    cs->LockCount = -1;
    cs->RecursionCount = 0;
    cs->OwningThread = 0;

    return 0;
}

bool refii::kernel::RtlTryEnterCriticalSection(XRTL_CRITICAL_SECTION* cs)
{
    uint32_t thisThread = g_ppcContext->r13.u32;
    assert(thisThread != NULL);

    std::atomic_ref owningThread(cs->OwningThread);

    uint32_t previousOwner = 0;

    if (owningThread.compare_exchange_weak(previousOwner, thisThread) || previousOwner == thisThread)
    {
        cs->RecursionCount++;
        return true;
    }

    return false;
}

void refii::kernel::RtlInitializeCriticalSectionAndSpinCount(XRTL_CRITICAL_SECTION* cs, uint32_t spinCount)
{
    cs->Header.Absolute = (spinCount + 255) >> 8;
    cs->LockCount = -1;
    cs->RecursionCount = 0;
    cs->OwningThread = 0;
}

void refii::kernel::RtlUnwind_x()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::RtlCaptureContext_x()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::KeEnterCriticalRegion()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::KeBugCheck()
{
    kDebugTrap();
}

void refii::kernel::KeLockL2()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::KeUnlockL2()
{
    LOG_UTILITY("!!! STUB !!!");
}

bool refii::kernel::KeSetEvent(XKEVENT* pEvent, uint32_t Increment, bool Wait)
{
    bool result = QueryKernelObject<Event>(*pEvent)->Set();

    ++g_keSetEventGeneration;
    g_keSetEventGeneration.notify_all();

    return result;
}

bool refii::kernel::KeResetEvent(XKEVENT* pEvent)
{
    return QueryKernelObject<Event>(*pEvent)->Reset();
}

uint32_t refii::kernel::KeWaitForSingleObject(XDISPATCHER_HEADER* Object, uint32_t WaitReason, uint32_t WaitMode, bool Alertable, be<int64_t>* Timeout)
{
    const uint32_t timeout = GuestTimeoutToMilliseconds(Timeout);

    switch (Object->Type)
    {
    case 0:
    case 1:
        QueryKernelObject<Event>(*Object)->Wait(timeout);
        break;

    case 5:
        QueryKernelObject<Semaphore>(*Object)->Wait(timeout);
        break;

    default:
        assert(false && "Unrecognized kernel object type.");
        return STATUS_TIMEOUT;
    }

    return STATUS_SUCCESS;
}

uint32_t& refii::kernel::TlsGetValueRef(size_t index)
{
    // Having this a global thread_local variable
    // for some reason crashes on boot in debug builds.
    thread_local std::vector<uint32_t> s_tlsValues;

    if (s_tlsValues.size() <= index)
    {
        s_tlsValues.resize(index + 1, 0);
    }

    return s_tlsValues[index];
}

uint32_t refii::kernel::KeTlsGetValue(uint32_t dwTlsIndex)
{
    return TlsGetValueRef(dwTlsIndex);
}

uint32_t refii::kernel::KeTlsSetValue(uint32_t dwTlsIndex, uint32_t lpTlsValue)
{
    TlsGetValueRef(dwTlsIndex) = lpTlsValue;
    return TRUE;
}

uint32_t refii::kernel::KeTlsAlloc()
{
    std::lock_guard<Mutex> lock(g_tlsAllocationMutex);
    if (!g_tlsFreeIndices.empty())
    {
        size_t index = g_tlsFreeIndices.back();
        g_tlsFreeIndices.pop_back();
        return index;
    }

    return g_tlsNextIndex++;
}

uint32_t refii::kernel::KeTlsFree(uint32_t dwTlsIndex)
{
    std::lock_guard<Mutex> lock(g_tlsAllocationMutex);
    g_tlsFreeIndices.push_back(dwTlsIndex);
    return TRUE;
}

void refii::kernel::NtOpenFile()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t refii::kernel::NtCreateFile(be<uint32_t>* FileHandle, uint32_t DesiredAccess, XOBJECT_ATTRIBUTES* Attributes, XIO_STATUS_BLOCK* IoStatusBlock, uint64_t* AllocationSize, uint32_t FileAttributes, uint32_t ShareAccess, uint32_t CreateDisposition, uint32_t CreateOptions)
{
    LOG_UTILITY("!!! STUB !!!");
    return 0;
}

void refii::kernel::NtWriteFile()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::NtSetInformationFile()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::NtQueryInformationFile()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::NtQueryVolumeInformationFile()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::NtQueryDirectoryFile()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::NtReadFileScatter()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::NtReadFile()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::NtQueryFullAttributesFile()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::NtFlushBuffersFile()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::NtQueryVirtualMemory()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t refii::kernel::NtSuspendThread(GuestThreadHandle* hThread, uint32_t* suspendCount)
{
    assert(hThread != GetKernelObject(CURRENT_THREAD_HANDLE) && hThread->thread.get_id() == std::this_thread::get_id());

    hThread->suspended = true;
    hThread->suspended.wait(true);

    return S_OK;
}

uint32_t refii::kernel::NtFreeVirtualMemory(uint32_t processHandle, be<uint32_t>* baseAddress, be<uint32_t>* regionSize, uint32_t freeType)
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t refii::kernel::NtAllocateVirtualMemory(uint32_t processHandle, be<uint32_t>* baseAddress, uint32_t zeroBits, be<uint32_t>* regionSize, uint32_t allocationType, uint32_t protect)
{
    LOG_UTILITY("!!! STUB !!!");
    return 0;
}

uint32_t refii::kernel::NtWaitForSingleObjectEx(uint32_t Handle, uint32_t WaitMode, uint32_t Alertable, be<int64_t>* Timeout)
{
    uint32_t timeout = GuestTimeoutToMilliseconds(Timeout);

    if (IsKernelObject(Handle))
    {
        return GetKernelObject(Handle)->Wait(timeout);
    }
    else
    {
        assert(false && "Unrecognized handle value.");
    }

    return STATUS_TIMEOUT;
}

uint32_t refii::kernel::NtCreateEvent(be<uint32_t>* handle, void* objAttributes, uint32_t eventType, uint32_t initialState)
{
    *handle = GetKernelHandle(CreateKernelObject<Event>(!eventType, !!initialState));
    return 0;
}

void refii::kernel::NtDuplicateObject()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t refii::kernel::NtCreateMutant(be<uint32_t>* handle, void* objAttributes, uint32_t initialOwner)
{
    *handle = GetKernelHandle(CreateKernelObject<Mutant>(!!initialOwner));
    return STATUS_SUCCESS;
}

uint32_t refii::kernel::NtReleaseMutant(Mutant* handle, be<uint32_t>* previousCount)
{
    if (previousCount)
    {
        *previousCount = handle->recursionCount.load();
    }

    return handle->Release();
}

void refii::kernel::NtDeviceIoControlFile()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t refii::kernel::NtPulseEvent(Event* handle, uint32_t* previousState)
{
    handle->Set();
    handle->Reset();
    return 0;
}

uint32_t refii::kernel::NtYieldExecution()
{
    std::this_thread::yield();
    return STATUS_SUCCESS;
}

uint32_t refii::kernel::NtClose(uint32_t handle)
{
    if (handle == GUEST_INVALID_HANDLE_VALUE)
        return 0xFFFFFFFF;

    if (IsKernelObject(handle))
    {
        DestroyKernelObject(handle);
        return 0;
    }
    else
    {
        assert(false && "Unrecognized kernel object.");
        return 0xFFFFFFFF;
    }
}

uint32_t refii::kernel::NtClearEvent(Event* handle, uint32_t* previousState)
{
    handle->Reset();
    return 0;
}

uint32_t refii::kernel::NtResumeThread(GuestThreadHandle* hThread, uint32_t* suspendCount)
{
    assert(hThread != GetKernelObject(CURRENT_THREAD_HANDLE));

    hThread->suspended = false;
    hThread->suspended.notify_all();

    return S_OK;
}

uint32_t refii::kernel::NtSetEvent(Event* handle, uint32_t* previousState)
{
    handle->Set();
    return 0;
}

uint32_t refii::kernel::NtCreateSemaphore(be<uint32_t>* Handle, XOBJECT_ATTRIBUTES* ObjectAttributes, uint32_t InitialCount, uint32_t MaximumCount)
{
    *Handle = GetKernelHandle(CreateKernelObject<Semaphore>(InitialCount, MaximumCount));
    return STATUS_SUCCESS;
}

uint32_t refii::kernel::NtReleaseSemaphore(Semaphore* Handle, uint32_t ReleaseCount, int32_t* PreviousCount)
{
    uint32_t previousCount;
    Handle->Release(ReleaseCount, &previousCount);

    if (PreviousCount != nullptr)
        *PreviousCount = ByteSwap(previousCount);

    return STATUS_SUCCESS;
}

void refii::kernel::NtWaitForMultipleObjectsEx()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t refii::kernel::RtlAllocateHeap(uint32_t heapHandle, uint32_t flags, uint32_t size)
{
    void* ptr = g_userHeap.Alloc(size);
    if ((flags & 0x8) != 0)
        memset(ptr, 0, size);

    assert(ptr);
    return g_memory.MapVirtual(ptr);
}

uint32_t refii::kernel::RtlReAllocateHeap(uint32_t heapHandle, uint32_t flags, uint32_t memoryPointer, uint32_t size)
{
    void* ptr = g_userHeap.Alloc(size);
    if ((flags & 0x8) != 0)
        memset(ptr, 0, size);

    if (memoryPointer != 0)
    {
        void* oldPtr = g_memory.Translate(memoryPointer);
        memcpy(ptr, oldPtr, std::min<size_t>(size, g_userHeap.Size(oldPtr)));
        g_userHeap.Free(oldPtr);
    }

    assert(ptr);
    return g_memory.MapVirtual(ptr);
}

uint32_t refii::kernel::RtlFreeHeap(uint32_t heapHandle, uint32_t flags, uint32_t memoryPointer)
{
    if (memoryPointer != NULL)
        g_userHeap.Free(g_memory.Translate(memoryPointer));

    return true;
}

uint32_t refii::kernel::RtlSizeHeap(uint32_t heapHandle, uint32_t flags, uint32_t memoryPointer)
{
    if (memoryPointer != NULL)
        return (uint32_t)g_userHeap.Size(g_memory.Translate(memoryPointer));

    return 0;
}

uint32_t refii::kernel::XAllocMem(uint32_t size, uint32_t flags)
{
    void* ptr = (flags & 0x80000000) != 0 ?
        g_userHeap.AllocPhysical(size, (1ull << ((flags >> 24) & 0xF))) :
        g_userHeap.Alloc(size);

    if ((flags & 0x40000000) != 0)
        memset(ptr, 0, size);

    assert(ptr);
    return g_memory.MapVirtual(ptr);
}

void refii::kernel::XFreeMem(uint32_t baseAddress, uint32_t flags)
{
    if (baseAddress != NULL)
        g_userHeap.Free(g_memory.Translate(baseAddress));
}

void refii::kernel::MmQueryStatistics()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::ExFreePool()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t refii::kernel::VirtualAlloc(uint32_t lpAddress, uint32_t dwSize, uint32_t flAllocationType, uint32_t flProtect)
{
    LOGF_UTILITY("VirtualAlloc: lpAddress=0x{:x}, dwSize=0x{:x}, flAllocationType=0x{:x}, flProtect=0x{:x}",
        lpAddress, dwSize, flAllocationType, flProtect);

    if (dwSize == 0)
    {
        LOGF_UTILITY("VirtualAlloc: Invalid size 0");
        return 0;
    }

    // Convert VirtualAlloc flags to XAllocMem flags
    uint32_t xallocFlags = 0;

    // Handle zero-initialization
    if (flProtect & (PAGE_READWRITE | PAGE_EXECUTE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_WRITECOPY))
    {
        xallocFlags |= 0x40000000; // XALLOC_MEMTYPE_ZERO
    }

    // Handle physical memory
    if (flAllocationType & MEM_PHYSICAL)
    {
        xallocFlags |= 0x80000000; // XALLOC_MEMTYPE_PHYSICAL
    }

    // Handle large pages (use 2MB alignment)
    if (flAllocationType & MEM_LARGE_PAGES)
    {
        xallocFlags |= (21 << 24); // 2MB alignment (2^21)
    }

    // Use existing working XAllocMem function
    uint32_t result = XAllocMem(dwSize, xallocFlags);

    if (result != 0)
    {
        LOGF_UTILITY("VirtualAlloc: Success, allocated 0x{:x} bytes at 0x{:x}", dwSize, result);
    }
    else
    {
        LOGF_UTILITY("VirtualAlloc: Failed to allocate 0x{:x} bytes", dwSize);
    }

    return result;
}

uint32_t refii::kernel::VirtualFree(uint32_t lpAddress, uint32_t dwSize, uint32_t dwFreeType)
{
    LOGF_UTILITY("VirtualFree: lpAddress=0x{:x}, dwSize=0x{:x}, dwFreeType=0x{:x}",
        lpAddress, dwSize, dwFreeType);

    if (lpAddress == 0)
    {
        LOGF_UTILITY("VirtualFree: Invalid address 0");
        return 0;
    }

    // For MEM_RELEASE, dwSize should be 0 (but we'll ignore it for simplicity)
    if (dwFreeType & MEM_RELEASE)
    {
        // Use existing working XFreeMem function
        XFreeMem(lpAddress, 0);
        LOGF_UTILITY("VirtualFree: Released memory at 0x{:x}", lpAddress);
        return 1;
    }
    else if (dwFreeType & MEM_DECOMMIT)
    {
        // For emulation, just return success for decommit
        LOGF_UTILITY("VirtualFree: Decommit at 0x{:x} (emulated)", lpAddress);
        return 1;
    }
    else
    {
        LOGF_UTILITY("VirtualFree: Invalid free type 0x{:x}", dwFreeType);
        return 0;
    }
}

uint32_t refii::kernel::MmGetPhysicalAddress(uint32_t address)
{
    LOGF_UTILITY("0x{:x}", address);
    return address;
}

uint32_t refii::kernel::MmAllocatePhysicalMemoryEx(uint32_t flags, uint32_t size, uint32_t protect, uint32_t minAddress, uint32_t maxAddress, uint32_t alignment)
{
    LOGF_UTILITY("0x{:x}, 0x{:x}, 0x{:x}, 0x{:x}, 0x{:x}, 0x{:x}", flags, size, protect, minAddress, maxAddress, alignment);
    return g_memory.MapVirtual(g_userHeap.AllocPhysical(size, alignment));
}

DECLARE_STUB_FUNCTION_RETURN(uint32_t, refii::kernel::MmSetAddressProtect, 
    (uint32_t guestAddress,uint32_t size,uint32_t protect), 0)


uint32_t refii::kernel::MmQueryAddressProtect(uint32_t guestAddress)
{
    return PAGE_READWRITE;
}

void refii::kernel::MmQueryAllocationSize()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t refii::kernel::ExAllocatePool(uint32_t size)
{
    void* ptr = g_userHeap.Alloc(size);
    return g_memory.MapVirtual(ptr);
}

void refii::kernel::ExAllocatePoolTypeWithTag()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t refii::kernel::XamAlloc(uint32_t size)
{
    void* ptr = g_userHeap.Alloc(size);
    return g_memory.MapVirtual(ptr);
}

void refii::kernel::XamFree(uint32_t ptr)
{
    if (ptr != NULL)
        g_userHeap.Free(g_memory.Translate(ptr));
}

void* MmGetHostAddress(uint32_t ptr)
{
    return refii::kernel::g_memory.Translate(ptr);
}

// Non-critical
DECLARE_STUB_FUNCTION_VOID(refii::kernel::XeCryptShaInit, ())
DECLARE_STUB_FUNCTION_VOID(refii::kernel::XeCryptShaUpdate, ())
DECLARE_STUB_FUNCTION_VOID(refii::kernel::XeCryptShaFinal, ())
DECLARE_STUB_FUNCTION_RETURN(uint32_t, refii::kernel::XexLoadImage, (), 0)
DECLARE_STUB_FUNCTION_RETURN(uint32_t, refii::kernel::XexUnloadImage, (), 0)
DECLARE_STUB_FUNCTION_RETURN(uint32_t, refii::kernel::NtQueueApcThread, (), 0)
DECLARE_STUB_FUNCTION_RETURN(uint32_t, refii::kernel::NtProtectVirtualMemory, (), 0)
DECLARE_STUB_FUNCTION_RETURN(uint32_t, refii::kernel::XamShowMarketplaceUI, (), 0)
DECLARE_STUB_FUNCTION_RETURN(uint32_t, refii::kernel::XamUserCheckPrivilege, (), 0)
DECLARE_STUB_FUNCTION_RETURN(uint32_t, refii::kernel::XamUserAreUsersFriends, (), 0) // :(
DECLARE_STUB_FUNCTION_RETURN(uint32_t, refii::kernel::XamReadTileToTexture, (), 0)
DECLARE_STUB_FUNCTION_RETURN(uint32_t, refii::kernel::XamShowGamerCardUIForXUID, (), 0)
DECLARE_STUB_FUNCTION_RETURN(uint32_t, refii::kernel::XamContentGetLicenseMask, (), 0)
DECLARE_STUB_FUNCTION_RETURN(uint32_t, refii::kernel::XamParseGamerTileKey, (), 0)
DECLARE_STUB_FUNCTION_VOID(refii::kernel::NtCancelTimer, ())
DECLARE_STUB_FUNCTION_VOID(refii::kernel::NtCreateTimer, ())
DECLARE_STUB_FUNCTION_VOID(refii::kernel::NtSetTimerEx, ())
