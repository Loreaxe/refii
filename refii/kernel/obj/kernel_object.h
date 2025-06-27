#pragma once
namespace refii {
namespace kernel {
    struct KernelObject
    {
        virtual ~KernelObject()
        {
        }

        virtual uint32_t Wait(uint32_t timeout)
        {
            assert(false && "Wait not implemented for this kernel object.");
            return STATUS_TIMEOUT;
        }
    };
}
}
