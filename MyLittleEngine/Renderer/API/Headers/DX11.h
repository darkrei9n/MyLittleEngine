#pragma once

#include "../../Headers/APIBase.h"

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class DX11Render : public APIBase
{
	HWND* hWnd;

public:

	DX11Render(HWND* hwnd) {
		hWnd = hwnd;
	}
};