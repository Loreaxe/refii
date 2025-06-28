#pragma once

#define NOMINMAX

#if defined(_WIN32)
#include <windows.h>
#include <ShlObj_core.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;
#elif defined(__linux__)
#include <unistd.h>
#include <pwd.h>
#endif

#ifdef UNLEASHED_RECOMP_D3D12
#include <dxcapi.h>
#endif

#include <algorithm>
#include <mutex>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <cassert>
#include <chrono>
#include <span>
#include <xbox.h>
#include <xxhash.h>
#include <ankerl/unordered_dense.h>
#include <ddspp.h>
#include <ppc/ppc_recomp_shared.h>
#include <toml++/toml.hpp>
#include <zstd.h>
#include <stb_image.h>
#include <blockingconcurrentqueue.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>
#include <backends/imgui_impl_sdl2.h>
#include <o1heap.h>
#include <cstddef>
#include <smolv.h>
#include <set>
#include <fmt/core.h>
#include <list>
#include <semaphore>
#include <numeric>
#include <charconv>

#include "framework.h"
#include <kernel/obj/mutex.h>

#include <os/logger.h>

#ifndef _WIN32
#include <sys/mman.h>
#endif

#ifdef _WIN32
#include <ntstatus.h>
#endif

#define DECLARE_STUB_FUNCTION(return_type, function_name, params) \
    return_type function_name params \
    { \
        LOG_UTILITY("===STUB==="); \
    }

#define DECLARE_STUB_FUNCTION_VOID(function_name, params)    \
    DECLARE_STUB_FUNCTION(void, function_name, params)

#define DECLARE_STUB_FUNCTION_RETURN(return_type, function_name, params, return_value) \
    return_type function_name params \
    { \
        LOG_UTILITY("===STUB==="); \
        return return_value; \
    }

#define DECLARE_STUB_FUNCTION_DEFAULT(return_type, function_name, params) \
    return_type function_name params \
    { \
        LOG_UTILITY("===STUB==="); \
        return return_type{}; \
    }

#define DECLARE_STUB_FUNCTION_TRAP(function_name, params) \
    void function_name params \
    { \
        LOG_UTILITY("===STUB==="); \
        __builtin_debugtrap(); \
    }
