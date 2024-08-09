#include "EntryPoint.hpp"

constexpr const char* DLL_PATH = "./Fusion-CS2.dll";
constexpr const char* TARGET_PROCESS = "cs2.exe";

void Logo() {
    constexpr std::wstring_view DODGER_BLUE = L"\x1b[38;2;30;144;255m";
    constexpr std::wstring_view RESET = L"\x1b[0m";

    constexpr std::array logo{
        L"███████╗██╗░░░██╗░██████╗██╗░█████╗░███╗░░██╗",
        L"██╔════╝██║░░░██║██╔════╝██║██╔══██╗████╗░██║",
        L"█████╗░░██║░░░██║╚█████╗░██║██║░░██║██╔██╗██║",
        L"██╔══╝░░██║░░░██║░╚═══██╗██║██║░░██║██║╚████║",
        L"██║░░░░░╚██████╔╝██████╔╝██║╚█████╔╝██║░╚███║",
        L"╚═╝░░░░░░╚═════╝░╚═════╝░╚═╝░╚════╝░╚═╝░░╚══╝"
    };

    auto hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hConsole, &mode);
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hConsole, mode);

    for (const auto& line : logo) {
        WriteConsoleW(hConsole, DODGER_BLUE.data(), DODGER_BLUE.length(), nullptr, nullptr);
        WriteConsoleW(hConsole, line, wcslen(line), nullptr, nullptr);
        WriteConsoleW(hConsole, RESET.data(), RESET.length(), nullptr, nullptr);
        WriteConsoleW(hConsole, L"\n", 1, nullptr, nullptr);
    }

    std::cout << std::endl;
}

int main() {
    try {
        Logo();

        DWORD ProcessId = Utils::FindProcessId(TARGET_PROCESS);
        if (ProcessId == 0) {
            Utils::Log(Utils::LogType::ERR, "The target process was not found.");
        }

        auto ProcessHandle = Utils::OpenHandle(ProcessId);

        if (!Injection::ManualMap(ProcessHandle.get(), DLL_PATH)) {
            Utils::Log(Utils::LogType::ERR, "Mapping was unsuccessful.");
        }

        Utils::Log(Utils::LogType::SUCCESS, "Successfully loaded the cheat!");
        system("PAUSE");
    }
    catch (const std::exception& e) {
        Utils::Log(Utils::LogType::ERR, e.what());
        system("PAUSE");

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}