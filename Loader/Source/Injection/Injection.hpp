#pragma once

#include <Windows.h>
#include <winternl.h>
#include <fstream>
#include <vector>

class CInjector {
public:
    bool ManualMap(HANDLE Process, const std::string& DllPath);

    using LoadLibraryFunc = HINSTANCE(WINAPI*)(const char* LibFileName);
    using GetProcAddressFunc = FARPROC(WINAPI*)(HMODULE Module, const char* ProcName);
    using DllEntryPointFunc = BOOL(WINAPI*)(void* Module, DWORD Reason, void* Reserved);

    struct MappingData {
        LoadLibraryFunc LoadLibraryA;
        GetProcAddressFunc GetProcAddress;
        HINSTANCE ModuleHandle;
    };

private:
    static void WINAPI Shellcode(LPVOID DataPtr);
};

inline auto Injector = std::make_unique<CInjector>();