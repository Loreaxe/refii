#include <stdafx.h>
#include <kernel/function.h>
#include <kernel/kernel.h>
#include <kernel/xam.h>
#include <apu/audio.h>
#include <gpu/video.h>
#include <os/logger.h>

GUEST_FUNCTION_HOOK(__imp__XGetVideoMode, refii::kernel::VdQueryVideoMode); // XGetVideoMode
GUEST_FUNCTION_HOOK(__imp__XNotifyGetNext, refii::kernel::XNotifyGetNext);
GUEST_FUNCTION_HOOK(__imp__XGetGameRegion, refii::kernel::XGetGameRegion);
GUEST_FUNCTION_HOOK(__imp__XMsgStartIORequest, refii::kernel::XMsgStartIORequest);
GUEST_FUNCTION_HOOK(__imp__XamUserGetSigninState, refii::kernel::XamUserGetSigninState);
GUEST_FUNCTION_HOOK(__imp__XamGetSystemVersion, refii::kernel::XamGetSystemVersion);
GUEST_FUNCTION_HOOK(__imp__XamContentCreateEx, refii::kernel::XamContentCreateEx);
GUEST_FUNCTION_HOOK(__imp__XamContentDelete, refii::kernel::XamContentDelete);
GUEST_FUNCTION_HOOK(__imp__XamContentClose, refii::kernel::XamContentClose);
GUEST_FUNCTION_HOOK(__imp__XamContentGetCreator, refii::kernel::XamContentGetCreator);
GUEST_FUNCTION_HOOK(__imp__XamContentCreateEnumerator, refii::kernel::XamContentCreateEnumerator);
GUEST_FUNCTION_HOOK(__imp__XamContentGetDeviceState, refii::kernel::XamContentGetDeviceState);
GUEST_FUNCTION_HOOK(__imp__XamContentGetDeviceData, refii::kernel::XamContentGetDeviceData);
GUEST_FUNCTION_HOOK(__imp__XamEnumerate, refii::kernel::XamEnumerate);
GUEST_FUNCTION_HOOK(__imp__XamNotifyCreateListener, refii::kernel::XamNotifyCreateListener);
GUEST_FUNCTION_HOOK(__imp__XamUserGetSigninInfo, refii::kernel::XamUserGetSigninInfo);
GUEST_FUNCTION_HOOK(__imp__XamShowSigninUI, refii::kernel::XamShowSigninUI);
GUEST_FUNCTION_HOOK(__imp__XamShowDeviceSelectorUI, refii::kernel::XamShowDeviceSelectorUI);
GUEST_FUNCTION_HOOK(__imp__XamShowMessageBoxUI, refii::kernel::XamShowMessageBoxUI);
GUEST_FUNCTION_HOOK(__imp__XamShowDirtyDiscErrorUI, refii::kernel::XamShowDirtyDiscErrorUI);
GUEST_FUNCTION_HOOK(__imp__XamEnableInactivityProcessing, refii::kernel::XamEnableInactivityProcessing);
GUEST_FUNCTION_HOOK(__imp__XamResetInactivity, refii::kernel::XamResetInactivity);
GUEST_FUNCTION_HOOK(__imp__XamShowMessageBoxUIEx, refii::kernel::XamShowMessageBoxUIEx);
GUEST_FUNCTION_HOOK(__imp__XGetLanguage, refii::kernel::XGetLanguage);
GUEST_FUNCTION_HOOK(__imp__XGetAVPack, refii::kernel::XGetAVPack);
GUEST_FUNCTION_HOOK(__imp__XamLoaderTerminateTitle, refii::kernel::XamLoaderTerminateTitle);
GUEST_FUNCTION_HOOK(__imp__XamGetExecutionId, refii::kernel::XamGetExecutionId);
GUEST_FUNCTION_HOOK(__imp__XamLoaderLaunchTitle, refii::kernel::XamLoaderLaunchTitle);
GUEST_FUNCTION_HOOK(__imp__NtOpenFile, refii::kernel::NtOpenFile);
GUEST_FUNCTION_HOOK(__imp__RtlInitAnsiString, refii::kernel::RtlInitAnsiString);
GUEST_FUNCTION_HOOK(__imp__NtCreateFile, refii::kernel::NtCreateFile);
GUEST_FUNCTION_HOOK(__imp__NtClose, refii::kernel::NtClose);
GUEST_FUNCTION_HOOK(__imp__NtSetInformationFile, refii::kernel::NtSetInformationFile);
GUEST_FUNCTION_HOOK(__imp__FscSetCacheElementCount, refii::kernel::FscSetCacheElementCount);
GUEST_FUNCTION_HOOK(__imp__NtWaitForSingleObjectEx, refii::kernel::NtWaitForSingleObjectEx);
GUEST_FUNCTION_HOOK(__imp__NtWriteFile, refii::kernel::NtWriteFile);
GUEST_FUNCTION_HOOK(__imp__ExGetXConfigSetting, refii::kernel::ExGetXConfigSetting);
GUEST_FUNCTION_HOOK(__imp__NtQueryVirtualMemory, refii::kernel::NtQueryVirtualMemory);
GUEST_FUNCTION_HOOK(__imp__MmQueryStatistics, refii::kernel::MmQueryStatistics);
GUEST_FUNCTION_HOOK(__imp__NtCreateEvent, refii::kernel::NtCreateEvent);
GUEST_FUNCTION_HOOK(__imp__XexCheckExecutablePrivilege, refii::kernel::XexCheckExecutablePrivilege);
GUEST_FUNCTION_HOOK(__imp__DbgPrint, refii::kernel::DbgPrint);
GUEST_FUNCTION_HOOK(__imp____C_specific_handler, refii::kernel::__C_specific_handler_x);
GUEST_FUNCTION_HOOK(__imp__RtlNtStatusToDosError, refii::kernel::RtlNtStatusToDosError);
GUEST_FUNCTION_HOOK(__imp__XexGetProcedureAddress, refii::kernel::XexGetProcedureAddress);
GUEST_FUNCTION_HOOK(__imp__XexGetModuleSection, refii::kernel::XexGetModuleSection);
GUEST_FUNCTION_HOOK(__imp__RtlUnicodeToMultiByteN, refii::kernel::RtlUnicodeToMultiByteN);
GUEST_FUNCTION_HOOK(__imp__KeDelayExecutionThread, refii::kernel::KeDelayExecutionThread);
GUEST_FUNCTION_HOOK(__imp__ExFreePool, refii::kernel::ExFreePool);
GUEST_FUNCTION_HOOK(__imp__NtQueryInformationFile, refii::kernel::NtQueryInformationFile);
GUEST_FUNCTION_HOOK(__imp__NtQueryVolumeInformationFile, refii::kernel::NtQueryVolumeInformationFile);
GUEST_FUNCTION_HOOK(__imp__NtQueryDirectoryFile, refii::kernel::NtQueryDirectoryFile);
GUEST_FUNCTION_HOOK(__imp__NtReadFileScatter, refii::kernel::NtReadFileScatter);
GUEST_FUNCTION_HOOK(__imp__NtReadFile, refii::kernel::NtReadFile);
GUEST_FUNCTION_HOOK(__imp__NtDuplicateObject, refii::kernel::NtDuplicateObject);
GUEST_FUNCTION_HOOK(__imp__NtAllocateVirtualMemory, refii::kernel::NtAllocateVirtualMemory);
GUEST_FUNCTION_HOOK(__imp__NtFreeVirtualMemory, refii::kernel::NtFreeVirtualMemory);
GUEST_FUNCTION_HOOK(__imp__ObDereferenceObject, refii::kernel::ObDereferenceObject);
GUEST_FUNCTION_HOOK(__imp__KeSetBasePriorityThread, refii::kernel::KeSetBasePriorityThread);
GUEST_FUNCTION_HOOK(__imp__ObReferenceObjectByHandle, refii::kernel::ObReferenceObjectByHandle);
GUEST_FUNCTION_HOOK(__imp__KeQueryBasePriorityThread, refii::kernel::KeQueryBasePriorityThread);
GUEST_FUNCTION_HOOK(__imp__NtSuspendThread, refii::kernel::NtSuspendThread);
GUEST_FUNCTION_HOOK(__imp__KeSetAffinityThread, refii::kernel::KeSetAffinityThread);
GUEST_FUNCTION_HOOK(__imp__RtlLeaveCriticalSection, refii::kernel::RtlLeaveCriticalSection);
GUEST_FUNCTION_HOOK(__imp__RtlEnterCriticalSection, refii::kernel::RtlEnterCriticalSection);
GUEST_FUNCTION_HOOK(__imp__RtlImageXexHeaderField, refii::kernel::RtlImageXexHeaderField);
GUEST_FUNCTION_HOOK(__imp__HalReturnToFirmware, refii::kernel::HalReturnToFirmware);
GUEST_FUNCTION_HOOK(__imp__RtlFillMemoryUlong, refii::kernel::RtlFillMemoryUlong);
GUEST_FUNCTION_HOOK(__imp__KeBugCheckEx, refii::kernel::KeBugCheckEx);
GUEST_FUNCTION_HOOK(__imp__KeGetCurrentProcessType, refii::kernel::KeGetCurrentProcessType);
GUEST_FUNCTION_HOOK(__imp__RtlCompareMemoryUlong, refii::kernel::RtlCompareMemoryUlong);
GUEST_FUNCTION_HOOK(__imp__RtlInitializeCriticalSection, refii::kernel::RtlInitializeCriticalSection);
GUEST_FUNCTION_HOOK(__imp__RtlRaiseException, refii::kernel::RtlRaiseException_x);
GUEST_FUNCTION_HOOK(__imp__KfReleaseSpinLock, refii::kernel::KfReleaseSpinLock);
GUEST_FUNCTION_HOOK(__imp__KfAcquireSpinLock, refii::kernel::KfAcquireSpinLock);
GUEST_FUNCTION_HOOK(__imp__KeQueryPerformanceFrequency, refii::kernel::KeQueryPerformanceFrequency);
GUEST_FUNCTION_HOOK(__imp__MmFreePhysicalMemory, refii::kernel::MmFreePhysicalMemory);
GUEST_FUNCTION_HOOK(__imp__VdPersistDisplay, refii::kernel::VdPersistDisplay);
GUEST_FUNCTION_HOOK(__imp__VdSwap, refii::kernel::VdSwap);
GUEST_FUNCTION_HOOK(__imp__VdGetSystemCommandBuffer, refii::kernel::VdGetSystemCommandBuffer);
GUEST_FUNCTION_HOOK(__imp__KeReleaseSpinLockFromRaisedIrql, refii::kernel::KeReleaseSpinLockFromRaisedIrql);
GUEST_FUNCTION_HOOK(__imp__KeAcquireSpinLockAtRaisedIrql, refii::kernel::KeAcquireSpinLockAtRaisedIrql);
GUEST_FUNCTION_HOOK(__imp__KiApcNormalRoutineNop, refii::kernel::KiApcNormalRoutineNop);
GUEST_FUNCTION_HOOK(__imp__VdEnableRingBufferRPtrWriteBack, refii::kernel::VdEnableRingBufferRPtrWriteBack);
GUEST_FUNCTION_HOOK(__imp__VdInitializeRingBuffer, refii::kernel::VdInitializeRingBuffer);
GUEST_FUNCTION_HOOK(__imp__MmGetPhysicalAddress, refii::kernel::MmGetPhysicalAddress);
GUEST_FUNCTION_HOOK(__imp__VdSetSystemCommandBufferGpuIdentifierAddress, refii::kernel::VdSetSystemCommandBufferGpuIdentifierAddress);
GUEST_FUNCTION_HOOK(__imp__ExRegisterTitleTerminateNotification, refii::kernel::ExRegisterTitleTerminateNotification);
GUEST_FUNCTION_HOOK(__imp__VdShutdownEngines, refii::kernel::VdShutdownEngines);
GUEST_FUNCTION_HOOK(__imp__VdQueryVideoMode, refii::kernel::VdQueryVideoMode);
GUEST_FUNCTION_HOOK(__imp__VdGetCurrentDisplayInformation, refii::kernel::VdGetCurrentDisplayInformation);
GUEST_FUNCTION_HOOK(__imp__VdSetDisplayMode, refii::kernel::VdSetDisplayMode);
GUEST_FUNCTION_HOOK(__imp__VdSetGraphicsInterruptCallback, refii::kernel::VdSetGraphicsInterruptCallback);
GUEST_FUNCTION_HOOK(__imp__VdInitializeEngines, refii::kernel::VdInitializeEngines);
GUEST_FUNCTION_HOOK(__imp__VdIsHSIOTrainingSucceeded, refii::kernel::VdIsHSIOTrainingSucceeded);
GUEST_FUNCTION_HOOK(__imp__VdGetCurrentDisplayGamma, refii::kernel::VdGetCurrentDisplayGamma);
GUEST_FUNCTION_HOOK(__imp__VdQueryVideoFlags, refii::kernel::VdQueryVideoFlags);
GUEST_FUNCTION_HOOK(__imp__VdCallGraphicsNotificationRoutines, refii::kernel::VdCallGraphicsNotificationRoutines);
GUEST_FUNCTION_HOOK(__imp__VdInitializeScalerCommandBuffer, refii::kernel::VdInitializeScalerCommandBuffer);
GUEST_FUNCTION_HOOK(__imp__KeLeaveCriticalRegion, refii::kernel::KeLeaveCriticalRegion);
GUEST_FUNCTION_HOOK(__imp__VdRetrainEDRAM, refii::kernel::VdRetrainEDRAM);
GUEST_FUNCTION_HOOK(__imp__VdRetrainEDRAMWorker, refii::kernel::VdRetrainEDRAMWorker);
GUEST_FUNCTION_HOOK(__imp__KeEnterCriticalRegion, refii::kernel::KeEnterCriticalRegion);
GUEST_FUNCTION_HOOK(__imp__MmAllocatePhysicalMemoryEx, refii::kernel::MmAllocatePhysicalMemoryEx);
GUEST_FUNCTION_HOOK(__imp__ObDeleteSymbolicLink, refii::kernel::ObDeleteSymbolicLink);
GUEST_FUNCTION_HOOK(__imp__ObCreateSymbolicLink, refii::kernel::ObCreateSymbolicLink);
GUEST_FUNCTION_HOOK(__imp__MmQueryAddressProtect, refii::kernel::MmQueryAddressProtect);
GUEST_FUNCTION_HOOK(__imp__VdEnableDisableClockGating, refii::kernel::VdEnableDisableClockGating);
GUEST_FUNCTION_HOOK(__imp__KeBugCheck, refii::kernel::KeBugCheck);
GUEST_FUNCTION_HOOK(__imp__KeLockL2, refii::kernel::KeLockL2);
GUEST_FUNCTION_HOOK(__imp__KeUnlockL2, refii::kernel::KeUnlockL2);
GUEST_FUNCTION_HOOK(__imp__KeSetEvent, refii::kernel::KeSetEvent);
GUEST_FUNCTION_HOOK(__imp__KeResetEvent, refii::kernel::KeResetEvent);
GUEST_FUNCTION_HOOK(__imp__KeWaitForSingleObject, refii::kernel::KeWaitForSingleObject);
GUEST_FUNCTION_HOOK(__imp__KeTlsGetValue, refii::kernel::KeTlsGetValue);
GUEST_FUNCTION_HOOK(__imp__KeTlsSetValue, refii::kernel::KeTlsSetValue);
GUEST_FUNCTION_HOOK(__imp__KeTlsAlloc, refii::kernel::KeTlsAlloc);
GUEST_FUNCTION_HOOK(__imp__KeTlsFree, refii::kernel::KeTlsFree);
GUEST_FUNCTION_HOOK(__imp__XMsgInProcessCall, refii::kernel::XMsgInProcessCall);
GUEST_FUNCTION_HOOK(__imp__XamUserReadProfileSettings, refii::kernel::XamUserReadProfileSettings);
GUEST_FUNCTION_HOOK(__imp__NetDll_WSAStartup, refii::kernel::NetDll_WSAStartup);
GUEST_FUNCTION_HOOK(__imp__NetDll_WSACleanup, refii::kernel::NetDll_WSACleanup);
GUEST_FUNCTION_HOOK(__imp__NetDll_socket, refii::kernel::NetDll_socket);
GUEST_FUNCTION_HOOK(__imp__NetDll_closesocket, refii::kernel::NetDll_closesocket);
GUEST_FUNCTION_HOOK(__imp__NetDll_setsockopt, refii::kernel::NetDll_setsockopt);
GUEST_FUNCTION_HOOK(__imp__NetDll_bind, refii::kernel::NetDll_bind);
GUEST_FUNCTION_HOOK(__imp__NetDll_connect, refii::kernel::NetDll_connect);
GUEST_FUNCTION_HOOK(__imp__NetDll_listen, refii::kernel::NetDll_listen);
GUEST_FUNCTION_HOOK(__imp__NetDll_accept, refii::kernel::NetDll_accept);
GUEST_FUNCTION_HOOK(__imp__NetDll_select, refii::kernel::NetDll_select);
GUEST_FUNCTION_HOOK(__imp__NetDll_recv, refii::kernel::NetDll_recv);
GUEST_FUNCTION_HOOK(__imp__NetDll_send, refii::kernel::NetDll_send);
GUEST_FUNCTION_HOOK(__imp__NetDll_inet_addr, refii::kernel::NetDll_inet_addr);
GUEST_FUNCTION_HOOK(__imp__NetDll___WSAFDIsSet, refii::kernel::NetDll___WSAFDIsSet);
GUEST_FUNCTION_HOOK(__imp__XMsgStartIORequestEx, refii::kernel::XMsgStartIORequestEx);
GUEST_FUNCTION_HOOK(__imp__XamInputGetCapabilities, refii::kernel::XamInputGetCapabilities);
GUEST_FUNCTION_HOOK(__imp__XamInputGetState, refii::kernel::XamInputGetState);
GUEST_FUNCTION_HOOK(__imp__XamInputSetState, refii::kernel::XamInputSetState);
GUEST_FUNCTION_HOOK(__imp__XexGetModuleHandle, refii::kernel::XexGetModuleHandle);
GUEST_FUNCTION_HOOK(__imp__RtlTryEnterCriticalSection, refii::kernel::RtlTryEnterCriticalSection);
GUEST_FUNCTION_HOOK(__imp__RtlInitializeCriticalSectionAndSpinCount, refii::kernel::RtlInitializeCriticalSectionAndSpinCount);
GUEST_FUNCTION_HOOK(__imp__XeCryptBnQwBeSigVerify, refii::kernel::XeCryptBnQwBeSigVerify);
GUEST_FUNCTION_HOOK(__imp__XeKeysGetKey, refii::kernel::XeKeysGetKey);
GUEST_FUNCTION_HOOK(__imp__XeCryptRotSumSha, refii::kernel::XeCryptRotSumSha);
GUEST_FUNCTION_HOOK(__imp__XeCryptSha, refii::kernel::XeCryptSha);
GUEST_FUNCTION_HOOK(__imp__KeEnableFpuExceptions, refii::kernel::KeEnableFpuExceptions);
GUEST_FUNCTION_HOOK(__imp__RtlUnwind, refii::kernel::RtlUnwind_x);
GUEST_FUNCTION_HOOK(__imp__RtlCaptureContext, refii::kernel::RtlCaptureContext_x);
GUEST_FUNCTION_HOOK(__imp__NtQueryFullAttributesFile, refii::kernel::NtQueryFullAttributesFile);
GUEST_FUNCTION_HOOK(__imp__RtlMultiByteToUnicodeN, refii::kernel::RtlMultiByteToUnicodeN);
GUEST_FUNCTION_HOOK(__imp__DbgBreakPoint, refii::kernel::DbgBreakPoint);
GUEST_FUNCTION_HOOK(__imp__MmQueryAllocationSize, refii::kernel::MmQueryAllocationSize);
GUEST_FUNCTION_HOOK(__imp__NtClearEvent, refii::kernel::NtClearEvent);
GUEST_FUNCTION_HOOK(__imp__NtResumeThread, refii::kernel::NtResumeThread);
GUEST_FUNCTION_HOOK(__imp__NtSetEvent, refii::kernel::NtSetEvent);
GUEST_FUNCTION_HOOK(__imp__NtCreateSemaphore, refii::kernel::NtCreateSemaphore);
GUEST_FUNCTION_HOOK(__imp__NtReleaseSemaphore, refii::kernel::NtReleaseSemaphore);
GUEST_FUNCTION_HOOK(__imp__NtWaitForMultipleObjectsEx, refii::kernel::NtWaitForMultipleObjectsEx);
GUEST_FUNCTION_HOOK(__imp__RtlCompareStringN, refii::kernel::RtlCompareStringN);
GUEST_FUNCTION_HOOK(__imp__StfsControlDevice, refii::kernel::StfsControlDevice);
GUEST_FUNCTION_HOOK(__imp__StfsCreateDevice, refii::kernel::StfsCreateDevice);
GUEST_FUNCTION_HOOK(__imp__NtFlushBuffersFile, refii::kernel::NtFlushBuffersFile);
GUEST_FUNCTION_HOOK(__imp__KeQuerySystemTime, refii::kernel::KeQuerySystemTime);
GUEST_FUNCTION_HOOK(__imp__RtlTimeToTimeFields, refii::kernel::RtlTimeToTimeFields);
GUEST_FUNCTION_HOOK(__imp__RtlFreeAnsiString, refii::kernel::RtlFreeAnsiString);
GUEST_FUNCTION_HOOK(__imp__RtlUnicodeStringToAnsiString, refii::kernel::RtlUnicodeStringToAnsiString);
GUEST_FUNCTION_HOOK(__imp__RtlInitUnicodeString, refii::kernel::RtlInitUnicodeString);
GUEST_FUNCTION_HOOK(__imp__ExTerminateThread, refii::kernel::ExTerminateThread);
GUEST_FUNCTION_HOOK(__imp__ExCreateThread, refii::kernel::ExCreateThread);
GUEST_FUNCTION_HOOK(__imp__IoInvalidDeviceRequest, refii::kernel::IoInvalidDeviceRequest);
GUEST_FUNCTION_HOOK(__imp__ObReferenceObject, refii::kernel::ObReferenceObject);
GUEST_FUNCTION_HOOK(__imp__IoCreateDevice, refii::kernel::IoCreateDevice);
GUEST_FUNCTION_HOOK(__imp__IoDeleteDevice, refii::kernel::IoDeleteDevice);
GUEST_FUNCTION_HOOK(__imp__ExAllocatePoolTypeWithTag, refii::kernel::ExAllocatePoolTypeWithTag);
GUEST_FUNCTION_HOOK(__imp__RtlTimeFieldsToTime, refii::kernel::RtlTimeFieldsToTime);
GUEST_FUNCTION_HOOK(__imp__IoCompleteRequest, refii::kernel::IoCompleteRequest);
GUEST_FUNCTION_HOOK(__imp__RtlUpcaseUnicodeChar, refii::kernel::RtlUpcaseUnicodeChar);
GUEST_FUNCTION_HOOK(__imp__ObIsTitleObject, refii::kernel::ObIsTitleObject);
GUEST_FUNCTION_HOOK(__imp__IoCheckShareAccess, refii::kernel::IoCheckShareAccess);
GUEST_FUNCTION_HOOK(__imp__IoSetShareAccess, refii::kernel::IoSetShareAccess);
GUEST_FUNCTION_HOOK(__imp__IoRemoveShareAccess, refii::kernel::IoRemoveShareAccess);
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetStartup, refii::kernel::NetDll_XNetStartup);
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetGetTitleXnAddr, refii::kernel::NetDll_XNetGetTitleXnAddr);
GUEST_FUNCTION_HOOK(__imp__KeWaitForMultipleObjects, refii::kernel::KeWaitForMultipleObjects);
GUEST_FUNCTION_HOOK(__imp__KeRaiseIrqlToDpcLevel, refii::kernel::RaiseIrqlToDpcLevel);
GUEST_FUNCTION_HOOK(__imp__KfLowerIrql, refii::kernel::KfLowerIrql);
GUEST_FUNCTION_HOOK(__imp__KeReleaseSemaphore, refii::kernel::KeReleaseSemaphore);
GUEST_FUNCTION_HOOK(__imp__XAudioGetVoiceCategoryVolume, refii::kernel::XAudioGetVoiceCategoryVolume);
GUEST_FUNCTION_HOOK(__imp__XAudioGetVoiceCategoryVolumeChangeMask, refii::kernel::XAudioGetVoiceCategoryVolumeChangeMask);
GUEST_FUNCTION_HOOK(__imp__KeResumeThread, refii::kernel::KeResumeThread);
GUEST_FUNCTION_HOOK(__imp__KeInitializeSemaphore, refii::kernel::KeInitializeSemaphore);
GUEST_FUNCTION_HOOK(__imp__XMAReleaseContext, refii::kernel::XMAReleaseContext);
GUEST_FUNCTION_HOOK(__imp__XMACreateContext, refii::kernel::XMACreateContext);
GUEST_FUNCTION_HOOK(__imp__XAudioRegisterRenderDriverClient, XAudioRegisterRenderDriverClient);
GUEST_FUNCTION_HOOK(__imp__XAudioUnregisterRenderDriverClient, XAudioUnregisterRenderDriverClient);
GUEST_FUNCTION_HOOK(__imp__XAudioSubmitRenderDriverFrame, XAudioSubmitRenderDriverFrame);
GUEST_FUNCTION_HOOK(__imp__KeTryToAcquireSpinLockAtRaisedIrql, refii::kernel::KeTryToAcquireSpinLockAtRaisedIrql);
GUEST_FUNCTION_HOOK(__imp__NtCreateMutant, refii::kernel::NtCreateMutant);
GUEST_FUNCTION_HOOK(__imp__NtDeviceIoControlFile, refii::kernel::NtDeviceIoControlFile);
GUEST_FUNCTION_HOOK(__imp__NtReleaseMutant, refii::kernel::NtReleaseMutant);
GUEST_FUNCTION_HOOK(__imp__IoDismountVolumeByFileHandle, refii::kernel::IoDismountVolumeByFileHandle);
GUEST_FUNCTION_HOOK(__imp__XNotifyPositionUI, refii::kernel::XNotifyPositionUI);
GUEST_FUNCTION_HOOK(__imp__ExAllocatePool, refii::kernel::ExAllocatePool);
GUEST_FUNCTION_HOOK(__imp__XamAlloc, refii::kernel::XamAlloc);
GUEST_FUNCTION_HOOK(__imp__XamContentSetThumbnail, refii::kernel::XamContentSetThumbnail);
GUEST_FUNCTION_HOOK(__imp__XamFree, refii::kernel::XamFree);
GUEST_FUNCTION_HOOK(__imp__NtPulseEvent, refii::kernel::NtPulseEvent);
GUEST_FUNCTION_HOOK(__imp__NtYieldExecution, refii::kernel::NtYieldExecution);
GUEST_FUNCTION_HOOK(__imp__XamLoaderGetDvdTrayState, refii::kernel::XamLoaderGetDvdTrayState);
GUEST_FUNCTION_HOOK(__imp__XamTaskCloseHandle, refii::kernel::XamTaskCloseHandle);
GUEST_FUNCTION_HOOK(__imp__XamTaskSchedule, refii::kernel::XamTaskSchedule);
GUEST_FUNCTION_HOOK(__imp__XamTaskShouldExit, refii::kernel::XamTaskShouldExit);
GUEST_FUNCTION_HOOK(__imp__XamUserCreateAchievementEnumerator, refii::kernel::XamUserCreateAchievementEnumerator);
GUEST_FUNCTION_HOOK(__imp__XamUserGetName, refii::kernel::XamUserGetName);
GUEST_FUNCTION_HOOK(__imp__XamUserGetXUID, refii::kernel::XamUserGetXUID);
GUEST_FUNCTION_HOOK(__imp__XamUserWriteProfileSettings, refii::kernel::XamUserWriteProfileSettings);
GUEST_FUNCTION_HOOK(__imp__XeKeysConsolePrivateKeySign, refii::kernel::XeKeysConsolePrivateKeySign);
GUEST_FUNCTION_HOOK(__imp__XeKeysConsoleSignatureVerification, refii::kernel::XeKeysConsoleSignatureVerification);
GUEST_FUNCTION_HOOK(__imp__XexLoadImageHeaders, refii::kernel::XexLoadImageHeaders);
GUEST_FUNCTION_HOOK(__imp__IoDismountVolumeByName, refii::kernel::IoDismountVolumeByName);
GUEST_FUNCTION_HOOK(__imp__IoSynchronousDeviceIoControlRequest, refii::kernel::IoSynchronousDeviceIoControlRequest);
GUEST_FUNCTION_HOOK(__imp__ObOpenObjectByName, refii::kernel::ObOpenObjectByName);
GUEST_FUNCTION_HOOK(__imp__ObReferenceObjectByName, refii::kernel::ObReferenceObjectByName);

GUEST_FUNCTION_HOOK(sub_822387B8, refii::kernel::RtlAllocateHeap);
GUEST_FUNCTION_HOOK(sub_82239490, refii::kernel::RtlFreeHeap);
GUEST_FUNCTION_HOOK(sub_82CC62E0, refii::kernel::RtlReAllocateHeap);
GUEST_FUNCTION_HOOK(sub_82239808, refii::kernel::RtlSizeHeap);
GUEST_FUNCTION_HOOK(sub_822386D0, refii::kernel::XAllocMem);
GUEST_FUNCTION_HOOK(sub_822398B0, refii::kernel::XFreeMem);
GUEST_FUNCTION_HOOK(sub_82CC74F8, refii::kernel::VirtualAlloc);
GUEST_FUNCTION_HOOK(sub_82CC7548, refii::kernel::VirtualFree);
// native memory operations
GUEST_FUNCTION_HOOK(sub_82CB0B30, memmove);
GUEST_FUNCTION_HOOK(sub_826BF770, memcpy);
GUEST_FUNCTION_HOOK(sub_826BFCF0, memset);

GUEST_FUNCTION_HOOK(__imp__MmSetAddressProtect, refii::kernel::MmSetAddressProtect);
GUEST_FUNCTION_HOOK(__imp__NtProtectVirtualMemory, refii::kernel::NtProtectVirtualMemory);
GUEST_FUNCTION_HOOK(__imp__NtQueueApcThread, refii::kernel::NtQueueApcThread);
GUEST_FUNCTION_HOOK(__imp__NtCancelTimer, refii::kernel::NtCancelTimer);
GUEST_FUNCTION_HOOK(__imp__NtCreateTimer, refii::kernel::NtCreateTimer);
GUEST_FUNCTION_HOOK(__imp__NtSetTimerEx, refii::kernel::NtSetTimerEx);
GUEST_FUNCTION_HOOK(__imp__XamContentGetLicenseMask, refii::kernel::XamContentGetLicenseMask);
GUEST_FUNCTION_HOOK(__imp__XamGetOverlappedResult, refii::kernel::XamGetOverlappedResult);
GUEST_FUNCTION_HOOK(__imp__XamGetPrivateEnumStructureFromHandle, refii::kernel::XamGetPrivateEnumStructureFromHandle);
GUEST_FUNCTION_HOOK(__imp__XamParseGamerTileKey, refii::kernel::XamParseGamerTileKey);
GUEST_FUNCTION_HOOK(__imp__XamReadTileToTexture, refii::kernel::XamReadTileToTexture);
GUEST_FUNCTION_HOOK(__imp__XamShowAchievementsUI, refii::kernel::XamShowAchievementsUI);
GUEST_FUNCTION_HOOK(__imp__XamShowGamerCardUIForXUID, refii::kernel::XamShowGamerCardUIForXUID);
GUEST_FUNCTION_HOOK(__imp__XamShowKeyboardUI, refii::kernel::XamShowKeyboardUI);
GUEST_FUNCTION_HOOK(__imp__XamShowMarketplaceUI, refii::kernel::XamShowMarketplaceUI);
GUEST_FUNCTION_HOOK(__imp__XamUserAreUsersFriends, refii::kernel::XamUserAreUsersFriends);
GUEST_FUNCTION_HOOK(__imp__XamUserCheckPrivilege, refii::kernel::XamUserCheckPrivilege);
GUEST_FUNCTION_HOOK(__imp__XeCryptShaInit, refii::kernel::XeCryptShaInit);
GUEST_FUNCTION_HOOK(__imp__XeCryptShaUpdate, refii::kernel::XeCryptShaUpdate);
GUEST_FUNCTION_HOOK(__imp__XeCryptShaFinal, refii::kernel::XeCryptShaFinal);
GUEST_FUNCTION_HOOK(__imp__XexLoadImage, refii::kernel::XexLoadImage);
GUEST_FUNCTION_HOOK(__imp__XexUnloadImage, refii::kernel::XexUnloadImage);
GUEST_FUNCTION_HOOK(__imp__XMsgCancelIORequest, refii::kernel::XMsgCancelIORequest);
GUEST_FUNCTION_HOOK(__imp__XMsgCompleteIORequest, refii::kernel::XMsgCompleteIORequest);
GUEST_FUNCTION_HOOK(__imp__XamCreateEnumeratorHandle, refii::kernel::XamContentCreateEnumerator);
GUEST_FUNCTION_HOOK(__imp__XamSessionRefObjByHandle, refii::kernel::XamSessionRefObjByHandle);
GUEST_FUNCTION_HOOK(__imp__XamSessionCreateHandle, refii::kernel::XamSessionCreateHandle);
GUEST_FUNCTION_HOOK(__imp__XamUserGetMembershipTierFromXUID, refii::kernel::XamUserGetMembershipTierFromXUID);
GUEST_FUNCTION_HOOK(__imp__XamUserGetOnlineCountryFromXUID, refii::kernel::XamUserGetOnlineCountryFromXUID);
GUEST_FUNCTION_HOOK(__imp__XamVoiceClose, refii::kernel::XamVoiceClose);
GUEST_FUNCTION_HOOK(__imp__XamVoiceCreate, refii::kernel::XamVoiceCreate);
GUEST_FUNCTION_HOOK(__imp__XamVoiceHeadsetPresent, refii::kernel::XamVoiceHeadsetPresent);
GUEST_FUNCTION_HOOK(__imp__XamVoiceSubmitPacket, refii::kernel::XamVoiceSubmitPacket);
GUEST_FUNCTION_HOOK(__imp__NetDll_ioctlsocket, refii::kernel::NetDll_ioctlsocket);
GUEST_FUNCTION_HOOK(__imp__NetDll_recvfrom, refii::kernel::NetDll_recvfrom);
GUEST_FUNCTION_HOOK(__imp__NetDll_sendto, refii::kernel::NetDll_sendto);
GUEST_FUNCTION_HOOK(__imp__NetDll_WSAGetLastError, refii::kernel::NetDll_WSAGetLastError);
GUEST_FUNCTION_HOOK(__imp__NetDll_WSAGetOverlappedResult, refii::kernel::NetDll_WSAGetOverlappedResult);
GUEST_FUNCTION_HOOK(__imp__NetDll_WSARecvFrom, refii::kernel::NetDll_WSARecvFrom);
GUEST_FUNCTION_HOOK(__imp__NetDll_WSASendTo, refii::kernel::NetDll_WSASendTo);
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetCleanup, refii::kernel::NetDll_XNetCleanup);
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetConnect, refii::kernel::NetDll_XNetConnect);
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetGetConnectStatus, refii::kernel::NetDll_XNetGetConnectStatus);
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetInAddrToXnAddr, refii::kernel::NetDll_XNetInAddrToXnAddr);
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetRandom, refii::kernel::NetDll_XNetRandom);
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetRegisterKey, refii::kernel::NetDll_XNetRegisterKey);
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetUnregisterInAddr, refii::kernel::NetDll_XNetUnregisterInAddr);
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetUnregisterKey, refii::kernel::NetDll_XNetUnregisterKey);
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetXnAddrToInAddr, refii::kernel::NetDll_XNetXnAddrToInAddr);
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetXnAddrToMachineId, refii::kernel::NetDll_XNetXnAddrToMachineId);
GUEST_FUNCTION_HOOK(__imp__XNetLogonGetTitleID, refii::kernel::XNetLogonGetTitleID);
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetQosListen, refii::kernel::NetDll_XNetQosListen);
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetQosRelease, refii::kernel::NetDll_XNetQosRelease);



// stubs
GUEST_FUNCTION_STUB(sub_82CC5D38); // HeapCreate
GUEST_FUNCTION_STUB(sub_82CC6C40); // HeapDestroy
GUEST_FUNCTION_STUB(__imp__vsprintf);
GUEST_FUNCTION_STUB(__imp___vsnprintf);
GUEST_FUNCTION_STUB(__imp__sprintf);
GUEST_FUNCTION_STUB(__imp___snprintf);
GUEST_FUNCTION_STUB(__imp___snwprintf);
GUEST_FUNCTION_STUB(__imp__vswprintf);
GUEST_FUNCTION_STUB(__imp___vscwprintf);
GUEST_FUNCTION_STUB(__imp__swprintf);



//#define GUEST__Direct3D_CreateDevice sub_8246B710
//#define GUEST__D3DDevice_SetRenderTarget sub_82473D78
//#define GUEST__D3DDevice_CreateSurface sub_8246D540
//#define GUEST__D3DDevice_SetDepthStencilSurface sub_824740A8
//#define GUEST__D3DSurface_GetDesc sub_8246D668
//#define GUEST__D3DDevice_CreateTexture sub_8246D420
//#define GUEST__D3DDevice_SetTexture sub_8246D788
//#define GUEST__LockTextureRect sub_8246D408
//#define GUEST__D3DDevice_CreatePixelShader sub_8247D840
//#define GUEST__D3DDevice_SetPixelShader sub_8247BF68
//#define GUEST__D3DDevice_CreateIndexBuffer sub_82481310
//#define GUEST__D3DIndexBuffer_Lock sub_824813C0
//#define GUEST__D3DIndexBuffer_Unlock sub_82481420
//#define GUEST__D3DDevice_CreateVertexShader sub_8247D938
//#define GUEST__D3DDevice_CreateVertexBuffer sub_824811D8
//#define GUEST__D3DVertexBuffer_Lock sub_824812A0
//#define GUEST__D3DVertexBuffer_Unlock sub_82481300
//#define GUEST__D3DDevice_SetVertexShader sub_8247C228
//#define GUEST__UnlockTextureRect sub_8246B870
//#define GUEST__D3D_DestroyResource sub_82480B68
//#define GUEST__D3DXFillTexture
//#define GUEST__D3DXFillVolumeTexture
//#define GUEST__D3DDevice_SetStreamSource sub_82473838
//#define GUEST__D3DDevice_SetScissorRect sub_82473548
//#define GUEST__D3DDevice_DrawIndexedVertices sub_82490030
//#define GUEST__D3DDevice_DrawVertices sub_8248FC28
//#define GUEST__D3DDevice_SetVertexDeclaration sub_8247C3F8
//#define GUEST__D3DDevice_SetIndices sub_82473960
//#define GUEST__D3DDevice_Resolve sub_82478FE8
//#define GUEST__D3DDevice_Clear sub_82489F40
//#define GUEST__D3DDevice_SetViewport sub_8215D508
//#define GUEST__D3DIndexBuffer_GetDesc
//#define GUEST__D3DVertexBuffer_GetDesc
//#define GUEST__D3DXFilterTexture sub_82495F10
//#define GUEST__D3DDevice_Present sub_82273858
//#define GUEST__D3DDevice_SetResolution sub_824E39C8
//#define GUEST__D3DDevice_DrawVerticesUP sub_8248FBC8
//#define GUEST__D3DDevice_AcquireThreadOwnership sub_8246B5A8
//#define GUEST__D3DDevice_ReleaseThreadOwnership sub_8246B5E8
//#define GUEST__D3DDevice_Release sub_8246B4E0
//#define GUEST__XGGetTextureDesc sub_82576868
//#define GUEST__D3DDevice_SetPrediction sub_8248A000
//#define GUEST__D3DDevice_SetShaderGPRAllocation sub_8247C4F8
//#define GUEST__D3DDevice_CreateVertexDeclaration sub_8247C418
//#define GUEST__D3DXCompileShaderEx sub_82497FF8
//#define GUEST__hcgVertexShaderCreateByHlsl sub_82286630
//#define GUEST__hcgPixelShaderCreateByHlsl sub_82286770
//#define GUEST__D3DDevice_SetGammaRamp sub_82473A78
//
//GUEST_FUNCTION_HOOK(GUEST__hcgVertexShaderCreateByHlsl, refii::gpu::CreateMovieVertexShader);
//GUEST_FUNCTION_HOOK(GUEST__hcgPixelShaderCreateByHlsl, refii::gpu::CreateMoviePixelShader);
//
//
//GUEST_FUNCTION_HOOK(GUEST__Direct3D_CreateDevice, refii::gpu::CreateDevice);
//GUEST_FUNCTION_HOOK(GUEST__D3DDevice_SetRenderTarget, refii::gpu::SetRenderTarget);
//GUEST_FUNCTION_HOOK(GUEST__D3D_DestroyResource, refii::gpu::DestructResource);
//GUEST_FUNCTION_HOOK(GUEST__D3DDevice_SetStreamSource, refii::gpu::SetStreamSource);
//GUEST_FUNCTION_HOOK(GUEST__D3DDevice_SetViewport, refii::gpu::SetViewport);
//GUEST_FUNCTION_HOOK(GUEST__D3DDevice_Clear, refii::gpu::Clear);
//GUEST_FUNCTION_HOOK(GUEST__D3DDevice_Present, Video::Present);
//GUEST_FUNCTION_HOOK(GUEST__D3DDevice_SetResolution, refii::gpu::SetResolution);
//GUEST_FUNCTION_HOOK(GUEST__D3DDevice_CreateSurface, refii::gpu::CreateSurface);
//GUEST_FUNCTION_HOOK(GUEST__D3DSurface_GetDesc, refii::gpu::GetSurfaceDesc);
//
//GUEST_FUNCTION_HOOK(GUEST__D3DDevice_SetDepthStencilSurface, refii::gpu::SetDepthStencilSurface);
//GUEST_FUNCTION_HOOK(GUEST__D3DDevice_CreateTexture, refii::gpu::CreateTexture);
//GUEST_FUNCTION_HOOK(GUEST__D3DDevice_SetTexture, refii::gpu::SetTexture);
//GUEST_FUNCTION_HOOK(GUEST__LockTextureRect, refii::gpu::LockTextureRect);
//GUEST_FUNCTION_HOOK(GUEST__UnlockTextureRect, refii::gpu::UnlockTextureRect);
//GUEST_FUNCTION_HOOK(GUEST__D3DDevice_SetScissorRect, refii::gpu::SetScissorRect);
//GUEST_FUNCTION_HOOK(GUEST__D3DDevice_Resolve, refii::gpu::StretchRect);
//GUEST_FUNCTION_HOOK(GUEST__D3DDevice_SetIndices, refii::gpu::SetIndices);
//
//GUEST_FUNCTION_HOOK(GUEST__D3DDevice_CreatePixelShader, refii::gpu::CreatePixelShader);
//GUEST_FUNCTION_HOOK(GUEST__D3DDevice_SetPixelShader, refii::gpu::SetPixelShader);
//
//GUEST_FUNCTION_HOOK(GUEST__D3DDevice_CreateIndexBuffer, refii::gpu::CreateIndexBuffer);
//GUEST_FUNCTION_HOOK(GUEST__D3DIndexBuffer_Lock, refii::gpu::LockIndexBuffer);
//GUEST_FUNCTION_HOOK(GUEST__D3DIndexBuffer_Unlock, refii::gpu::UnlockIndexBuffer);
////GUEST_FUNCTION_HOOK(GUEST__D3DIndexBuffer_GetDesc, refii::gpu::GetIndexBufferDesc);
//
//GUEST_FUNCTION_HOOK(GUEST__D3DDevice_CreateVertexShader, refii::gpu::CreateVertexShader);
//GUEST_FUNCTION_HOOK(GUEST__D3DDevice_SetVertexShader, refii::gpu::SetVertexShader);
//GUEST_FUNCTION_HOOK(GUEST__D3DDevice_CreateVertexDeclaration, refii::gpu::CreateVertexDeclaration);
////GUEST_FUNCTION_HOOK(, refii::gpu::GetVertexDeclaration);
//GUEST_FUNCTION_HOOK(GUEST__D3DDevice_SetVertexDeclaration, refii::gpu::SetVertexDeclaration);
////GUEST_FUNCTION_HOOK(, refii::gpu::HashVertexDeclaration);
//
//GUEST_FUNCTION_HOOK(GUEST__D3DDevice_CreateVertexBuffer, refii::gpu::CreateVertexBuffer);
//GUEST_FUNCTION_HOOK(GUEST__D3DVertexBuffer_Lock, refii::gpu::LockVertexBuffer);
//GUEST_FUNCTION_HOOK(GUEST__D3DVertexBuffer_Unlock, refii::gpu::UnlockVertexBuffer);
////GUEST_FUNCTION_HOOK(, refii::gpu::GetVertexBufferDesc);
//
//GUEST_FUNCTION_HOOK(GUEST__D3DDevice_DrawIndexedVertices, refii::gpu::DrawIndexedPrimitive);
//GUEST_FUNCTION_HOOK(GUEST__D3DDevice_DrawVertices, refii::gpu::DrawPrimitive);
//GUEST_FUNCTION_HOOK(GUEST__D3DDevice_DrawVerticesUP, refii::gpu::DrawPrimitiveUP);
//
////GUEST_FUNCTION_HOOK(GUEST__D3DXFillTexture, refii::gpu::D3DXFillTexture);
////GUEST_FUNCTION_HOOK(GUEST__D3DXFillVolumeTexture, refii::gpu::D3DXFillVolumeTexture);
//
//
//GUEST_FUNCTION_STUB(sub_8246EEB8); // D3D::D3DDevice_BlockUntilIdle
//GUEST_FUNCTION_STUB(GUEST__D3DDevice_AcquireThreadOwnership);
//GUEST_FUNCTION_STUB(GUEST__D3DDevice_ReleaseThreadOwnership);
//GUEST_FUNCTION_STUB(GUEST__D3DDevice_SetGammaRamp); // SetGammaRamp
//GUEST_FUNCTION_STUB(GUEST__D3DDevice_SetShaderGPRAllocation);
//GUEST_FUNCTION_STUB(GUEST__D3DXCompileShaderEx);
//GUEST_FUNCTION_STUB(GUEST__D3DDevice_SetPrediction); // D3DDevice_SetPredication
//GUEST_FUNCTION_STUB(GUEST__D3DXFilterTexture); // D3DXFilterTexture
//GUEST_FUNCTION_STUB(GUEST__D3DDevice_Release); // D3DDevice_Release
//GUEST_FUNCTION_STUB(sub_82466ED8);


