#pragma once

#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <vector>

namespace Utils
{
    HANDLE OpenTargetProc(const std::wstring& ProcessName);
    bool ReadFile(const std::wstring& FilePath, std::vector<BYTE>& Buffer);
}