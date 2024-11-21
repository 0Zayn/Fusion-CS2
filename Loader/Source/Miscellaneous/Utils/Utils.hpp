#pragma once

#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <memory>
#include <iostream>

namespace Utils {
    enum class LogType {
        INFO,
        WARNING,
        ERR,
        SUCCESS
    };

    void Log(LogType type, const std::string& Message);

    DWORD FindProcessId(const std::string& Name);
    std::unique_ptr<void, decltype(&CloseHandle)> OpenHandle(DWORD ProcessId);
}