/*

Interface.cpp (Hooks DX11 to render an interface + visuals)
Authors: 0Zayn (Zayn)

*/

#include "Interface.hpp"

namespace Interface {
    std::unique_ptr<CRenderer> Renderer = std::make_unique<CRenderer>();

    CRenderer::CRenderer() noexcept = default;
    CRenderer::~CRenderer() noexcept {
        Shutdown();
    }

    void CRenderer::Initialize() noexcept {
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

        ImGui_ImplWin32_Init(Window);
        ImGui_ImplDX11_Init(Device.Get(), Context.Get());

        SetStyle();
    }

    LRESULT CRenderer::HandleWindowProc(HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam) noexcept {
        if (Initialized && ImGui_ImplWin32_WndProcHandler(Hwnd, Msg, WParam, LParam))
            return true;

        if (GetAsyncKeyState(VK_INSERT) & 1)
            Globals::Interface::Open = !Globals::Interface::Open;

        return CallWindowProc(OriginalWndProc, Hwnd, Msg, WParam, LParam);
    }

    HRESULT CRenderer::HandlePresent(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags) noexcept {
        if (!Initialized) {
            if (SUCCEEDED(SwapChain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(Device.GetAddressOf())))) {
                Device->GetImmediateContext(Context.GetAddressOf());

                DXGI_SWAP_CHAIN_DESC Sd;
                SwapChain->GetDesc(&Sd);
                Window = Sd.OutputWindow;

                Microsoft::WRL::ComPtr<ID3D11Texture2D> BackBuffer;

                SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(BackBuffer.GetAddressOf()));
                Device->CreateRenderTargetView(BackBuffer.Get(), nullptr, RenderView.GetAddressOf());

                OriginalWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(Window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(static_cast<WNDPROC>([](HWND Hwnd, UINT Msg, WPARAM WParam, LPARAM LParam) -> LRESULT { return Renderer->HandleWindowProc(Hwnd, Msg, WParam, LParam); }))));

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

    void CRenderer::SetStyle() noexcept {
        ImGuiStyle& Style = ImGui::GetStyle();

        Style.WindowRounding = 8.0f;

        Style.ChildRounding = 8.0f;
        Style.FrameRounding = 6.0f;

        Style.GrabRounding = 4.0f;
        Style.PopupRounding = 4.0f;

        Style.ScrollbarRounding = 4.0f;
        Style.TabRounding = 4.0f;

        ImVec4* Colors = Style.Colors;

        Colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.12f, 0.94f);
        Colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.28f, 0.50f);
        Colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);
        Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.24f, 1.00f);
        Colors[ImGuiCol_FrameBgActive] = ImVec4(0.28f, 0.28f, 0.30f, 1.00f);
        Colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.17f, 1.00f);
        Colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
        Colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
        Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
        Colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
        Colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.28f, 0.30f, 1.00f);
        Colors[ImGuiCol_ButtonActive] = ImVec4(0.33f, 0.33f, 0.35f, 1.00f);
        Colors[ImGuiCol_Header] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
        Colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.25f, 0.27f, 1.00f);
        Colors[ImGuiCol_HeaderActive] = ImVec4(0.30f, 0.30f, 0.32f, 1.00f);
    }

    void CRenderer::RenderFrame() noexcept {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();

        ImGui::NewFrame();

        if (Globals::ESP::Enabled)
            Visuals::Players();

        if (Globals::Interface::Open)
            RenderMenu();

        ImGui::EndFrame();
        ImGui::Render();
        Context->OMSetRenderTargets(1, RenderView.GetAddressOf(), nullptr);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    void CRenderer::RenderMenu() noexcept {
        ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
        ImGui::Begin("Fusion: CS2", &Globals::Interface::Open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

        ImGui::TextColored(ImVec4(0.28f, 0.56f, 1.00f, 1.00f), "Fusion CS2 - Internal Cheat");
        ImGui::Separator();

        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        static int CurrentTab = 0;
        const char* Tabs[] = { "Visuals", "Combat" };

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(15, 20));

        for (int i = 0; i < IM_ARRAYSIZE(Tabs); ++i) {
            if (i > 0)
                ImGui::SameLine();

            if (ImGui::Button(Tabs[i], ImVec2(ImGui::GetWindowWidth() / IM_ARRAYSIZE(Tabs) - 20, 25)))
                CurrentTab = i;
        }

        ImGui::PopStyleVar();
        ImGui::Spacing();
        ImGui::Separator();

        if (CurrentTab == 0) { // Visuals
            ImGui::BeginChild("Visuals", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
            ImGui::Text("ESP");
            ImGui::Separator();

            ImGui::Checkbox("Enabled", &Globals::ESP::Enabled);

            ImGui::Checkbox("Box", &Globals::ESP::Box);
            ImGui::Checkbox("Name", &Globals::ESP::Name);

            ImGui::Checkbox("Health", &Globals::ESP::Health);
            ImGui::Checkbox("Health Text", &Globals::ESP::HealthText);

            ImGui::Text("Other");
            ImGui::Checkbox("Draw FOV", &Globals::Aimbot::DrawFOV);

            ImGui::EndChild();
        }
        else if (CurrentTab == 1) { // Combat
            ImGui::BeginChild("Combat", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
            
            ImGui::Text("Misc");
            ImGui::Separator();

            ImGui::Checkbox("Team Check", &Globals::Misc::TeamCheck);

            ImGui::Text("Aimbot");
            ImGui::Separator();

            ImGui::Checkbox("Enabled", &Globals::Aimbot::Enabled);
            ImGui::Checkbox("Sticky Aim", &Globals::Aimbot::StickyAim);

            ImGui::Separator();
            ImGui::SliderFloat("Smoothing", &Globals::Aimbot::Smoothing, 0.1f, 1.0f, "%.2f");
            ImGui::SliderFloat("FOV", &Globals::Aimbot::FOV, 1.0f, 500.0f, "%.1f");

            ImGui::EndChild();
        }

        ImGui::End();
    }

    void CRenderer::Shutdown() noexcept {
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