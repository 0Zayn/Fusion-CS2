#include "Utils.hpp"

namespace Utils {
    void Log(LogType type, const std::string& Message) {
        switch (type) {
        case LogType::INFO:    std::cout << "[INFO] ";    break;
        case LogType::WARNING: std::cout << "[WARNING] "; break;
        case LogType::ERR:     std::cout << "[ERROR] ";   break;
        case LogType::SUCCESS: std::cout << "[SUCCESS] "; break;
        }
        std::cout << Message << std::endl;
    }

    DWORD FindProcessId(const std::string& Name) {
        PROCESSENTRY32 Entry = { sizeof(PROCESSENTRY32) };
        
        auto Snapshot = std::unique_ptr<void, decltype(&CloseHandle)>(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0), CloseHandle);
        if (Snapshot.get() == INVALID_HANDLE_VALUE) 
            throw std::runtime_error("CreateToolhelp32Snapshot failed: " + std::to_string(GetLastError()));

        if (Process32First(Snapshot.get(), &Entry)) {
            do {
                if (Name == Entry.szExeFile) 
                    return Entry.th32ProcessID;
            } while (Process32Next(Snapshot.get(), &Entry));
        }

        return 0;
    }

    std::unique_ptr<void, decltype(&CloseHandle)> OpenHandle(DWORD ProcessId) {
        auto Handle = std::unique_ptr<void, decltype(&CloseHandle)>(OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId), CloseHandle);

        if (!Handle) 
            throw std::runtime_error("OpenProcess failed: " + std::to_string(GetLastError()));

        return Handle;
    }
}
