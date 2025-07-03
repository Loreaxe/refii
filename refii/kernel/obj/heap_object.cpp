#include "heap_object.h"

refii::kernel::HeapObject::HeapObject(uint32_t flags, uint32_t initialSize, uint32_t maximumSize)
    : flags(flags), initialSize(initialSize), maximumSize(maximumSize)
{
}

refii::kernel::HeapObject::~HeapObject()
{
    // Free all remaining allocations from this heap
    for (void* ptr : allocations)
    {
        g_userHeap.Free(ptr);
    }
}

void* refii::kernel::HeapObject::Allocate(size_t size, uint32_t allocFlags)
{
    // Check size limits
    if (maximumSize != 0)
    {
        size_t currentSize = 0;
        for (void* ptr : allocations)
        {
            currentSize += g_userHeap.Size(ptr);
        }

        if (currentSize + size > maximumSize)
        {
            return nullptr;
        }
    }

    // Handle serialization if needed
    std::unique_lock<Mutex> lock;
    if (!(flags & HEAP_NO_SERIALIZE))
    {
        lock = std::unique_lock<Mutex>(heapLock);
    }

    // Allocate from the global heap
    void* ptr = g_userHeap.Alloc(size);
    if (!ptr)
        return nullptr;

    // Track this allocation
    allocations.insert(ptr);

    // Handle zero memory flag
    if ((allocFlags & HEAP_ZERO_MEMORY) || (flags & HEAP_ZERO_MEMORY))
    {
        memset(ptr, 0, size);
    }

    return ptr;
}

bool refii::kernel::HeapObject::Free(void* ptr)
{
    if (!ptr)
        return true;

    std::unique_lock<Mutex> lock;
    if (!(flags & HEAP_NO_SERIALIZE))
    {
        lock = std::unique_lock<Mutex>(heapLock);
    }

    // Check if this allocation belongs to this heap
    if (allocations.find(ptr) == allocations.end())
    {
        return false;
    }

    allocations.erase(ptr);
    g_userHeap.Free(ptr);
    return true;
}

bool refii::kernel::HeapObject::Owns(void* ptr)
{
    std::unique_lock<Mutex> lock;
    if (!(flags & HEAP_NO_SERIALIZE))
    {
        lock = std::unique_lock<Mutex>(heapLock);
    }

    return allocations.find(ptr) != allocations.end();
}
