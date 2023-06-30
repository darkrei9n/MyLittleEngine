#define WIN32_LEAN_AND_MEAN

#include <thread>
#include "Renderer/Headers/Renderer.h"
#include <string>
#include <iostream>

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	GlobalManager *globalManager = &GlobalManager::getInstance();

	switch (msg) 
	{
	case WM_CLOSE:
		PostQuitMessage(68);
		break;
	case WM_ACTIVATE: 
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			globalManager->getTimer()->pauseTimer();
		}
		else
		{
			globalManager->getTimer()->startTimer();
		}
	case WM_KEYDOWN:
		if (wParam == 'D')
		{
			globalManager->getCamera()->translate(Vector3(0.1f, 0.0f, 0.0f));
		}
		if (wParam == 'A')
		{
			globalManager->getCamera()->translate(Vector3(-0.1f, 0.0f, 0.0f));
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
	GlobalManager *globalManager = &GlobalManager::getInstance();

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
	Timer* timer = globalManager->getTimer();
	
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)&globalManager);
	

	//Initialize Renderer
	Renderer renderdx12 = Renderer(std::make_shared<HWND>(hWndDX12), hinstance);
	Renderer rendervulkan = Renderer(std::make_shared<HWND>(hWndVulkan), hinstance);
	//Setup DX12 Thread and Vulkan Thread and begin initialization.
	std::thread dx12Thread(&Renderer::initialize, &renderdx12, RENDERAPI::DX12, 600, 600);
	std::thread vulkanThread(&Renderer::initialize, &rendervulkan, RENDERAPI::VULKAN, 600, 600);
	
	//Wait for threads to finish initialization.
	dx12Thread.join();
	vulkanThread.join();

	//Main Loop.
	while ((gResult = GetMessage(&msg, nullptr, 0, 0)) > 0) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		/*if(GetAsyncKeyState('A') & 0x8000)
		{
			globalManagerPtr->getCamera()->translate(Vector3(-0.1f, 0.0f, 0.0f));
		}
		if(GetAsyncKeyState('D') & 0x8000)
		{
			globalManagerPtr->getCamera()->translate(Vector3(0.1f, 0.0f, 0.0f));
		}*/
	/*	if(GetAsyncKeyState('W') & 0x8000)
		{
			globalManagerPtr->getCamera()->translate(Vector3(0.0f, 0.0f, -0.1f));
		}
		if(GetAsyncKeyState('S') & 0x8000)
		{
			globalManagerPtr->getCamera()->translate(Vector3(0.0f, 0.0f, 0.1f));
		}*/

		renderdx12.Update();

		timer->tick();
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