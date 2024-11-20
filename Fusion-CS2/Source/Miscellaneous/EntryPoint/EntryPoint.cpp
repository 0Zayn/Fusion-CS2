/*

EntryPoint.cpp (Basic internal aimbot, and ESP for CS2)
Authors: 0Zayn (Zayn)

*/

#include "EntryPoint.hpp"

DWORD WINAPI Setup(LPVOID Reversed) {
    bool InitializedHook = false;

    do {
        if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success) {
            kiero::bind(8, reinterpret_cast<void**>(&Interface::Renderer->OriginalPresent),
                reinterpret_cast<void*>(static_cast<HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT)>(
                    [](IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags) -> HRESULT {
                        return Interface::Renderer->HandlePresent(SwapChain, SyncInterval, Flags);
                    }
                ))
            );

            InitializedHook = true;
        }
    } while (!InitializedHook);

    std::thread([] {
        while (true) {
            Aimbot->Run();
            std::this_thread::sleep_for(std::chrono::microseconds(5));
        }
    }).detach();

    return TRUE;
}

BOOL WINAPI DllMain(HMODULE Module, DWORD Reason, LPVOID Reserved) {
    switch (Reason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(Module);
        CreateThread(nullptr, 0, Setup, Module, 0, nullptr);
        break;
    case DLL_PROCESS_DETACH:
        Interface::Renderer->Shutdown();
        kiero::shutdown();
        break;
    }

    return TRUE;
}