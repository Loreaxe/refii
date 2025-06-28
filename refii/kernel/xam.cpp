#include <stdafx.h>
#include "xam.h"
#include <hid/hid.h>
#include <ui/game_window.h>
#include <cpu/guest_thread.h>
#include <ranges>
#include <unordered_set>
#include "xxHashMap.h"
#include <user/paths.h>
#include <SDL.h>
#include <kernel/kernel.h>

struct XamListener : refii::kernel::KernelObject
{
    uint32_t id{};
    uint64_t areas{};
    std::vector<std::tuple<uint32_t, uint32_t>> notifications;

    XamListener(const XamListener&) = delete;
    XamListener& operator=(const XamListener&) = delete;

    XamListener();
    ~XamListener();
};

struct XamEnumeratorBase :refii::kernel::KernelObject
{
    virtual uint32_t Next(void* buffer)
    {
        return -1;
    }
};

template<typename TIterator = std::vector<XHOSTCONTENT_DATA>::iterator>
struct XamEnumerator : XamEnumeratorBase
{
    uint32_t fetch;
    size_t size;
    TIterator position;
    TIterator begin;
    TIterator end;

    XamEnumerator() = default;
    XamEnumerator(uint32_t fetch, size_t size, TIterator begin, TIterator end) : fetch(fetch), size(size), position(begin), begin(begin), end(end)
    {

    }

    uint32_t Next(void* buffer) override
    {
        if (position == end)
        {
            return -1;
        }

        if (buffer == nullptr)
        {
            for (size_t i = 0; i < fetch; i++)
            {
                if (position == end)
                {
                    return i == 0 ? -1 : i;
                }

                ++position;
            }
        }

        for (size_t i = 0; i < fetch; i++)
        {
            if (position == end)
            {
                return i == 0 ? -1 : i;
            }

            memcpy(buffer, &*position, size);

            ++position;
            buffer = (void*)((size_t)buffer + size);
        }

        return fetch;
    }
};

std::array<xxHashMap<XHOSTCONTENT_DATA>, 3> gContentRegistry{};
std::unordered_set<XamListener*> gListeners{};
xxHashMap<std::string> gRootMap;

std::string_view refii::kernel::XamGetRootPath(const std::string_view& root)
{
    const auto result = gRootMap.find(StringHash(root));

    if (result == gRootMap.end())
        return "";

    return result->second;
}

void refii::kernel::XamRootCreate(const std::string_view& root, const std::string_view& path)
{
    gRootMap.emplace(StringHash(root), path);
}

XamListener::XamListener()
{
    gListeners.insert(this);
}

XamListener::~XamListener()
{
    gListeners.erase(this);
}

XCONTENT_DATA refii::kernel::XamMakeContent(uint32_t type, const std::string_view& name)
{
    XCONTENT_DATA data{ 1, type };

    strncpy(data.szFileName, name.data(), sizeof(data.szFileName));

    return data;
}

void refii::kernel::XamRegisterContent(const XCONTENT_DATA& data, const std::string_view& root)
{
    const auto idx = data.dwContentType - 1;

    gContentRegistry[idx].emplace(StringHash(data.szFileName), XHOSTCONTENT_DATA{ data }).first->second.szRoot = root;
}

void XamRegisterContent(uint32_t type, const std::string_view name, const std::string_view& root)
{
    XCONTENT_DATA data{ 1, type, {}, "" };

    strncpy(data.szFileName, name.data(), sizeof(data.szFileName));

    refii::kernel::XamRegisterContent(data, root);
}

uint32_t refii::kernel::XamNotifyCreateListener(uint64_t qwAreas)
{
    auto* listener = refii::kernel::CreateKernelObject<XamListener>();

    listener->areas = qwAreas;

    return refii::kernel::GetKernelHandle(listener);
}

void refii::kernel::XamNotifyEnqueueEvent(uint32_t dwId, uint32_t dwParam)
{
    for (const auto& listener : gListeners)
    {
        if (((1 << MSG_AREA(dwId)) & listener->areas) == 0)
            continue;

        listener->notifications.emplace_back(dwId, dwParam);
    }
}

bool refii::kernel::XNotifyGetNext(uint32_t hNotification, uint32_t dwMsgFilter, be<uint32_t>* pdwId, be<uint32_t>* pParam)
{
    auto& listener = *refii::kernel::GetKernelObject<XamListener>(hNotification);

    if (dwMsgFilter)
    {
        for (size_t i = 0; i < listener.notifications.size(); i++)
        {
            if (std::get<0>(listener.notifications[i]) == dwMsgFilter)
            {
                if (pdwId)
                    *pdwId = std::get<0>(listener.notifications[i]);

                if (pParam)
                    *pParam = std::get<1>(listener.notifications[i]);

                listener.notifications.erase(listener.notifications.begin() + i);

                return true;
            }
        }
    }
    else
    {
        if (listener.notifications.empty())
            return false;

        if (pdwId)
            *pdwId = std::get<0>(listener.notifications[0]);

        if (pParam)
            *pParam = std::get<1>(listener.notifications[0]);

        listener.notifications.erase(listener.notifications.begin());

        return true;
    }

    return false;
}

uint32_t refii::kernel::XamShowMessageBoxUI(uint32_t dwUserIndex, be<uint16_t>* wszTitle, be<uint16_t>* wszText, uint32_t cButtons,
    xpointer<be<uint16_t>>* pwszButtons, uint32_t dwFocusButton, uint32_t dwFlags, be<uint32_t>* pResult, XXOVERLAPPED* pOverlapped)
{
    *pResult = cButtons ? cButtons - 1 : 0;

#if _DEBUG
    assert("XamShowMessageBoxUI encountered!" && false);
#elif _WIN32
    // This code is Win32-only as it'll most likely crash, misbehave or
    // cause corruption due to using a different type of memory than what
    // wchar_t is on Linux. Windows uses 2 bytes while Linux uses 4 bytes.
    std::vector<std::wstring> texts{};

    texts.emplace_back(reinterpret_cast<wchar_t*>(wszTitle));
    texts.emplace_back(reinterpret_cast<wchar_t*>(wszText));

    for (size_t i = 0; i < cButtons; i++)
        texts.emplace_back(reinterpret_cast<wchar_t*>(pwszButtons[i].get()));

    for (auto& text : texts)
    {
        for (size_t i = 0; i < text.size(); i++)
            ByteSwapInplace(text[i]);
    }

    wprintf(L"[XamShowMessageBoxUI] !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    wprintf(L"[XamShowMessageBoxUI] If you are encountering this message and the game has ceased functioning,\n");
    wprintf(L"[XamShowMessageBoxUI] please create an issue at https://github.com/hedge-dev/UnleashedRecomp/issues.\n");
    wprintf(L"[XamShowMessageBoxUI] !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    wprintf(L"[XamShowMessageBoxUI] %ls\n", texts[0].c_str());
    wprintf(L"[XamShowMessageBoxUI] %ls\n", texts[1].c_str());
    wprintf(L"[XamShowMessageBoxUI] ");

    for (size_t i = 0; i < cButtons; i++)
    {
        wprintf(L"%ls", texts[2 + i].c_str());

        if (i != cButtons - 1)
            wprintf(L" | ");
    }

    wprintf(L"\n");
    wprintf(L"[XamShowMessageBoxUI] Defaulted to button: %d\n", pResult->get());
    wprintf(L"[XamShowMessageBoxUI] !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
#endif

    if (pOverlapped)
    {
        pOverlapped->dwCompletionContext = GuestThread::GetCurrentThreadId();
        pOverlapped->Error = 0;
        pOverlapped->Length = -1;
    }

    XamNotifyEnqueueEvent(9, 0);

    return 0;
}

uint32_t refii::kernel::XamContentCreateEnumerator(uint32_t dwUserIndex, uint32_t DeviceID, uint32_t dwContentType,
    uint32_t dwContentFlags, uint32_t cItem, be<uint32_t>* pcbBuffer, be<uint32_t>* phEnum)
{
    if (dwUserIndex != 0)
    {
        GuestThread::SetLastError(ERROR_NO_SUCH_USER);
        return 0xFFFFFFFF;
    }

    const auto& registry = gContentRegistry[dwContentType - 1];
    const auto& values = registry | std::views::values;
    auto* enumerator = refii::kernel::CreateKernelObject<XamEnumerator<decltype(values.begin())>>(cItem, sizeof(_XCONTENT_DATA), values.begin(), values.end());

    if (pcbBuffer)
        *pcbBuffer = sizeof(_XCONTENT_DATA) * cItem;

    *phEnum = refii::kernel::GetKernelHandle(enumerator);

    return 0;
}

uint32_t refii::kernel::XamEnumerate(uint32_t hEnum, uint32_t dwFlags, void* pvBuffer, uint32_t cbBuffer, be<uint32_t>* pcItemsReturned, XXOVERLAPPED* pOverlapped)
{
    auto* enumerator = refii::kernel::GetKernelObject<XamEnumeratorBase>(hEnum);
    const auto count = enumerator->Next(pvBuffer);

    if (count == -1)
        return ERROR_NO_MORE_FILES;

    if (pcItemsReturned)
        *pcItemsReturned = count;

    return ERROR_SUCCESS;
}

uint32_t refii::kernel::XamContentCreateEx(uint32_t dwUserIndex, const char* szRootName, const XCONTENT_DATA* pContentData,
    uint32_t dwContentFlags, be<uint32_t>* pdwDisposition, be<uint32_t>* pdwLicenseMask,
    uint32_t dwFileCacheSize, uint64_t uliContentSize, PXXOVERLAPPED pOverlapped)
{
    const auto& registry = gContentRegistry[pContentData->dwContentType - 1];
    const auto exists = registry.contains(StringHash(pContentData->szFileName));
    const auto mode = dwContentFlags & 0xF;

    if (mode == CREATE_ALWAYS)
    {
        if (pdwDisposition)
            *pdwDisposition = XCONTENT_NEW;

        if (!exists)
        {
            std::string root = "";

            if (pContentData->dwContentType == XCONTENTTYPE_SAVEDATA)
            {
                std::u8string savePathU8 = GetSavePath(true).u8string();
                root = (const char *)(savePathU8.c_str());
            }
            else if (pContentData->dwContentType == XCONTENTTYPE_DLC)
            {
                root = GAME_INSTALL_DIRECTORY "/dlc";
            }
            else
            {
                root = GAME_INSTALL_DIRECTORY;
            }

            XamRegisterContent(*pContentData, root);

            std::error_code ec;
            std::filesystem::create_directory(std::u8string_view((const char8_t*)(root.c_str())), ec);

            XamRootCreate(szRootName, root);
        }
        else
        {
            XamRootCreate(szRootName, registry.find(StringHash(pContentData->szFileName))->second.szRoot);
        }

        return ERROR_SUCCESS;
    }

    if (mode == OPEN_EXISTING)
    {
        if (exists)
        {
            if (pdwDisposition)
                *pdwDisposition = XCONTENT_EXISTING;

            XamRootCreate(szRootName, registry.find(StringHash(pContentData->szFileName))->second.szRoot);

            return ERROR_SUCCESS;
        }
        else
        {
            if (pdwDisposition)
                *pdwDisposition = XCONTENT_NEW;

            return ERROR_PATH_NOT_FOUND;
        }
    }

    return ERROR_PATH_NOT_FOUND;
}

uint32_t refii::kernel::XamContentClose(const char* szRootName, XXOVERLAPPED* pOverlapped)
{
    gRootMap.erase(StringHash(szRootName));
    return 0;
}

uint32_t refii::kernel::XamContentGetDeviceData(uint32_t DeviceID, XDEVICE_DATA* pDeviceData)
{
    pDeviceData->DeviceID = DeviceID;
    pDeviceData->DeviceType = XCONTENTDEVICETYPE_HDD;
    pDeviceData->ulDeviceBytes = 0x10000000;
    pDeviceData->ulDeviceFreeBytes = 0x10000000;
    pDeviceData->wszName[0] = 'S';
    pDeviceData->wszName[1] = 'o';
    pDeviceData->wszName[2] = 'n';
    pDeviceData->wszName[3] = 'i';
    pDeviceData->wszName[4] = 'c';
    pDeviceData->wszName[5] = '\0';

    return 0;
}

uint32_t refii::kernel::XamInputGetCapabilities(uint32_t unk, uint32_t userIndex, uint32_t flags, XAMINPUT_CAPABILITIES* caps)
{
    uint32_t result = hid::GetCapabilities(userIndex, caps);

    if (result == ERROR_SUCCESS)
    {
        ByteSwapInplace(caps->Flags);
        ByteSwapInplace(caps->Gamepad.wButtons);
        ByteSwapInplace(caps->Gamepad.sThumbLX);
        ByteSwapInplace(caps->Gamepad.sThumbLY);
        ByteSwapInplace(caps->Gamepad.sThumbRX);
        ByteSwapInplace(caps->Gamepad.sThumbRY);
        ByteSwapInplace(caps->Vibration.wLeftMotorSpeed);
        ByteSwapInplace(caps->Vibration.wRightMotorSpeed);
    }

    return result;
}

uint32_t refii::kernel::XamInputGetState(uint32_t userIndex, uint32_t flags, XAMINPUT_STATE* state)
{
    memset(state, 0, sizeof(*state));

    if (hid::IsInputAllowed())
        hid::GetState(userIndex, state);

    auto keyboardState = SDL_GetKeyboardState(NULL);

    if (GameWindow::s_isFocused && !keyboardState[SDL_SCANCODE_LALT])
    {
        if (keyboardState[Config::Key_LeftStickUp])
            state->Gamepad.sThumbLY = 32767;
        if (keyboardState[Config::Key_LeftStickDown])
            state->Gamepad.sThumbLY = -32768;
        if (keyboardState[Config::Key_LeftStickLeft])
            state->Gamepad.sThumbLX = -32768;
        if (keyboardState[Config::Key_LeftStickRight])
            state->Gamepad.sThumbLX = 32767;

        if (keyboardState[Config::Key_RightStickUp])
            state->Gamepad.sThumbRY = 32767;
        if (keyboardState[Config::Key_RightStickDown])
            state->Gamepad.sThumbRY = -32768;
        if (keyboardState[Config::Key_RightStickLeft])
            state->Gamepad.sThumbRX = -32768;
        if (keyboardState[Config::Key_RightStickRight])
            state->Gamepad.sThumbRX = 32767;

        if (keyboardState[Config::Key_LeftTrigger])
            state->Gamepad.bLeftTrigger = 0xFF;
        if (keyboardState[Config::Key_RightTrigger])
            state->Gamepad.bRightTrigger = 0xFF;

        if (keyboardState[Config::Key_DPadUp])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_DPAD_UP;
        if (keyboardState[Config::Key_DPadDown])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_DPAD_DOWN;
        if (keyboardState[Config::Key_DPadLeft])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_DPAD_LEFT;
        if (keyboardState[Config::Key_DPadRight])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_DPAD_RIGHT;

        if (keyboardState[Config::Key_Start])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_START;
        if (keyboardState[Config::Key_Back])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_BACK;

        if (keyboardState[Config::Key_LeftBumper])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_LEFT_SHOULDER;
        if (keyboardState[Config::Key_RightBumper])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_RIGHT_SHOULDER;

        if (keyboardState[Config::Key_A])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_A;
        if (keyboardState[Config::Key_B])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_B;
        if (keyboardState[Config::Key_X])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_X;
        if (keyboardState[Config::Key_Y])
            state->Gamepad.wButtons |= XAMINPUT_GAMEPAD_Y;
    }

    state->Gamepad.wButtons &= ~hid::g_prohibitedButtons;

    if (hid::g_isLeftStickProhibited)
    {
        state->Gamepad.sThumbLX = 0;
        state->Gamepad.sThumbLY = 0;
    }

    if (hid::g_isRightStickProhibited)
    {
        state->Gamepad.sThumbRX = 0;
        state->Gamepad.sThumbRY = 0;
    }

    ByteSwapInplace(state->Gamepad.wButtons);
    ByteSwapInplace(state->Gamepad.sThumbLX);
    ByteSwapInplace(state->Gamepad.sThumbLY);
    ByteSwapInplace(state->Gamepad.sThumbRX);
    ByteSwapInplace(state->Gamepad.sThumbRY);

    return ERROR_SUCCESS;
}

uint32_t refii::kernel::XamInputSetState(uint32_t userIndex, uint32_t flags, XAMINPUT_VIBRATION* vibration)
{
    if (!hid::IsInputDeviceController() || !Config::Vibration)
        return ERROR_SUCCESS;

    ByteSwapInplace(vibration->wLeftMotorSpeed);
    ByteSwapInplace(vibration->wRightMotorSpeed);

    return hid::SetState(userIndex, vibration);
}

uint32_t refii::kernel::XamUserGetSigninState(uint32_t userIndex)
{
    return true;
}

uint32_t refii::kernel::XamGetSystemVersion()
{
    return 0;
}

void refii::kernel::XamContentDelete()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t refii::kernel::XamContentGetCreator(uint32_t userIndex, const XCONTENT_DATA* contentData, be<uint32_t>* isCreator, be<uint64_t>* xuid, XXOVERLAPPED* overlapped)
{
    if (isCreator)
        *isCreator = true;

    if (xuid)
        *xuid = 0xB13EBABEBABEBABE;

    return 0;
}

uint32_t refii::kernel::XamContentGetDeviceState()
{
    return 0;
}

uint32_t refii::kernel::XamUserGetSigninInfo(uint32_t userIndex, uint32_t flags, XUSER_SIGNIN_INFO* info)
{
    if (userIndex == 0)
    {
        memset(info, 0, sizeof(*info));
        info->xuid = 0xB13EBABEBABEBABE;
        info->SigninState = 1;
        strcpy(info->Name, "SWA");
        return 0;
    }

    return 0x00000525; // ERROR_NO_SUCH_USER
}

void refii::kernel::XamShowSigninUI()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t refii::kernel::XamShowDeviceSelectorUI(uint32_t userIndex, uint32_t contentType, uint32_t contentFlags, uint64_t totalRequested, be<uint32_t>* deviceId, XXOVERLAPPED* overlapped)
{
    XamNotifyEnqueueEvent(9, true);
    *deviceId = 1;
    XamNotifyEnqueueEvent(9, false);
    return 0;
}

void refii::kernel::XamShowDirtyDiscErrorUI()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::XamEnableInactivityProcessing()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::XamResetInactivity()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::XamShowMessageBoxUIEx()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::XamLoaderTerminateTitle()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::XamGetExecutionId()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::XamLoaderLaunchTitle()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::XamUserReadProfileSettings(uint32_t titleId, uint32_t userIndex, uint32_t xuidCount, uint64_t* xuids, uint32_t settingCount, uint32_t* settingIds, be<uint32_t>* bufferSize, void* buffer, void* overlapped)
{
    if (buffer != nullptr)
    {
        memset(buffer, 0, *bufferSize);
    }
    else
    {
        *bufferSize = 4;
    }
}

void refii::kernel::XamContentSetThumbnail()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t refii::kernel::XamLoaderGetDvdTrayState()
{
    return 0; // 0 = closed
}

void refii::kernel::XamTaskCloseHandle()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::XamTaskSchedule()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::XamTaskShouldExit()
{
    LOG_UTILITY("!!! STUB !!!");
}

void refii::kernel::XamUserCreateAchievementEnumerator()
{
    LOG_UTILITY("!!! STUB !!!");
}

uint32_t refii::kernel::XamUserGetName(uint32_t userIndex, char* userName, uint32_t userNameLength)
{
    if (userIndex == 0 && userName != nullptr && userNameLength > 0)
    {
        strncpy(userName, "Player", userNameLength - 1);
        userName[userNameLength - 1] = '\0';
        return 0;
    }
    return 0x00000525; // ERROR_NO_SUCH_USER
}

uint32_t refii::kernel::XamUserGetXUID(uint32_t userIndex, be<uint64_t>* xuid)
{
    if (userIndex == 0 && xuid != nullptr)
    {
        *xuid = 0xB13EBABEBABEBABE;
        return 0;
    }
    return 0x00000525; // ERROR_NO_SUCH_USER
}

void refii::kernel::XamUserWriteProfileSettings()
{
    LOG_UTILITY("!!! STUB !!!");
}

#define E_INVALIDARG 0x80070057 // ERROR_INVALID_PARAMETER
#define E_NOTIMPL 0x80004001
DECLARE_STUB_FUNCTION_RETURN(uint32_t, refii::kernel::XamGetPrivateEnumStructureFromHandle,
    (uint32_t enumHandle, uint32_t objPtrPtr), E_INVALIDARG)


DECLARE_STUB_FUNCTION_RETURN(uint32_t, refii::kernel::XamShowAchievementsUI, (uint32_t userIndex), 0)
DECLARE_STUB_FUNCTION_RETURN(uint32_t, refii::kernel::XamShowKeyboardUI, (), 0)


DECLARE_STUB_FUNCTION_RETURN(uint32_t, refii::kernel::XMsgCancelIORequest, 
    (uint32_t overlappedPtr, uint32_t wait), E_NOTIMPL)
DECLARE_STUB_FUNCTION_RETURN(uint32_t, refii::kernel::XMsgCompleteIORequest,
    (uint32_t overlappedPtr, uint32_t status, uint32_t extendedError, uint32_t bytesTransferred), E_NOTIMPL)
DECLARE_STUB_FUNCTION_RETURN(uint32_t, refii::kernel::XamGetOverlappedResult,
    (uint32_t overlappedPtr, uint32_t resultPtr, uint32_t wait), E_NOTIMPL)


DECLARE_STUB_FUNCTION_VOID(refii::kernel::XamSessionRefObjByHandle, ())
DECLARE_STUB_FUNCTION_VOID(refii::kernel::XamSessionCreateHandle, ())
DECLARE_STUB_FUNCTION_VOID(refii::kernel::XamUserGetMembershipTierFromXUID, ())
DECLARE_STUB_FUNCTION_VOID(refii::kernel::XamUserGetOnlineCountryFromXUID, ())
DECLARE_STUB_FUNCTION_VOID(refii::kernel::XamVoiceClose, ())
DECLARE_STUB_FUNCTION_VOID(refii::kernel::XamVoiceCreate, ())
DECLARE_STUB_FUNCTION_VOID(refii::kernel::XamVoiceHeadsetPresent, ())
DECLARE_STUB_FUNCTION_VOID(refii::kernel::XamVoiceSubmitPacket, ())
