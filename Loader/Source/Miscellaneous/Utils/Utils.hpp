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

    void Log(LogType type, const std::string& message);

    DWORD FindProcessId(const std::string& processName);
    std::unique_ptr<void, decltype(&CloseHandle)> OpenHandle(DWORD processId);

}