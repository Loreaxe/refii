#pragma once
#include <kernel/kernel.h>
#include <kernel/obj/guest_heap.h>


#define HEAP_NO_SERIALIZE           0x00000001
#define HEAP_GROWABLE               0x00000002
#define HEAP_GENERATE_EXCEPTIONS    0x00000004
#define HEAP_ZERO_MEMORY            0x00000008
#define HEAP_CREATE_ENABLE_EXECUTE  0x00040000

using namespace refii::kernel;

namespace refii::kernel {
    struct HeapObject : KernelObject
    {
        uint32_t flags;
        uint32_t initialSize;
        uint32_t maximumSize;
        Mutex heapLock;

        // Track allocations made from this heap
        ankerl::unordered_dense::set<void*> allocations;

        HeapObject(uint32_t flags, uint32_t initialSize, uint32_t maximumSize);
        ~HeapObject();

        void* Allocate(size_t size, uint32_t allocFlags);
        bool Free(void* ptr);
        bool Owns(void* ptr);
    };
} // namespace refii::kernel
