/*

Interface.cpp (Hooks DX11 to render an interface + visuals)
Authors: 0Zayn (Zayn)

*/

#include "Interface.hpp"

namespace Interface {
    std::unique_ptr<Renderer> g_Renderer = std::make_unique<Renderer>();

    Renderer::Renderer() noexcept = default;
    Renderer::~Renderer() noexcept {
        Shutdown();
    }

    void Renderer::Initialize() noexcept {
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

        ImGui_ImplWin32_Init(Window);
        ImGui_ImplDX11_Init(Device.Get(), Context.Get());
    }

    LRESULT Renderer::HandleWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept {
        if (Initialized && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
            return true;

        if (GetAsyncKeyState(VK_INSERT) & 1)
            Globals::Interface::Open = !Globals::Interface::Open;

        return CallWindowProc(OriginalWndProc, hWnd, uMsg, wParam, lParam);
    }

    HRESULT Renderer::HandlePresent(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags) noexcept {
        if (!Initialized) {
            if (SUCCEEDED(SwapChain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(Device.GetAddressOf())))) {
                Device->GetImmediateContext(Context.GetAddressOf());

                DXGI_SWAP_CHAIN_DESC sd;
                SwapChain->GetDesc(&sd);
                Window = sd.OutputWindow;

                Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
                SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
                Device->CreateRenderTargetView(backBuffer.Get(), nullptr, RenderView.GetAddressOf());

                OriginalWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(Window, GWLP_WNDPROC,
                    reinterpret_cast<LONG_PTR>(static_cast<WNDPROC>([](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
                        -> LRESULT { return g_Renderer->HandleWindowProc(hWnd, uMsg, wParam, lParam); }))));

                Initialize();
                Initialized = true;
            }
            else return OriginalPresent(SwapChain, SyncInterval, Flags);
        }

        if (GetAsyncKeyState(VK_END) & 1) {
            Shutdown();
            return OriginalPresent(SwapChain, SyncInterval, Flags);
        }

        {
            std::lock_guard<std::mutex> lock(RenderMutex);
            RenderFrame();
        }

        return OriginalPresent(SwapChain, SyncInterval, Flags);
    }

    void Renderer::RenderFrame() noexcept {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (Globals::Interface::Open)
            RenderMenu();

        if (Globals::ESP::Enabled)
            Visuals::Players();

        ImGui::EndFrame();
        ImGui::Render();
        Context->OMSetRenderTargets(1, RenderView.GetAddressOf(), nullptr);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    void Renderer::RenderMenu() noexcept {
        ImGuiStyle& Style = ImGui::GetStyle();
        Style.WindowRounding = 8.0f;
        Style.ChildRounding = 8.0f;
        Style.FrameRounding = 6.0f;
        Style.GrabRounding = 4.0f;
        Style.PopupRounding = 4.0f;
        Style.ScrollbarRounding = 4.0f;
        Style.TabRounding = 4.0f;

        ImVec4* colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.12f, 0.94f);
        colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.28f, 0.50f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.24f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.28f, 0.28f, 0.30f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.17f, 1.00f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.28f, 0.30f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.33f, 0.33f, 0.35f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.25f, 0.27f, 1.00f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.30f, 0.30f, 0.32f, 1.00f);

        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
        ImGui::Begin("Fusion: CS2", &Globals::Interface::Open, ImGuiWindowFlags_NoCollapse);

        ImGui::TextColored(ImVec4(0.28f, 0.56f, 1.00f, 1.00f), "Internal CS2 Cheat");

        ImGui::Separator();

        static int CurrentTab = 0;
        const char* Tabs[] = { "ESP", "Aimbot" };
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 10));

        for (int i = 0; i < IM_ARRAYSIZE(Tabs); i++) {
            if (i > 0) ImGui::SameLine();
            if (ImGui::Button(Tabs[i], ImVec2(ImGui::GetWindowWidth() * 0.5f - 12, 0)))
                CurrentTab = i;
            if (CurrentTab == i)
                ImGui::PushStyleColor(ImGuiCol_Button, colors[ImGuiCol_ButtonActive]);
            else
                ImGui::PushStyleColor(ImGuiCol_Button, colors[ImGuiCol_Button]);
            ImGui::PopStyleColor(1);
        }

        ImGui::PopStyleVar();

        ImGui::Spacing();

        switch (CurrentTab) {
        case 0: // ESP
            ImGui::BeginChild("ESP Settings", ImVec2(0, 0), true);

            ImGui::Checkbox("Enabled", &Globals::ESP::Enabled);

            ImGui::Checkbox("Box", &Globals::ESP::Box);

            ImGui::Checkbox("Health", &Globals::ESP::Health);
            ImGui::Checkbox("Health Text", &Globals::ESP::HealthText);

            ImGui::EndChild();
            break;
        case 1: // Aimbot
            ImGui::BeginChild("Aimbot Settings", ImVec2(0, 0), true);
            ImGui::Checkbox("Enabled", &Globals::Aimbot::Enabled);

            ImGui::Checkbox("Sticky Aim", &Globals::Aimbot::StickyAim);

            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.20f, 0.20f, 0.22f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.25f, 0.25f, 0.27f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.30f, 0.30f, 0.32f, 1.00f));

            ImGui::SliderFloat("Smoothing", &Globals::Aimbot::Smoothing, 0.1f, 1.0f, "%.2f");

            ImGui::PopStyleColor(3);
            ImGui::EndChild();
            break;
        }

        ImGui::End();
    }

    void Renderer::Shutdown() noexcept {
        if (!Initialized) return;

        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        if (Window)
            SetWindowLongPtr(Window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(OriginalWndProc));

        RenderView.Reset();
        Context.Reset();
        Device.Reset();

        Initialized = false;
    }
}