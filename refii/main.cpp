#include <stdafx.h>
#include <cpuid.h>
#include <cpu/guest_thread.h>
#include <gpu/video.h>
#include <kernel/function.h>
#include <kernel/kernel.h>
#include <kernel/xam.h>
#include <kernel/io/file_system.h>
#include <file.h>
#include <xex.h>
#include <apu/audio.h>
#include <hid/hid.h>
#include <user/config.h>
#include <user/paths.h>
#include <user/persistent_storage_manager.h>
#include <user/registry.h>
#include <kernel/xdbf.h>
#include <os/logger.h>
#include <os/process.h>
#include <os/registry.h>
#include <ui/game_window.h>
#include <preload_executable.h>
#include <kernel/linux_hooks.h>

#ifdef _WIN32
#include <timeapi.h>
#endif

#if defined(_WIN32) && defined(UNLEASHED_RECOMP_D3D12)
static std::array<std::string_view, 3> g_D3D12RequiredModules =
{
    "D3D12/D3D12Core.dll",
    "dxcompiler.dll",
    "dxil.dll"
};
#endif

#ifndef _WIN32
#define OPEN_EXISTING 3
#endif

const size_t XMAIOBegin = 0x7FEA0000;
const size_t XMAIOEnd = XMAIOBegin + 0x0000FFFF;
refii::kernel::GuestMemory refii::kernel::g_memory;
refii::kernel::GuestHeap refii::kernel::g_userHeap;
XDBFWrapper g_xdbfWrapper;
std::unordered_map<uint16_t, GuestTexture*> g_xdbfTextureCache;

uint32_t LdrLoadModule(const std::filesystem::path &path)
{
    auto loadResult = LoadFile(path);
    if (loadResult.empty())
    {
        assert("Failed to load module" && false);
        return 0;
    }

    auto* header = reinterpret_cast<const Xex2Header*>(loadResult.data());
    auto* security = reinterpret_cast<const Xex2SecurityInfo*>(loadResult.data() + header->securityOffset);
    const auto* fileFormatInfo = reinterpret_cast<const Xex2OptFileFormatInfo*>(getOptHeaderPtr(loadResult.data(), XEX_HEADER_FILE_FORMAT_INFO));
    auto entry = *reinterpret_cast<const uint32_t*>(getOptHeaderPtr(loadResult.data(), XEX_HEADER_ENTRY_POINT));
    ByteSwapInplace(entry);

    auto srcData = loadResult.data() + header->headerSize;
    auto destData = reinterpret_cast<uint8_t*>(refii::kernel::g_memory.Translate(security->loadAddress));

    if (fileFormatInfo->compressionType == XEX_COMPRESSION_NONE)
    {
        memcpy(destData, srcData, security->imageSize);
    }
    else if (fileFormatInfo->compressionType == XEX_COMPRESSION_BASIC)
    {
        auto* blocks = reinterpret_cast<const Xex2FileBasicCompressionBlock*>(fileFormatInfo + 1);
        const size_t numBlocks = (fileFormatInfo->infoSize / sizeof(Xex2FileBasicCompressionInfo)) - 1;

        for (size_t i = 0; i < numBlocks; i++)
        {
            memcpy(destData, srcData, blocks[i].dataSize);

            srcData += blocks[i].dataSize;
            destData += blocks[i].dataSize;

            memset(destData, 0, blocks[i].zeroSize);
            destData += blocks[i].zeroSize;
        }
    }
    else
    {
        assert(false && "Unknown compression type.");
    }

    auto res = reinterpret_cast<const Xex2ResourceInfo*>(getOptHeaderPtr(loadResult.data(), XEX_HEADER_RESOURCE_INFO));

    g_xdbfWrapper = XDBFWrapper((uint8_t*)refii::kernel::g_memory.Translate(res->offset.get()), res->sizeOfData);

    return entry;
}

__attribute__((noinline))
void PrintAVXUnsupported()
{
    printf("[*] CPU does not support the AVX instruction set.\n");

#ifdef _WIN32
    MessageBoxA(nullptr, "Your CPU does not meet the minimum system requirements.", "Unleashed Recompiled", MB_ICONERROR);
#endif

    std::_Exit(1);
}

__attribute__((constructor(101), target("no-avx,no-avx2"), noinline))

void init()
{
#ifdef __x86_64__
    uint32_t eax, ebx, ecx, edx;
    __get_cpuid(1, &eax, &ebx, &ecx, &edx);

    if ((ecx & (1 << 28)) == 0)
    {
        PrintAVXUnsupported();  // now legal
    }
#endif
}

static std::filesystem::path GetProjectRoot()
{
    static const std::filesystem::path rootPath = []() -> std::filesystem::path {
#if defined(__linux__)
        char exePath[PATH_MAX] = {};
        ssize_t len = ::readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
        if (len > 0 && len < PATH_MAX) {
            exePath[len] = '\0';
            std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();

            while (!exeDir.empty()) {
                if (exeDir.filename() == "Git") {
                    return exeDir;
                }

                std::filesystem::path parent = exeDir.parent_path();
                if (parent == exeDir) // reached root
                    break;
                exeDir = std::move(parent);
            }
        }
#endif
        // fallback
        return std::filesystem::current_path();
    }();

    return rootPath;
}

static std::filesystem::path MapVirtualPath(const std::filesystem::path& virtualPath)
{
#ifdef _WIN32
    return virtualPath;
#else
    std::string pathStr = virtualPath.string();

    // Handle Windows-style path prefixes
    if (pathStr.starts_with("P:\\")) {
        pathStr = pathStr.substr(3); // remove "P:\\"
    } else if (pathStr.starts_with("P:/")) {
        pathStr = pathStr.substr(3); // remove "P:/"
    }

    // Convert any remaining backslashes to forward slashes
    std::replace(pathStr.begin(), pathStr.end(), '\\', '/');

    // Use auto-detected root path
    auto fullPath = GetProjectRoot() / pathStr;

    return std::filesystem::weakly_canonical(fullPath); // normalize path
#endif
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    timeBeginPeriod(1);
#endif

    os::process::CheckConsole();

    if (!os::registry::Init())
        LOGN_WARNING("OS does not support registry.");

    os::logger::Init();

    PreloadContext preloadContext;
    preloadContext.PreloadExecutable();

    bool forceInstaller = false;
    bool forceDLCInstaller = false;
    bool useDefaultWorkingDirectory = true;
    bool forceInstallationCheck = false;
    bool graphicsApiRetry = false;
    const char *sdlVideoDriver = nullptr;

    // bootleg paths
    std::filesystem::path exePath = os::process::GetExecutablePath();
    std::filesystem::path modulePath = exePath.parent_path() / "default.xex";

    if (!useDefaultWorkingDirectory)
    {
        // Set the current working directory to the executable's path.
        std::error_code ec;
        std::filesystem::current_path(os::process::GetExecutablePath().parent_path(), ec);
    }

    Config::Load();


#if defined(_WIN32) && defined(UNLEASHED_RECOMP_D3D12)
    for (auto& dll : g_D3D12RequiredModules)
    {
        if (!std::filesystem::exists(refiiBinPath / dll))
        {
            char text[512];
            snprintf(text, sizeof(text), Localise("System_Win32_MissingDLLs").c_str(), dll.data());
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, GameWindow::GetTitle(), text, GameWindow::s_pWindow);
            std::_Exit(1);
        }
    }
#endif

    os::process::ShowConsole();

#ifdef _WIN32
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
#endif

    hid::Init();

    bool isGameInstalled = true;// Installer::checkGameInstall(GAME_INSTALL_DIRECTORY, modulePath);
    bool runInstallerWizard = forceInstaller || forceDLCInstaller || !isGameInstalled;
    //if (runInstallerWizard)
    //{
    //    if (!Video::CreateHostDevice(sdlVideoDriver, graphicsApiRetry))
    //    {
    //        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, GameWindow::GetTitle(), Localise("Video_BackendError").c_str(), GameWindow::s_pWindow);
    //        std::_Exit(1);
    //    }

    //    if (!InstallerWizard::Run(GAME_INSTALL_DIRECTORY, isGameInstalled && forceDLCInstaller))
    //    {
    //        std::_Exit(0);
    //    }
    //}

    //ModLoader::Init();

    //if (!PersistentStorageManager::LoadBinary())
    //    LOGFN_ERROR("Failed to load persistent storage binary... (status code {})", (int)PersistentStorageManager::BinStatus);

    if (refii::kernel::g_memory.base == nullptr)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, GameWindow::GetTitle(), Localise("System_MemoryAllocationFailed").c_str(), GameWindow::s_pWindow);
        std::_Exit(1);
    }

    refii::kernel::g_userHeap.Init();

    const auto gameContent = refii::kernel::XamMakeContent(XCONTENTTYPE_RESERVED, "Game");
    const auto cacheContent = refii::kernel::XamMakeContent(XCONTENTTYPE_RESERVED, "Cache");
    const auto updateContent = refii::kernel::XamMakeContent(XCONTENTTYPE_RESERVED, "update");

    refii::kernel::XamRegisterContent(gameContent, MapVirtualPath("P:/x360/refii-game/game").string());
    refii::kernel::XamRegisterContent(cacheContent, MapVirtualPath("P:/x360/refii-game/cache").string());
    refii::kernel::XamRegisterContent(updateContent, MapVirtualPath("P:/x360/refii-game/update").string());

    // Mount game
    refii::kernel::XamContentCreateEx(0, "game", &gameContent, OPEN_EXISTING, nullptr, nullptr, 0, 0, nullptr);

    // OS mounts game data to D:
    refii::kernel::XamContentCreateEx(0, "D", &gameContent, OPEN_EXISTING, nullptr, nullptr, 0, 0, nullptr);

    // Mount cache
    refii::kernel::XamContentCreateEx(0, "cache", &cacheContent, OPEN_EXISTING, nullptr, nullptr, 0, 0, nullptr);

    // Mount update
    refii::kernel::XamContentCreateEx(0, "update", &updateContent, OPEN_EXISTING, nullptr, nullptr, 0, 0, nullptr);


    //XAudioInitializeSystem();

    uint32_t entry = LdrLoadModule(modulePath);

    //if (!runInstallerWizard)
    //{
        //if (!Video::CreateHostDevice(sdlVideoDriver, graphicsApiRetry))
        //{
        //    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, GameWindow::GetTitle(), Localise("Video_BackendError").c_str(), GameWindow::s_pWindow);
        //    std::_Exit(1);
        //}
    //}

   // Video::StartPipelinePrecompilation();

#ifdef __linux__
    refii::kernel::InitializeGlobalCriticalSections();
    refii::kernel::InitializeCallbackArray();
    refii::kernel::InitializeCallbackRdata();
#endif

    GuestThread::Start({ entry, 0, 0 });

    return 0;
}


