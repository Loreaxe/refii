#include <stdafx.h>
#include "hal.h"

void refii::kernel::VdHSIOCalibrationLock()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::XboxHardwareInfo()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::XGetVideoMode()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t refii::kernel::FscSetCacheElementCount()
{
    return 0;
}

void refii::kernel::HalReturnToFirmware()
{
    LOG_UTILITY("!!! STUB !!!");
}

bool refii::kernel::VdPersistDisplay(uint32_t a1, uint32_t* a2)
{
    *a2 = NULL;
    return false;
}

void refii::kernel::VdSwap()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::VdGetSystemCommandBuffer()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::VdEnableRingBufferRPtrWriteBack()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::VdInitializeRingBuffer()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::VdSetSystemCommandBufferGpuIdentifierAddress()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::VdShutdownEngines()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::VdQueryVideoMode(XVIDEO_MODE* vm)
{
    memset(vm, 0, sizeof(XVIDEO_MODE));
    vm->DisplayWidth = 1280;
    vm->DisplayHeight = 720;
    vm->IsInterlaced = false;
    vm->IsWidescreen = true;
    vm->IsHighDefinition = true;
    vm->RefreshRate = 0x42700000;
    vm->VideoStandard = 1;
    vm->Unknown4A = 0x4A;
    vm->Unknown01 = 0x01;
}

void refii::kernel::VdGetCurrentDisplayInformation()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::VdSetDisplayMode()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::VdSetGraphicsInterruptCallback()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::VdInitializeEngines()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::VdIsHSIOTrainingSucceeded()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::VdGetCurrentDisplayGamma()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::VdQueryVideoFlags()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::VdCallGraphicsNotificationRoutines()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::VdInitializeScalerCommandBuffer()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t refii::kernel::VdRetrainEDRAM()
{
    return 0;
}

void refii::kernel::VdRetrainEDRAMWorker()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::StfsControlDevice()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::StfsCreateDevice()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::IoInvalidDeviceRequest()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::IoCheckShareAccess()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::IoSetShareAccess()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::IoRemoveShareAccess()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::IoDismountVolumeByFileHandle()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::IoDismountVolumeByName()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::IoSynchronousDeviceIoControlRequest()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::XMAReleaseContext()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::XMACreateContext()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::IoCreateDevice()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::IoDeleteDevice()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::IoCompleteRequest()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::VdEnableDisableClockGating()
{
    LOG_UTILITY("!!! STUB !!!");
}
