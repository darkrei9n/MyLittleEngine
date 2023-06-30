#pragma once

#include "../../Headers/APIBase.h"
#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_6.h>
#include <D3Dcompiler.h>

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

//using namespace DirectX;
using namespace Microsoft::WRL;

class DX12Render: public APIBase
{
    GlobalManager globalManager;
    static const UINT FrameCount = 3;
    //Cause Im lazy
    int mHeight;
    int mWidth;
    //Initialization Objects
    ComPtr<ID3D12Device> m_device;
    ComPtr<IDXGIFactory7> factory;
    //Pipeline objects.
    D3D12_VIEWPORT m_viewport;
    D3D12_RECT m_scissorRect;
    ComPtr<IDXGISwapChain3> m_swapChain;
    
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    ComPtr<ID3D12DescriptorHeap> m_srvHeap;
    ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
    ComPtr<ID3D12PipelineState> m_pipelineState;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    UINT m_rtvDescriptorSize;
    UINT m_cbsrvDescriptorSize;

    //App Resources
    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    //Synchronization objects.
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;

    //Ol Depth Stencil
    ComPtr<ID3D12Resource> m_depthStencil;
    ComPtr<ID3D12Resource> m_depthStencilBuffer;

    std::shared_ptr<HWND> hWnd;
    std::shared_ptr<HINSTANCE> hInstance;

    void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);
    void CreateRenderTarget();
    void CreateDepthStencil();
    void CreateCommandList();
    void CreateSwapChain(int width, int height);
public:

    DX12Render(std::shared_ptr<HWND> hWnd, std::shared_ptr<HINSTANCE> hInstance)
    {
        this->hWnd = hWnd;
        this->hInstance = hInstance;
    }
	~DX12Render() = default;

	void InitAPI(int width, int height) override;
    void LoadAssetsFromFile(ASSETTYPE type, char* path) override;
    void Render() override;
    void Update() override;
    void End() override;

protected:
    void PopulateCommandList();
};