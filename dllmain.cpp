#include <iostream>
#include "lazy_importer.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_internal.h"
#include "xorstr.hpp"
#include <d3d11.h>
#pragma comment (lib, "d3d11.lib")


ID3D11RenderTargetView* rendertarget;
ID3D11DeviceContext* context;
ID3D11Device* device;
HWND hwnd;






typedef HRESULT(__fastcall* present_scene_t)(IDXGISwapChain* swapchain, UINT sync, UINT flags);

present_scene_t present_scene_original;

int X, Y;



bool ShowMenu = false;



HRESULT present_hooked(IDXGISwapChain* swapchain, UINT sync, UINT flags)
{
    if (!device)
    {
        ID3D11Texture2D* renderTarget = 0;
        ID3D11Texture2D* backBuffer = 0;
        D3D11_TEXTURE2D_DESC backBufferDesc = { 0 };

        swapchain->GetDevice(__uuidof(device), (PVOID*)&device);
        device->GetImmediateContext(&context);

        swapchain->GetBuffer(0, __uuidof(renderTarget), (PVOID*)&renderTarget);
        device->CreateRenderTargetView(renderTarget, nullptr, &rendertarget);
        renderTarget->Release();

        swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (PVOID*)&backBuffer);
        backBuffer->GetDesc(&backBufferDesc);

        X = backBufferDesc.Width;
        Y = backBufferDesc.Height;


        backBuffer->Release();

        if (!hwnd)
        {
            hwnd = FindWindowW(L"UnrealWindow", L"Fortnite  ");
        }
        ImGui_ImplDX11_Init(hwnd, device, context);
        ImGui_ImplDX11_CreateDeviceObjects();

    }
    context->OMSetRenderTargets(1, &rendertarget, nullptr);

    ImGui_ImplDX11_NewFrame();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::Begin("##scene", nullptr, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar);
    ImGuiIO& IO = ImGui::GetIO();
    ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetWindowSize(ImVec2(IO.DisplaySize.x, IO.DisplaySize.y), ImGuiCond_Always);
    ImGuiWindow& window = *ImGui::GetCurrentWindow();



    window.DrawList->AddText(ImVec2(50, 100), ImGui::GetColorU32(ImVec4(1.f, 0.f, 0.f, 1.f)), "OBS Hook");



    window.DrawList->PushClipRectFullScreen();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
    ImGui::Render();

    return present_scene_original(swapchain, sync, flags);
}

void hook() {
    std::uint64_t OBSHook = reinterpret_cast<std::uint64_t>(LI_FN(GetModuleHandleA).safe()("graphics-hook64.dll")); //version 24.0.3 only

    present_scene_original = *reinterpret_cast<present_scene_t*>(OBSHook + 0x8CC30); 


    *reinterpret_cast<present_scene_t*>(OBSHook + 0x8CC30) = present_hooked;
}




BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
        hook();

	return TRUE;
}