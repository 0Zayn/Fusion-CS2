#include "EntryPoint.hpp"

constexpr const char* DLL_PATH = "./Fusion-CS2.dll";
constexpr const char* TARGET_PROCESS = "cs2.exe";

int main() {
    try {
        DWORD ProcessId = Utils::FindProcessId(TARGET_PROCESS);
        if (ProcessId == 0) {
            Utils::Log(Utils::LogType::ERR, "The target process was not found.");
        }

        auto ProcessHandle = Utils::OpenHandle(ProcessId);

        if (!Injection::ManualMap(ProcessHandle.get(), DLL_PATH)) {
            Utils::Log(Utils::LogType::SUCCESS, "Mapping was unsuccessful.");
        }

        Utils::Log(Utils::LogType::SUCCESS, "Successfully mapped the Cheat into CS2!");
    }
    catch (const std::exception& e) {
        Utils::Log(Utils::LogType::ERR, e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}