#define WIN32_LEAN_AND_MEAN

#include <thread>
#include "Utility/Headers/Timer.h"
#include "Renderer/Headers/Renderer.h"
#include "Resource Management/Headers/ResourceManager.h"
#include <string>
#include <iostream>
GlobalManager globalManager;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) 
	{
	case WM_CLOSE:
		PostQuitMessage(68);
		break;
	case WM_KEYDOWN:
		if (wParam == 'D')
		{
			SetWindowText(hWnd, "Fack");
		}
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

int WinMain(
	HINSTANCE	hInstance,
	HINSTANCE	hPrevInstance,
	LPSTR		lpCmdLine,
	int			nCmdShow)

{
	WCHAR buffer[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, buffer, MAX_PATH);
	std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
	std::wstring path = std::wstring(buffer).substr(0, pos);


	const auto pClassName = "VulkanDX12 Test";
	const auto pClassNameDX12 = "My Little Engine Test DX12";
	const auto pClassNameVulkan = "My Little Engine Test Vulkan";

	//Register Window Class
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = nullptr;
	wc.hCursor = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = pClassName;

	RegisterClassEx(&wc);
	//Create Window Instance
	HWND hWnd = CreateWindowExA(
		0,
		pClassName,
		pClassName,
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
		0, 100, 1200, 600,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	HWND hWndDX12 = CreateWindowExA(
		0,
		pClassName,
		pClassNameDX12,
		WS_CHILD,
		0, 0, 600, 600,
		hWnd,
		nullptr,
		hInstance,
		nullptr);
	HWND hWndVulkan = CreateWindowExA(
		0,
		pClassName,
		pClassNameVulkan,
		WS_CHILD,
		600, 0, 600, 600,
		hWnd,
		nullptr,
		hInstance,
		nullptr);

	std::shared_ptr<HINSTANCE> hinstance = std::make_shared<HINSTANCE>(hInstance);
	//Show window
	ShowWindow(hWnd, SW_SHOW);
	ShowWindow(hWndDX12, SW_SHOW);
	ShowWindow(hWndVulkan, SW_SHOW);
	//Declare windows message variables
	MSG msg;
	BOOL gResult;

	//Start the timer
	Timer timer;
	timer.reset();
	globalManager = GlobalManager();

	//Initialize Renderer
	Renderer renderdx12 = Renderer(std::make_shared<HWND>(hWndDX12), hinstance, std::make_shared<GlobalManager>(globalManager));
	Renderer rendervulkan = Renderer(std::make_shared<HWND>(hWndVulkan), hinstance, std::make_shared<GlobalManager>(globalManager));
	//Setup DX12 Thread and Vulkan Thread and begin initialization.
	std::thread dx12Thread(&Renderer::initialize, &renderdx12, RENDERAPI::DX12, 960, 480);
	std::thread vulkanThread(&Renderer::initialize, &rendervulkan, RENDERAPI::VULKAN, 960, 480);
	
	//Wait for threads to finish initialization.
	dx12Thread.join();
	vulkanThread.join();

	//Main Loop.
	while ((gResult = GetMessage(&msg, nullptr, 0, 0)) > 0) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		timer.tick();
		renderdx12.Render();
		rendervulkan.Render();
	}

	if (gResult == -1)
	{
		//renderdx12.End();
		//rendervulkan.End();
		return -1;
	}
	else
	{
		return msg.wParam;
	}
}