#pragma once

#include <Windows.h>
#include <string>
#include <fstream>
#include <vector>

#include <Utils/Utils.hpp>

namespace Injection {
    using LoadLibraryAType = HINSTANCE(WINAPI*)(const char* lpLibFileName);
    using GetProcAddressType = FARPROC(WINAPI*)(HMODULE hModule, const char* lpProcName);
    using DllEntryPointType = BOOL(WINAPI*)(void* hDll, DWORD dwReason, void* pReserved);

    struct ManualMappingData {
        LoadLibraryAType LoadLibraryA;
        GetProcAddressType GetProcAddress;
        HINSTANCE ModuleHandle;
    };

    bool ManualMap(HANDLE TargetProcess, const std::string& DllPath);
}