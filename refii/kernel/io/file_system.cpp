#include "file_system.h"
#include <cpu/guest_thread.h>
#include <kernel/xam.h>
#include <kernel/function.h>
#include <os/logger.h>
#include <user/config.h>
#include <stdafx.h>
#include <kernel/kernel.h>
#include <kernel/platform_defs.h>

struct FileHandle : refii::kernel::KernelObject
{
    std::fstream stream;
    std::filesystem::path path;
};

struct CFileHandle : FileHandle
{
    uint32_t shareMode;
    bool isTextMode;

    CFileHandle() : shareMode(0), isTextMode(false) {}
};

struct FindHandle : refii::kernel::KernelObject
{
    std::error_code ec;
    ankerl::unordered_dense::map<std::u8string, std::pair<size_t, bool>> searchResult; // Relative path, file size, is directory
    decltype(searchResult)::iterator iterator;

    FindHandle(const std::string_view& path)
    {
        auto addDirectory = [&](const std::filesystem::path& directory)
            {
                for (auto& entry : std::filesystem::directory_iterator(directory, ec))
                {
                    std::u8string relativePath = entry.path().lexically_relative(directory).u8string();
                    searchResult.emplace(relativePath, std::make_pair(entry.is_directory(ec) ? 0 : entry.file_size(ec), entry.is_directory(ec)));
                }
            };

        std::string_view pathNoPrefix = path;
        size_t index = pathNoPrefix.find(":\\");
        if (index != std::string_view::npos)
            pathNoPrefix.remove_prefix(index + 2);

        // Force add a work folder to let the game see the files in mods,
        // if by some rare chance the user has no DLC or update files.
        if (pathNoPrefix.empty())
            searchResult.emplace(u8"work", std::make_pair(0, true));

        // Look for only work folder in mod folders, AR files cause issues.
        if (pathNoPrefix.starts_with("work"))
        {
            std::string pathStr(pathNoPrefix);
            std::replace(pathStr.begin(), pathStr.end(), '\\', '/');

            for (size_t i = 0; ; i++)
            {
                //auto* includeDirs = nullptr;// ModLoader::GetIncludeDirectories(i);
                //if (includeDirs == nullptr)
                    break;

                //for (auto& includeDir : *includeDirs)
                //    addDirectory(includeDir / pathStr);
            }
        }

        addDirectory(FileSystem::ResolvePath(path, false));

        iterator = searchResult.begin();
    }

    void fillFindData(WIN32_FIND_DATAA* lpFindFileData)
    {
        if (iterator->second.second)
            lpFindFileData->dwFileAttributes = ByteSwap(FILE_ATTRIBUTE_DIRECTORY);
        else
            lpFindFileData->dwFileAttributes = ByteSwap(FILE_ATTRIBUTE_NORMAL);

        strncpy(lpFindFileData->cFileName, (const char *)(iterator->first.c_str()), sizeof(lpFindFileData->cFileName));
        lpFindFileData->nFileSizeLow = ByteSwap(uint32_t(iterator->second.first >> 32U));
        lpFindFileData->nFileSizeHigh = ByteSwap(uint32_t(iterator->second.first));
        lpFindFileData->ftCreationTime = {};
        lpFindFileData->ftLastAccessTime = {};
        lpFindFileData->ftLastWriteTime = {};
    }
};

FileHandle* XCreateFileA
(
    const char* lpFileName,
    uint32_t dwDesiredAccess,
    uint32_t dwShareMode,
    void* lpSecurityAttributes,
    uint32_t dwCreationDisposition,
    uint32_t dwFlagsAndAttributes
)
{
    LOGF_UTILITY("lpFileName: {}, dwDesiredAccess: 0x{:X}, dwShareMode: 0x{:X}, lpSecurityAttributes: {}, dwCreationDisposition: 0x{:X}, dwFlagsAndAttributes: 0x{:X}",
        lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes);
    assert(((dwDesiredAccess & ~(GENERIC_READ | GENERIC_WRITE | FILE_READ_DATA)) == 0) && "Unknown desired access bits.");
    assert(((dwShareMode & ~(FILE_SHARE_READ | FILE_SHARE_WRITE)) == 0) && "Unknown share mode bits.");
    assert(((dwCreationDisposition & ~(CREATE_NEW | CREATE_ALWAYS)) == 0) && "Unknown creation disposition bits.");

    std::filesystem::path filePath = FileSystem::ResolvePath(lpFileName, true);
    std::fstream fileStream;
    std::ios::openmode fileOpenMode = std::ios::binary;
    if (dwDesiredAccess & (GENERIC_READ | FILE_READ_DATA))
    {
        fileOpenMode |= std::ios::in;
    }

    if (dwDesiredAccess & GENERIC_WRITE)
    {
        fileOpenMode |= std::ios::out;
    }

    fileStream.open(filePath, fileOpenMode);
    if (!fileStream.is_open())
    {
#ifdef _WIN32
        GuestThread::SetLastError(GetLastError());
#endif
        return refii::kernel::GetInvalidKernelObject<FileHandle>();
    }

    FileHandle *fileHandle = refii::kernel::CreateKernelObject<FileHandle>();
    fileHandle->stream = std::move(fileStream);
    fileHandle->path = std::move(filePath);
    return fileHandle;
}

static uint32_t XGetFileSizeA(FileHandle* hFile, be<uint32_t>* lpFileSizeHigh)
{
    std::error_code ec;
    auto fileSize = std::filesystem::file_size(hFile->path, ec);
    if (!ec)
    {
        if (lpFileSizeHigh != nullptr)
        {
            *lpFileSizeHigh = uint32_t(fileSize >> 32U);
        }
    
        return (uint32_t)(fileSize);
    }

    return INVALID_FILE_SIZE;
}

uint32_t XGetFileSizeExA(FileHandle* hFile, LARGE_INTEGER* lpFileSize)
{
    std::error_code ec;
    auto fileSize = std::filesystem::file_size(hFile->path, ec);
    if (!ec)
    {
        if (lpFileSize != nullptr)
        {
            lpFileSize->QuadPart = ByteSwap(fileSize);
        }

        return TRUE;
    }

    return FALSE;
}

uint32_t XReadFile
(
    FileHandle* hFile,
    void* lpBuffer,
    uint32_t nNumberOfBytesToRead,
    be<uint32_t>* lpNumberOfBytesRead,
    XOVERLAPPED* lpOverlapped
)
{
    uint32_t result = FALSE;
    if (lpOverlapped != nullptr)
    {
        std::streamoff streamOffset = lpOverlapped->Offset + (std::streamoff(lpOverlapped->OffsetHigh.get()) << 32U);
        hFile->stream.clear();
        hFile->stream.seekg(streamOffset, std::ios::beg);
        if (hFile->stream.bad())
        {
            return FALSE;
        }
    }

    uint32_t numberOfBytesRead;
    hFile->stream.read((char *)(lpBuffer), nNumberOfBytesToRead);
    if (!hFile->stream.bad())
    {
        numberOfBytesRead = uint32_t(hFile->stream.gcount());
        result = TRUE;
    }

    if (result)
    {
        if (lpOverlapped != nullptr)
        {
            lpOverlapped->Internal = 0;
            lpOverlapped->InternalHigh = numberOfBytesRead;
        }
        else if (lpNumberOfBytesRead != nullptr)
        {
            *lpNumberOfBytesRead = numberOfBytesRead;
        }
    }

    return result;
}

uint32_t XSetFilePointer(FileHandle* hFile, int32_t lDistanceToMove, be<int32_t>* lpDistanceToMoveHigh, uint32_t dwMoveMethod)
{
    int32_t distanceToMoveHigh = lpDistanceToMoveHigh ? lpDistanceToMoveHigh->get() : 0;
    std::streamoff streamOffset = lDistanceToMove + (std::streamoff(distanceToMoveHigh) << 32U);
    std::fstream::seekdir streamSeekDir = {};
    switch (dwMoveMethod)
    {
    case FILE_BEGIN:
        streamSeekDir = std::ios::beg;
        break;
    case FILE_CURRENT:
        streamSeekDir = std::ios::cur;
        break;
    case FILE_END:
        streamSeekDir = std::ios::end;
        break;
    default:
        assert(false && "Unknown move method.");
        break;
    }

    hFile->stream.clear();
    hFile->stream.seekg(streamOffset, streamSeekDir);
    if (hFile->stream.bad())
    {
        return INVALID_SET_FILE_POINTER;
    }

    std::streampos streamPos = hFile->stream.tellg();
    if (lpDistanceToMoveHigh != nullptr)
        *lpDistanceToMoveHigh = int32_t(streamPos >> 32U);

    return uint32_t(streamPos);
}

uint32_t XSetFilePointerEx(FileHandle* hFile, int32_t lDistanceToMove, LARGE_INTEGER* lpNewFilePointer, uint32_t dwMoveMethod)
{
    std::fstream::seekdir streamSeekDir = {};
    switch (dwMoveMethod)
    {
    case FILE_BEGIN:
        streamSeekDir = std::ios::beg;
        break;
    case FILE_CURRENT:
        streamSeekDir = std::ios::cur;
        break;
    case FILE_END:
        streamSeekDir = std::ios::end;
        break;
    default:
        assert(false && "Unknown move method.");
        break;
    }

    hFile->stream.clear();
    hFile->stream.seekg(lDistanceToMove, streamSeekDir);
    if (hFile->stream.bad())
    {
        return FALSE;
    }

    if (lpNewFilePointer != nullptr)
    {
        lpNewFilePointer->QuadPart = ByteSwap(int64_t(hFile->stream.tellg()));
    }

    return TRUE;
}

FindHandle* XFindFirstFileA(const char* lpFileName, WIN32_FIND_DATAA* lpFindFileData)
{
    std::string_view path = lpFileName;
    if (path.find("\\*") == (path.size() - 2) || path.find("/*") == (path.size() - 2))
    {
        path.remove_suffix(1);
    }
    else if (path.find("\\*.*") == (path.size() - 4) || path.find("/*.*") == (path.size() - 4))
    {
        path.remove_suffix(3);
    }
    else
    {
        assert(!std::filesystem::path(path).has_extension() && "Unknown search pattern.");
    }

    FindHandle findHandle(path);

    if (findHandle.searchResult.empty())
        return refii::kernel::GetInvalidKernelObject<FindHandle>();

    findHandle.fillFindData(lpFindFileData);

    return refii::kernel::CreateKernelObject<FindHandle>(std::move(findHandle));
}

uint32_t XFindNextFileA(FindHandle* Handle, WIN32_FIND_DATAA* lpFindFileData)
{
    Handle->iterator++;

    if (Handle->iterator == Handle->searchResult.end())
    {
        return FALSE;
    }
    else
    {
        Handle->fillFindData(lpFindFileData);
        return TRUE;
    }
}

uint32_t XReadFileEx(FileHandle* hFile, void* lpBuffer, uint32_t nNumberOfBytesToRead, XOVERLAPPED* lpOverlapped, uint32_t lpCompletionRoutine)
{
    LOGF_UTILITY("hFile=0x{:X}, lpBuffer=0x{:X}, nNumberOfBytesToRead={}, lpOverlapped=0x{:X}, lpCompletionRoutine=0x{:X}",
        (uintptr_t)hFile, (uintptr_t)lpBuffer, nNumberOfBytesToRead, (uintptr_t)lpOverlapped, (uintptr_t)lpCompletionRoutine);
    uint32_t result = FALSE;
    uint32_t numberOfBytesRead;
    std::streamoff streamOffset = lpOverlapped->Offset + (std::streamoff(lpOverlapped->OffsetHigh.get()) << 32U);
    hFile->stream.clear();
    hFile->stream.seekg(streamOffset, std::ios::beg);
    if (hFile->stream.bad())
        return FALSE;

    hFile->stream.read((char *)(lpBuffer), nNumberOfBytesToRead);
    if (!hFile->stream.bad())
    {
        numberOfBytesRead = uint32_t(hFile->stream.gcount());
        result = TRUE;
    }

    if (result)
    {
        lpOverlapped->Internal = 0;
        lpOverlapped->InternalHigh = numberOfBytesRead;
    }

    return result;
}

uint32_t XGetFileAttributesA(const char* lpFileName)
{
    LOGF_UTILITY("lpFileName=\"{}\"", lpFileName ? lpFileName : "(null)");
    std::filesystem::path filePath = FileSystem::ResolvePath(lpFileName, true);
    if (std::filesystem::is_directory(filePath))
        return FILE_ATTRIBUTE_DIRECTORY;
    else if (std::filesystem::is_regular_file(filePath))
        return FILE_ATTRIBUTE_NORMAL;
    else
        return INVALID_FILE_ATTRIBUTES;
}

uint32_t XWriteFile(FileHandle* hFile, const void* lpBuffer, uint32_t nNumberOfBytesToWrite, be<uint32_t>* lpNumberOfBytesWritten, void* lpOverlapped)
{
    assert(lpOverlapped == nullptr && "Overlapped not implemented.");

    hFile->stream.write((const char *)(lpBuffer), nNumberOfBytesToWrite);
    if (hFile->stream.bad())
        return FALSE;

    if (lpNumberOfBytesWritten != nullptr)
        *lpNumberOfBytesWritten = uint32_t(hFile->stream.gcount());

    return TRUE;
}

uint32_t XGetVolumeInformationA(
    const char* lpRootPathName,
    char* lpVolumeNameBuffer,
    uint32_t nVolumeNameSize,
    be<uint32_t>* lpVolumeSerialNumber,
    be<uint32_t>* lpMaximumComponentLength,
    be<uint32_t>* lpFileSystemFlags,
    char* lpFileSystemNameBuffer,
    uint32_t nFileSystemNameSize
)

{
    LOGF_UTILITY("lpRootPathName=\"{}\"",
        lpRootPathName ? lpRootPathName : "(null)");
    uint32_t lastErr = 0;
    auto resolvedPath = FileSystem::ResolvePath(lpRootPathName, false);
    if (resolvedPath.empty()) {
        GuestThread::SetLastError(ERROR_NOT_FOUND);
        return TRUE;
    } else {
        if (lpVolumeNameBuffer)
            strncpy(lpVolumeNameBuffer, "refii", nVolumeNameSize);

        if (lpVolumeSerialNumber)
            *lpVolumeSerialNumber = 0xdeadbeef;
        if (lpMaximumComponentLength)
            *lpMaximumComponentLength = 255;
        if (lpFileSystemFlags)
            *lpFileSystemFlags = 0;
        if (lpFileSystemNameBuffer)
            strncpy(lpFileSystemNameBuffer, "NTFS", nFileSystemNameSize);
        GuestThread::SetLastError(ERROR_SUCCESS); // Success
        return ERROR_SUCCESS;
    }
}



std::filesystem::path FileSystem::ResolvePath(const std::string_view& path, bool checkForMods)
{
    //if (checkForMods)
    //{
    //    std::filesystem::path resolvedPath = ModLoader::ResolvePath(path);

    //    if (!resolvedPath.empty())
    //    {
    //        if (ModLoader::s_isLogTypeConsole)
    //            LOGF_IMPL(Utility, "Mod Loader", "Loading file: \"{}\"", reinterpret_cast<const char*>(resolvedPath.u8string().c_str()));

    //        return resolvedPath;
    //    }
    //}

    thread_local std::string builtPath;
    builtPath.clear();

    size_t index = path.find(":\\");
    if (index != std::string::npos)
    {
        // rooted folder, handle direction
        std::string_view root = path.substr(0, index);

        // HACK: The game tries to load work folder from the "game" root path for 
        // Application and shader archives, which does not work in Recomp because 
        // we don't support stacking the update and game files on top of each other.
        // 
        // We can fix it by redirecting it to update instead as we know the original
        // game files don't have a work folder.
        if (path.starts_with("game:\\work\\"))
            root = "update";

        // really ghetto way of handling the root path
        if(root == "GAME")
            root = "game";
        else if (root == "UPDATE")
            root = "update";

        const auto newRoot = refii::kernel::XamGetRootPath(root);

        if (!newRoot.empty())
        {
            builtPath += newRoot;
            builtPath += '/';
        }
        
        builtPath += path.substr(index + 2);
    }
    else
    {
        builtPath += path;
    }

    std::replace(builtPath.begin(), builtPath.end(), '\\', '/');

    return std::u8string_view((const char8_t*)builtPath.c_str());
}

// Helper function to convert C mode string to file access flags
static void ParseFileMode(const char* mode, uint32_t& desiredAccess, uint32_t& creationDisposition, bool& isTextMode)
{
    desiredAccess = 0;
    creationDisposition = OPEN_EXISTING;
    isTextMode = false;

    if (!mode || !mode[0])
        return;

    // Parse primary mode
    switch (mode[0])
    {
    case 'r':
        desiredAccess = GENERIC_READ;
        creationDisposition = OPEN_EXISTING;
        break;
    case 'w':
        desiredAccess = GENERIC_WRITE;
        creationDisposition = CREATE_ALWAYS;
        break;
    case 'a':
        desiredAccess = GENERIC_WRITE;
        creationDisposition = OPEN_ALWAYS;
        break;
    }

    // Parse modifiers
    for (int i = 1; mode[i]; i++)
    {
        switch (mode[i])
        {
        case '+':
            desiredAccess = GENERIC_READ | GENERIC_WRITE;
            break;
        case 'b':
            // Binary mode (default)
            break;
        case 't':
            isTextMode = true;
            break;
        }
    }
}

CFileHandle* __fsopen(const char* filename, const char* mode, int shflag)
{
    LOGF_UTILITY("filename=\"{}\", mode=\"{}\", shflag=0x{:x}",
        filename ? filename : "(null)",
        mode ? mode : "(null)",
        shflag);

    if (!filename || !mode)
    {
        GuestThread::SetLastError(ERROR_INVALID_PARAMETER);
        return refii::kernel::GetInvalidKernelObject<CFileHandle>();
    }

    uint32_t desiredAccess, creationDisposition;
    bool isTextMode;
    ParseFileMode(mode, desiredAccess, creationDisposition, isTextMode);

    // Convert sharing flags
    uint32_t shareMode = 0;
    switch (shflag)
    {
    case 0x10: // _SH_DENYRW - deny read/write
        shareMode = 0;
        break;
    case 0x20: // _SH_DENYWR - deny write
        shareMode = FILE_SHARE_READ;
        break;
    case 0x30: // _SH_DENYRD - deny read  
        shareMode = FILE_SHARE_WRITE;
        break;
    case 0x40: // _SH_DENYNO - deny none
        shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
        break;
    default:
        shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
        break;
    }

    // Resolve path and open file
    std::filesystem::path filePath = FileSystem::ResolvePath(filename, false);
    std::fstream fileStream;
    std::ios::openmode fileOpenMode = std::ios::binary;

    if (desiredAccess & GENERIC_READ)
        fileOpenMode |= std::ios::in;
    if (desiredAccess & GENERIC_WRITE)
        fileOpenMode |= std::ios::out;

    if (mode[0] == 'a')
        fileOpenMode |= std::ios::app;

    if (mode[0] == 'w')
        fileOpenMode |= std::ios::trunc;

    fileStream.open(filePath, fileOpenMode);
    if (!fileStream.is_open())
    {
        GuestThread::SetLastError(GetLastError());
        return refii::kernel::GetInvalidKernelObject<CFileHandle>();
    }

    CFileHandle* fileHandle = refii::kernel::CreateKernelObject<CFileHandle>();
    fileHandle->stream = std::move(fileStream);
    fileHandle->path = std::move(filePath);
    fileHandle->shareMode = shareMode;
    fileHandle->isTextMode = isTextMode;

    LOGF_UTILITY("handle=0x{:x}", refii::kernel::GetKernelHandle(fileHandle));
    return fileHandle;
}
int __fsclose(CFileHandle* stream)
{
    if (refii::kernel::IsInvalidKernelObject(stream))
    {
        GuestThread::SetLastError(ERROR_INVALID_HANDLE);
        return -1;
    }
    LOGF_UTILITY("stream=0x{:x}", refii::kernel::GetKernelHandle(stream));
    stream->stream.close();
    refii::kernel::DestroyKernelObject(stream);

    return 0;
}
CFileHandle* _fopen(const char* filename, const char* mode)
{
    // Default sharing mode is _SH_DENYNO (allow all)
    return __fsopen(filename, mode, 0x40);
}
int ___fclose(CFileHandle* stream)
{
    return __fsclose(stream);
}
size_t __fread(void* buffer, size_t size, size_t count, CFileHandle* stream)
{

    if (refii::kernel::IsInvalidKernelObject(stream) || !buffer)
    {
        LOGF_ERROR("Invalid stream or buffer: stream=0x{:x}, buffer=0x{:x}", 
            refii::kernel::GetKernelHandle(stream), reinterpret_cast<uintptr_t>(buffer));
        return 0;
    }

    LOGF_UTILITY("buffer=0x{:x}, size={}, count={}, stream=0x{:x}",
        reinterpret_cast<uintptr_t>(buffer), size, count, refii::kernel::GetKernelHandle(stream));

    size_t totalBytes = size * count;
    stream->stream.read(static_cast<char*>(buffer), totalBytes);

    if (stream->stream.bad())
    {
        LOGF_ERROR("Failed to read from stream: stream=0x{:x}", refii::kernel::GetKernelHandle(stream));
        return 0;
    }

    size_t bytesRead = stream->stream.gcount();
    return bytesRead / size;
}
size_t __fwrite(const void* buffer, size_t size, size_t count, CFileHandle* stream)
{
    if (refii::kernel::IsInvalidKernelObject(stream) || !buffer)
    {
        LOGF_ERROR("Invalid stream or buffer: stream=0x{:x}, buffer=0x{:x}", 
            refii::kernel::GetKernelHandle(stream), reinterpret_cast<uintptr_t>(buffer));
        return 0;
    }

    LOGF_UTILITY("buffer=0x{:x}, size={}, count={}, stream=0x{:x}",
        reinterpret_cast<uintptr_t>(buffer), size, count, refii::kernel::GetKernelHandle(stream));

    size_t totalBytes = size * count;
    stream->stream.write(static_cast<const char*>(buffer), totalBytes);

    if (stream->stream.bad())
    {
        LOGF_ERROR("Failed to write to stream: stream=0x{:x}", refii::kernel::GetKernelHandle(stream));
        return 0;
    }

    return count;
}
int __fseek(CFileHandle* stream, long offset, int origin)
{

    if (refii::kernel::IsInvalidKernelObject(stream))
    {
        LOGF_ERROR("Invalid stream: stream=0x{:x}", refii::kernel::GetKernelHandle(stream));
        return -1;
    }

    LOGF_UTILITY("stream=0x{:x}, offset={}, origin={}",
        refii::kernel::GetKernelHandle(stream), offset, origin);

    std::ios::seekdir seekDir;
    switch (origin)
    {
    case SEEK_SET:
        seekDir = std::ios::beg;
        break;
    case SEEK_CUR:
        seekDir = std::ios::cur;
        break;
    case SEEK_END:
        seekDir = std::ios::end;
        break;
    default:
        return -1;
    }

    stream->stream.clear();
    stream->stream.seekg(offset, seekDir);

    return stream->stream.bad() ? -1 : 0;
}
long __ftell(CFileHandle* stream)
{
    if (refii::kernel::IsInvalidKernelObject(stream))
    {
        LOGF_ERROR("Invalid stream: stream=0x{:x}", refii::kernel::GetKernelHandle(stream));
        return -1;
    }
    LOGF_UTILITY("stream=0x{:x}", refii::kernel::GetKernelHandle(stream));
    std::streampos pos = stream->stream.tellg();
    return static_cast<long>(pos);
}
int __feof(CFileHandle* stream)
{
    if (refii::kernel::IsInvalidKernelObject(stream))
    {
        LOGF_ERROR("Invalid stream: stream=0x{:x}", refii::kernel::GetKernelHandle(stream));
        return -1;
    }
    LOGF_UTILITY("stream=0x{:x}", refii::kernel::GetKernelHandle(stream));
    return stream->stream.eof() ? 1 : 0;
}
int __ferror(CFileHandle* stream)
{
    LOGF_UTILITY("stream=0x{:x}", refii::kernel::GetKernelHandle(stream));
    if (refii::kernel::IsInvalidKernelObject(stream))
    {
        LOGF_ERROR("Invalid stream: stream=0x{:x}", refii::kernel::GetKernelHandle(stream));
        return 1;
    }

    return (stream->stream.bad() || stream->stream.fail()) ? 1 : 0;
}
void __clearerr(CFileHandle* stream)
{
   
    if (!refii::kernel::IsInvalidKernelObject(stream))
    {
        LOGF_UTILITY("stream=0x{:x}", refii::kernel::GetKernelHandle(stream));
        stream->stream.clear();
    }
    else
    {
        LOGF_ERROR("Invalid stream: stream=0x{:x}", refii::kernel::GetKernelHandle(stream));
    }
}

GUEST_FUNCTION_HOOK(sub_82CC3490, XCreateFileA);
GUEST_FUNCTION_HOOK(sub_82CC7468, XFindFirstFileA);
GUEST_FUNCTION_HOOK(sub_82CC7410, XFindNextFileA);
GUEST_FUNCTION_HOOK(sub_82CC3BB0, XGetFileAttributesA);
GUEST_FUNCTION_HOOK(sub_82CC6E10, XGetFileSizeA);
GUEST_FUNCTION_HOOK(sub_82CC7CB8, XGetFileSizeExA);
GUEST_FUNCTION_HOOK(sub_82CC3688, XReadFile);
GUEST_FUNCTION_HOOK(sub_82CC7DD0, XReadFileEx);
GUEST_FUNCTION_HOOK(sub_82CC7938, XSetFilePointer);
//GUEST_FUNCTION_HOOK(sub_8272EF10, XSetFilePointerEx);
GUEST_FUNCTION_HOOK(sub_82CC6FB8, XWriteFile);
GUEST_FUNCTION_HOOK(sub_82CC2F10, XGetVolumeInformationA);

// XGetFilePhysicalSortKey is used to measure where files are located on disc to load them in an optimal order.
// for now just return 0.
DECLARE_STUB_FUNCTION_RETURN(uint32_t, XGetFilePhysicalSortKey, (FileHandle* hFile), 0);
GUEST_FUNCTION_HOOK(sub_82CC7BB8, XGetFilePhysicalSortKey);

// XMountUtilityDrive
GUEST_FUNCTION_STUB(sub_8248CB00);

// Native C functions (optional... can be stubbed as well - crack)
// function names are prefixed with "__" to avoid conflicts with existing crt functions
GUEST_FUNCTION_HOOK(sub_82CAA910, __fsopen);
GUEST_FUNCTION_HOOK(sub_82CC5E50, __fsclose);
GUEST_FUNCTION_HOOK(sub_82CAABE8, ___fclose);
GUEST_FUNCTION_HOOK(sub_82CB1B90, __fread);
GUEST_FUNCTION_HOOK(sub_82CAAF70, __fwrite);
GUEST_FUNCTION_HOOK(sub_82CAB548, __fseek);
GUEST_FUNCTION_HOOK(sub_82CAB880, __ftell);
GUEST_FUNCTION_HOOK(sub_83007E80, __feof);
GUEST_FUNCTION_HOOK(sub_83007ED8, __ferror);
GUEST_FUNCTION_HOOK(sub_83008158, __clearerr);
