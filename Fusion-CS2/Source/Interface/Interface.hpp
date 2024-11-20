/*

Interface.hpp (Hooks DX11 to render an interface + visuals)
Authors: 0Zayn (Zayn)

*/

#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dwmapi.h>
#include <memory>
#include <atomic>
#include <wrl/client.h>
#include <mutex>

#include <Visuals/Visuals.hpp>
#include <Globals.hpp>

#include <Kiero/kiero.h>
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_win32.h>
#include <ImGui/imgui_impl_dx11.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

using Present = HRESULT(__stdcall*)(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags);
using WNDPROC = LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM);

namespace Interface {
    class CRenderer {
    public:
        CRenderer() noexcept;
        ~CRenderer() noexcept;

        void Initialize() noexcept;

        LRESULT HandleWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;
        HRESULT HandlePresent(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags) noexcept;

        Present OriginalPresent = nullptr;
        WNDPROC OriginalWndProc = nullptr;
        HWND Window = nullptr;

        void Shutdown() noexcept;

    private:
        void SetStyle() noexcept;

        void RenderFrame() noexcept;
        void RenderMenu() noexcept;

        Microsoft::WRL::ComPtr<ID3D11Device> Device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> Context;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RenderView;

        std::atomic<bool> Initialized{ false };
        std::mutex RenderMutex;
    };

    extern std::unique_ptr<CRenderer> Renderer;
}