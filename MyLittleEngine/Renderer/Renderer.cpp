#include "Headers/Renderer.h"


int Renderer::initialize(RENDERAPI api, int width, int height)
{
	switch (api)
	{
	case DX12:
		handle = new DX12Render(hwnd, hinstance);
		break;
	case DX11:
		//handle = new DX11Render(hwnd);
		break;
	case VULKAN:
		handle = new VulkanRender(hwnd, hinstance);
		break;
	default:
		return -1;
	}
	handle->InitAPI(width, height);

	return 0;
}

void Renderer::Render()
{
	handle->Render();
}

void Renderer::Update()
{
	handle->Update();
}