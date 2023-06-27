#include "Headers/DX12.h"

void DX12Render::InitAPI(int width, int height)
{
#if defined(debug) || defined(_DEBUG)
	// Enable the D3D12 debug layer.
	{
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}
#endif
	HRESULT hr = (S_OK);
	hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
	if (FAILED(hr))
	{
		MessageBox(*hWnd, "Failed to create DXGIFactory", "Error", MB_OK);
		return;
	}
	//Create Device
	ComPtr<IDXGIAdapter1> hardwareAdapter;
	GetHardwareAdapter(factory.Get(), &hardwareAdapter);

	hr = D3D12CreateDevice(
		hardwareAdapter.Get(),
		D3D_FEATURE_LEVEL_12_0,
		IID_PPV_ARGS(&m_device)
	);

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	hr = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
	if (FAILED(hr))
	{
		MessageBox(*hWnd, "Failed to create command queue", "Error", MB_OK);
		return;
	}

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = *hWnd;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = true;

	ComPtr<IDXGISwapChain> swapChain;
	hr = factory->CreateSwapChain(
		m_commandQueue.Get(),
		&swapChainDesc,
		&swapChain
	);

	if (FAILED(hr))
	{
		MessageBox(*hWnd, "Failed to create swap chain", "Error", MB_OK);
		return;
	}

	hr = swapChain.As(&m_swapChain);
	if (FAILED(hr))
	{
		MessageBox(*hWnd, "Failed to cast swap chain", "Error", MB_OK);
		return;
	}
	
	hr = factory->MakeWindowAssociation(*hWnd, DXGI_MWA_NO_ALT_ENTER);

	if (FAILED(hr))
	{
		MessageBox(*hWnd, "Failed to make window association", "Error", MB_OK);
		return;
	}
	DXGI_RGBA test = { 0.0f, 0.0f, 0.0f, 0.0f };

	m_swapChain->SetBackgroundColor(&test);
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// Create descriptor heaps.
	{
		//Describe and create a Render Target View Heap
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		hr = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));

		if (FAILED(hr))
		{
			MessageBox(*hWnd, "Failed to create descriptor heap", "Error", MB_OK);
			return;
		}

		//Describe and create a Depth Stencil View Heap
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		hr = m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap));

		if (FAILED(hr))
		{
			MessageBox(*hWnd, "Failed to create descriptor heap", "Error", MB_OK);
			return;
		}

		//Describe and create a Shader Resource View Heap
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = 1 * 10 * 3;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		hr = m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));

		if (FAILED(hr))
		{
			MessageBox(*hWnd, "Failed to create descriptor heap", "Error", MB_OK);
			return;
		}

		//Describe and create a Constant Buffer View Heap
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
		cbvHeapDesc.NumDescriptors = 1;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		hr = m_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap));

		if (FAILED(hr))
		{
			MessageBox(*hWnd, "Failed to create descriptor heap", "Error", MB_OK);
			return;
		}

		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_cbsrvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
	
	if (FAILED(hr))
	{
		MessageBox(*hWnd, "Failed to create command allocator", "Error", MB_OK);
		return;
	}
	OutputDebugString("DX DONE!");
}

void DX12Render::GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter)
{
	ComPtr<IDXGIAdapter1> adapter;
	ComPtr<IDXGIFactory6> factory;
	UINT adapterIndex = 0;
	bool adapterFound = false;

	if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
	{
		while (factory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				adapterIndex++;
				continue;
			}
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				adapterFound = true;
				break;
			}
			adapterIndex++;
		}
	}
	if (!adapterFound)
	{
		MessageBox(*hWnd, "Failed to find hardware adapter", "Error", MB_OK);
		return;
	}
}

void DX12Render::Render()
{
	m_swapChain->Present(1, 0);
}

void DX12Render::Update()
{
	PopulateCommandList();
}

void DX12Render::PopulateCommandList()
{
	
	/*m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get());

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_commandList->ClearRenderTargetView(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), clearColor, 0, nullptr);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	m_commandList->Close();*/
}

void DX12Render::End()
{

}