#include <stdafx.h>
#include <kernel/kernel.h>

#ifdef REFII_HEAP_EMU
uint32_t refii::kernel::RtlCreateHeap(uint32_t flags, uint32_t heapBase, uint32_t reserveSize,
    uint32_t commitSize, uint32_t lock, void* heapParams)
{
    LOGF_UTILITY("flags=0x{:x}, heapBase=0x{:x}, reserveSize=0x{:x}, commitSize=0x{:x}",
        flags, heapBase, reserveSize, commitSize);

    // Xbox 360 typically ignores heapBase and lock parameters

    // Create a heap tracking object
    HeapObject* heapObj = CreateKernelObject<HeapObject>(flags, commitSize, reserveSize);
    if (!heapObj)
    {
        LOG_UTILITY("Failed to create heap object");
        return 0;
    }

    uint32_t heapHandle = GetKernelHandle(heapObj);
    LOGF_UTILITY("Created heap with handle 0x{:x}", heapHandle);

    return heapHandle;
}

uint32_t refii::kernel::RtlDestroyHeap(uint32_t heapHandle)
{
    LOGF_UTILITY("heapHandle=0x{:x}", heapHandle);
    if (heapHandle == DEFAULT_PROCESS_HEAP || heapHandle == 0)
    {
        LOG_ERROR("Cannot destroy default process heap");
        return heapHandle;
    }

    if (IsKernelObject(heapHandle))
    {
        DestroyKernelObject(heapHandle);
        return 0; // Success
    }

    return heapHandle; // Failure
}

uint32_t refii::kernel::RtlAllocateHeap(uint32_t heapHandle, uint32_t flags, uint32_t size)
{
    void* ptr = nullptr;

    // Default process heap - use g_userHeap directly
    if (heapHandle == 0 || heapHandle == DEFAULT_PROCESS_HEAP)
    {
        ptr = g_userHeap.Alloc(size);
        if (ptr && (flags & HEAP_ZERO_MEMORY))
            memset(ptr, 0, size);
    }
    // Custom heap - use heap object
    else if (IsKernelObject(heapHandle))
    {
        HeapObject* heap = GetKernelObject<HeapObject>(heapHandle);
        ptr = heap->Allocate(size, flags);
    }
    else
    {
        LOG_ERROR("Invalid heap handle");
        return 0;
    }

    if (!ptr)
    {
        LOG_ERROR("Allocation failed");
        return 0;
    }

    return g_memory.MapVirtual(ptr);
}

uint32_t refii::kernel::RtlReAllocateHeap(uint32_t heapHandle, uint32_t flags, uint32_t memoryPointer, uint32_t size)
{
    LOGF_UTILITY("heapHandle=0x{:x}, flags=0x{:x}, memoryPointer=0x{:x}, size=0x{:x}", heapHandle, flags, memoryPointer, size);
    void* oldPtr = memoryPointer ? g_memory.Translate(memoryPointer) : nullptr;
    size_t oldSize = oldPtr ? g_userHeap.Size(oldPtr) : 0;

    // Allocate new memory
    uint32_t newPtr = RtlAllocateHeap(heapHandle, flags & ~HEAP_ZERO_MEMORY, size);
    if (!newPtr)
        return 0;

    // Copy old data if needed
    if (oldPtr)
    {
        void* newPtrHost = g_memory.Translate(newPtr);
        memcpy(newPtrHost, oldPtr, std::min(oldSize, (size_t)size));

        if (size > oldSize && (flags & HEAP_ZERO_MEMORY))
        {
            memset((uint8_t*)newPtrHost + oldSize, 0, size - oldSize);
        }

        RtlFreeHeap(heapHandle, 0, memoryPointer);
    }

    return newPtr;
}

uint32_t refii::kernel::RtlFreeHeap(uint32_t heapHandle, uint32_t flags, uint32_t memoryPointer)
{
    LOGF_UTILITY("heapHandle=0x{:x}, flags=0x{:x}, memoryPointer=0x{:x}", heapHandle, flags, memoryPointer);
    if (memoryPointer == 0)
        return TRUE;

    void* ptr = g_memory.Translate(memoryPointer);

    // Default process heap
    if (heapHandle == 0 || heapHandle == DEFAULT_PROCESS_HEAP)
    {
        g_userHeap.Free(ptr);
        return TRUE;
    }
    // Custom heap
    else if (IsKernelObject(heapHandle))
    {
        HeapObject* heap = GetKernelObject<HeapObject>(heapHandle);
        return heap->Free(ptr) ? TRUE : FALSE;
    }

    LOG_ERROR("Invalid heap handle");
    return FALSE;
}

uint32_t refii::kernel::RtlSizeHeap(uint32_t heapHandle, uint32_t flags, uint32_t memoryPointer)
{
    if (memoryPointer == 0)
        return 0;

    // All allocations ultimately come from g_userHeap, so we can use it for size
    return (uint32_t)g_userHeap.Size(g_memory.Translate(memoryPointer));
}
#else
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
#endif // REFII_HEAP_EMU
