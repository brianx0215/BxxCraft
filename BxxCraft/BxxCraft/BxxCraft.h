#pragma once

#include "BxxApplication.h"
#include "Mesh.h"

class BxxCraft : public BxxApplication {
public:
	BxxCraft(UINT width, UINT height, std::wstring name, std::wstring title);
	// Inherited via BxxApplication
	virtual void OnInit() override;
	virtual void OnUpdate() override;
	virtual void OnRender() override;
	virtual void OnDestory() override;

private:
	void LoadPipeline();
	void LoadAssets();
	void PopulateCommandList();
	void WaitForPreviousFrame();

	////Subfunctions in LoadPipeLine()
	//void CreateFactory();
	//void CreateDevice();
	//void CreateCommandQueue();
	//void CreateSwapChain();
	//void CreateDescriptorObjects();

	////Subfunctions in LoadAssets()
	//void CreateRootSignature();
	//void CreateShaders();
	//void CreatePSO();
	//void CreateCommandList();
	//void LoadScene();
	//void CreateSynchronizationObjects();

	static const UINT FrameCount = 2;

	//Pipeline objects
	UINT m_rtvDescriptorSize;
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator[FrameCount];
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	Microsoft::WRL::ComPtr<IDXGIFactory4> m_factory;
	Microsoft::WRL::ComPtr<ID3D12Device> m_device;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
	Microsoft::WRL::ComPtr<ID3D12Debug1> m_debugController;
	Microsoft::WRL::ComPtr<ID3D12DebugDevice> m_debugDevice;
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> m_infoQueue;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	Microsoft::WRL::ComPtr<ID3DBlob> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputElementDescs;

	//App resources
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBufferUpload;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBufferUpload;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
	

	//Synchronization objects
	UINT m_frameIndex;
	HANDLE m_fenceEvent;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;
};