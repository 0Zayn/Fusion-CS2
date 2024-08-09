#include "Utils.hpp"

namespace Utils {
    void Log(LogType type, const std::string& message) {
        switch (type) {
        case LogType::INFO:    std::cout << "[INFO] ";    break;
        case LogType::WARNING: std::cout << "[WARNING] "; break;
        case LogType::ERR:     std::cout << "[ERROR] ";   break;
        case LogType::SUCCESS: std::cout << "[SUCCESS] "; break;
        }
        std::cout << message << std::endl;
    }

    DWORD FindProcessId(const std::string& processName) {
        PROCESSENTRY32 processEntry = { sizeof(PROCESSENTRY32) };
        auto snapshot = std::unique_ptr<void, decltype(&CloseHandle)>(
            CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0), CloseHandle);

        if (snapshot.get() == INVALID_HANDLE_VALUE) {
            throw std::runtime_error("CreateToolhelp32Snapshot failed: " + std::to_string(GetLastError()));
        }

        if (Process32First(snapshot.get(), &processEntry)) {
            do {
                if (processName == processEntry.szExeFile) {
                    return processEntry.th32ProcessID;
                }
            } while (Process32Next(snapshot.get(), &processEntry));
        }

        return 0;
    }

    std::unique_ptr<void, decltype(&CloseHandle)> OpenHandle(DWORD processId) {
        auto handle = std::unique_ptr<void, decltype(&CloseHandle)>(
            OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId), CloseHandle);

        if (!handle) {
            throw std::runtime_error("OpenProcess failed: " + std::to_string(GetLastError()));
        }

        return handle;
    }
}