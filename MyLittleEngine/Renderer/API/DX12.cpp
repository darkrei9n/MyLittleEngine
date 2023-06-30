#include "Headers/DX12.h"

void DX12Render::InitAPI(int width, int height)
{
	mWidth = width;
	mHeight = height;
	globalManager = GlobalManager::getInstance();
	//Enable Debug Layer
#if defined(debug) || defined(_DEBUG)
	{
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}
#endif
	HRESULT hr = (S_OK);
	hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory));
	if (FAILED(hr))
	{
		MessageBox(NULL, "Failed to create DXGIFactory", "Error", MB_OK);
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

	// Create descriptor heaps.
	{

		//Describe and create a Shader Resource View Heap
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = 1 * 10 * 3;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		hr = m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));

		if (FAILED(hr))
		{
			MessageBox(NULL, "Failed to create descriptor heap", "Error", MB_OK);
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
			MessageBox(NULL, "Failed to create descriptor heap", "Error", MB_OK);
			return;
		}

		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_cbsrvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	CreateCommandList();
	CreateSwapChain(width, height);
	CreateRenderTarget();
	CreateDepthStencil();
	
	
	OutputDebugString("DX DONE!");
}

void DX12Render::LoadAssetsFromFile(ASSETTYPE type, char* path)
{

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
		MessageBox(NULL, "Failed to find hardware adapter", "Error", MB_OK);
		return;
	}
}

void DX12Render::CreateRenderTarget()
{
	HRESULT hr;
	//Describe and create a Render Target View Heap
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FrameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));

	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	if (FAILED(hr))
	{
		MessageBox(NULL, "Failed to create descriptor heap", "Error", MB_OK);
		return;
	}

	auto rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

	// Create a RTV for each frame.
	for (UINT n = 0; n < FrameCount; ++n)
	{
		m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
		m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
		rtvHandle.ptr += m_rtvDescriptorSize;
	}

}

void DX12Render::CreateDepthStencil()
{
	HRESULT hr;

	//Describe and create a Depth Stencil View Heap
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	hr = m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap));
	if (FAILED(hr))
	{
		MessageBox(NULL, "Failed to create descriptor heap", "Error", MB_OK);
		return;
	}

	D3D12_RESOURCE_DESC depthStencilDesc = {};
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Height = mHeight;
	depthStencilDesc.Width = mWidth;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	hr = m_device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE, &depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&depthOptimizedClearValue, IID_PPV_ARGS(m_depthStencilBuffer.GetAddressOf())
	);
	if (FAILED(hr))
	{
		MessageBox(NULL, "Failed to create depth stencil", "Error", MB_OK);
		return;
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;
	depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	m_device->CreateDepthStencilView(m_depthStencil.Get(), &depthStencilViewDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	if (FAILED(hr))
	{
		MessageBox(NULL, "Failed to create depth stencil view", "Error", MB_OK);
		return;
	}
}

void DX12Render::CreateCommandList()
{
	HRESULT hr;

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	hr = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
	if (FAILED(hr))
	{
		MessageBox(NULL, "Failed to create command queue", "Error", MB_OK);
		return;
	}

	hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));

	if (FAILED(hr))
	{
		MessageBox(NULL, "Failed to create command allocator", "Error", MB_OK);
		return;
	}

	hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList));

	m_commandList->Close();
}

void DX12Render::CreateSwapChain(int width, int height )
{
	HRESULT hr;
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = *hWnd.get();
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
		hr = m_device->GetDeviceRemovedReason();
		MessageBox(NULL, "Failed to create swap chain", "Error", MB_OK);
		return;
	}

	hr = swapChain.As(&m_swapChain);
	if (FAILED(hr))
	{
		MessageBox(NULL, "Failed to cast swap chain", "Error", MB_OK);
		return;
	}

	hr = factory->MakeWindowAssociation(*hWnd, DXGI_MWA_NO_ALT_ENTER);

	if (FAILED(hr))
	{
		MessageBox(NULL, "Failed to make window association", "Error", MB_OK);
		return;
	}
	DXGI_RGBA test = { 0.0f, 0.0f, 0.0f, 0.0f };

	m_swapChain->SetBackgroundColor(&test);
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
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

	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get());
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &barrier);

	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	const float clearColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	auto rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	auto dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

	m_commandList->ClearRenderTargetView(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), clearColor, 0, nullptr);
	//m_commandList->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);



	m_commandList->OMSetRenderTargets(1, &rtvHandle, true, &dsvHandle);


	m_commandList->ResourceBarrier(1, &barrier);

	m_commandList->Close();

	ID3D12CommandList* cmdsList[] = {m_commandList.Get()};
	m_commandQueue->ExecuteCommandLists(_countof(cmdsList), cmdsList);
}

void DX12Render::End()
{

}