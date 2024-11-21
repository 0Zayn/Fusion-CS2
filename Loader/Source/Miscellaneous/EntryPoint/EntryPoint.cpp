#include "EntryPoint.hpp"

void PrintLogo() {
    constexpr std::wstring_view DODGER_BLUE = L"\x1b[38;2;30;144;255m";
    constexpr std::wstring_view RESET = L"\x1b[0m";

    constexpr std::array Logo{
        L"███████╗██╗░░░██╗░██████╗██╗░█████╗░███╗░░██╗",
        L"██╔════╝██║░░░██║██╔════╝██║██╔══██╗████╗░██║",
        L"█████╗░░██║░░░██║╚█████╗░██║██║░░██║██╔██╗██║",
        L"██╔══╝░░██║░░░██║░╚═══██╗██║██║░░██║██║╚████║",
        L"██║░░░░░╚██████╔╝██████╔╝██║╚█████╔╝██║░╚███║",
        L"╚═╝░░░░░░╚═════╝░╚═════╝░╚═╝░╚════╝░╚═╝░░╚══╝"
    };

    auto Console = GetStdHandle(STD_OUTPUT_HANDLE);
    
    DWORD Mode;
    GetConsoleMode(Console, &Mode);
    
    Mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(Console, Mode);

    for (const auto& Line : Logo) {
        WriteConsoleW(Console, DODGER_BLUE.data(), DODGER_BLUE.length(), nullptr, nullptr);
        WriteConsoleW(Console, Line, wcslen(Line), nullptr, nullptr);
        WriteConsoleW(Console, RESET.data(), RESET.length(), nullptr, nullptr);
        WriteConsoleW(Console, L"\n", 1, nullptr, nullptr);
    }

    std::cout << std::endl;
}

int main() {
    try {
        PrintLogo();

        DWORD ProcessId = Utils::FindProcessId("cs2.exe");
        if (ProcessId == 0) 
            Utils::Log(Utils::LogType::ERR, "The target process was not found.");

        auto Handle = Utils::OpenHandle(ProcessId);

        if (!Injector->ManualMap(Handle.get(), "./Fusion-CS2.dll"))
            Utils::Log(Utils::LogType::ERR, "Mapping was unsuccessful.");

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
