#pragma once

#include "../API/Headers/DX12.h"
#include "../API/Headers/DX11.h"
#include"../API/Headers/Vulkan.h"
#include "APIBase.h"

enum RENDERAPI
{
	DX12 = 0,
	DX11 = 1,
	VULKAN = 2
};


class Renderer
{
public:
	Renderer(std::shared_ptr<HWND> hWnd, std::shared_ptr<HINSTANCE> hInstance, std::shared_ptr<GlobalManager> globalManager)
	{
		hwnd = hWnd;
		hinstance = hInstance;
		handle = nullptr;
		this->globalManager = globalManager;
	}
	int initialize(RENDERAPI api, int width, int height);
	void Render();
	void Update();
	void End();
private:
	std::shared_ptr<HWND> hwnd;
	std::shared_ptr<HINSTANCE> hinstance;
	std::shared_ptr<GlobalManager> globalManager;
	APIBase* handle;

	
};