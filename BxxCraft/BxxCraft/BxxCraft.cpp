#include "BxxCraft.h"
#include "InputController.h"

BxxCraft::BxxCraft(UINT width, UINT height, std::wstring name, std::wstring title) :
	BxxApplication(width, height, name, title),
	m_frameIndex(0),
	m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
	m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
	m_rtvDescriptorSize(0)
{
	s_FrameStartTick = 0;
}

float BxxCraft::s_frameDeltaTime;
int64_t BxxCraft::s_FrameStartTick;

void BxxCraft::OnInit() {
	LoadPipeline();
	LoadAssets();
	InputController::Initialize(Win32Application::GetHwnd());
}

void BxxCraft::OnUpdate() {
	InputController::Update(s_frameDeltaTime);
	InputController::IsAnyPressed();
}

void BxxCraft::OnRender() {
	PopulateCommandList();

	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	//m_infoQueue->AddMessage(D3D12_MESSAGE_CATEGORY_EXECUTION, D3D12_MESSAGE_SEVERITY_ERROR, D3D12_MESSAGE_ID_ATOMICCOPYBUFFER_INVALID_DST_RESOURCE, "hello");
	//Present the frame.
	int64_t CurrentTick = TimingController::GetCurrentTick();
	s_frameDeltaTime = static_cast<float>(TimingController::SecondsBetweenTicks(s_FrameStartTick, CurrentTick));
	s_FrameStartTick = CurrentTick;
	ThrowIfFailed(m_swapChain->Present(1, 0));
	WaitForPreviousFrame();
}

void BxxCraft::OnDestory() {}

void BxxCraft::LoadPipeline() {
	//Create Factory
	{
		UINT dxgiFactoryFlag = 0;

		// Enable debug layer 
#ifdef _DEBUG
		{
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugController)))) {
				m_debugController->EnableDebugLayer();
				dxgiFactoryFlag |= DXGI_CREATE_FACTORY_DEBUG;
				m_debugController->SetEnableGPUBasedValidation(TRUE);
				m_debugController->SetEnableSynchronizedCommandQueueValidation(TRUE);
			}
		}
#endif // _DEBUG
		ThrowIfFailed(CreateDXGIFactory2(
			dxgiFactoryFlag,
			IID_PPV_ARGS(&m_factory)
		));

		//Disable fullscreen transitions
		ThrowIfFailed(m_factory->MakeWindowAssociation(
			Win32Application::GetHwnd(),
			DXGI_MWA_NO_ALT_ENTER
		));
	}

	//Create Device
	{
		if (m_useWarpDevice) {
			Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
			ThrowIfFailed(m_factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

			ThrowIfFailed(D3D12CreateDevice(
				warpAdapter.Get(),
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(&m_device)
			));

		}
		else {
			Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
			GetHardwareAdapter(m_factory.Get(), &hardwareAdapter);

			ThrowIfFailed(D3D12CreateDevice(
				hardwareAdapter.Get(),
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(&m_device)
			));
		}
#ifdef _DEBUG

		if (m_useWarpDevice) {
			Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
			ThrowIfFailed(m_factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

			ThrowIfFailed(D3D12CreateDevice(
				warpAdapter.Get(),
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(&m_debugDevice)
			));

		}
		else {
			Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
			GetHardwareAdapter(m_factory.Get(), &hardwareAdapter);

			ThrowIfFailed(D3D12CreateDevice(
				hardwareAdapter.Get(),
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(&m_debugDevice)
			));
		}

#endif // _DEBUG

	}

	//Create Command Objects
	{
		//Create Command Queue
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		ThrowIfFailed(m_device->CreateCommandQueue(
			&queueDesc,
			IID_PPV_ARGS(&m_commandQueue)
		));

		//Create Command Allocator
		for (UINT i = 0; i < FrameBufferSize; i++) {
			ThrowIfFailed(m_device->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(&m_commandAllocator[i])
			));
		}

		//Create Command List
		ThrowIfFailed(m_device->CreateCommandList(
			0,	//node Mask
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_commandAllocator->Get(),
			m_pipelineState.Get(),
			IID_PPV_ARGS(&m_commandList)
		));
	}

	//Create Swap Chain
	{
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { };
		swapChainDesc.BufferCount = FrameBufferSize;
		swapChainDesc.Width = m_width;
		swapChainDesc.Height = m_height;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
		ThrowIfFailed(m_factory->CreateSwapChainForHwnd(
			m_commandQueue.Get(),
			Win32Application::GetHwnd(),
			&swapChainDesc,
			nullptr,	//No fullscreen descripter
			nullptr,	//No restrict output
			&swapChain
		));
		//Downcast swapChain from version 1 to version 3
		ThrowIfFailed(swapChain.As(&m_swapChain));
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	}

	//Create Descriptor Objects
	{
		//Create render target descriptor heaps
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameBufferSize;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(
			&rtvHeapDesc,
			IID_PPV_ARGS(&m_rtvHeap)));

		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		//Creat render target view
		//The HANDLE below is a iterator
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		for (UINT i = 0; i < FrameBufferSize; i++) {
			ThrowIfFailed(m_swapChain->GetBuffer(
				i,
				IID_PPV_ARGS(&m_renderTargets[i])
			));
			m_device->CreateRenderTargetView(
				m_renderTargets[i].Get(),
				nullptr,
				rtvHandle);
			rtvHandle.Offset(1, m_rtvDescriptorSize);
		}
	}
}

void BxxCraft::LoadAssets() {
	//Create Root Signature
	{
		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
		rootSignatureDesc.Init(
			0,
			nullptr,
			0,
			nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		);

		Microsoft::WRL::ComPtr<ID3DBlob> signature;
		Microsoft::WRL::ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3D12SerializeRootSignature(
			&rootSignatureDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			&signature,
			&error
		));
		ThrowIfFailed(m_device->CreateRootSignature(
			0,
			signature->GetBufferPointer(),
			signature->GetBufferSize(),
			IID_PPV_ARGS(&m_rootSignature)
		));
	}

	//Create Shaders
	{
#ifdef _DEBUG
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif // _DEBUG

		//Create vertex shader
		ThrowIfFailed(D3DCompileFromFile(
			GetAssetFullPath(L"BasicVS.hlsl").c_str(),
			nullptr,		//shader defines
			nullptr,		//shader include
			"main",
			"vs_5_0",
			compileFlags,
			0,				//flag2
			&m_vertexShader,
			nullptr			//error message
		));

		//Create pixel shader
		ThrowIfFailed(D3DCompileFromFile(
			GetAssetFullPath(L"BasicPS.hlsl").c_str(),
			nullptr,		//shader defines
			nullptr,		//shader include
			"main",
			"ps_5_0",
			compileFlags,
			0,				//flag2
			&m_pixelShader,
			nullptr			//error message
		));

		m_inputElementDescs = {
			// SemanticName, SemanticIndex, Format, InputSlot, AlignedOffset, InputSlotClass, InstanceDataStepRate
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		};
	}
	
	//Create PSO (Graphics Pipeline State Object)
	{
		//Describe and create the PSO.
		//In a real application. It will have many PSOs.
		//For each different shader or different combinations of shaders,
		//different blend states or different rasterizer states.
		//different topology types(points, lines, traingles, paths)
		//or a different number of render targets.

		//VS is the only required shader for a PSO.
		//In case of only outputs data with the stream output.
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = {
			m_inputElementDescs.data(),
			static_cast<UINT>(m_inputElementDescs.size())
		};
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;		//Sample mask has to do with multi-sampling. UINT_MAX = 0xffffffff means point sampling is finished.
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
		ThrowIfFailed(m_device->CreateGraphicsPipelineState(
			&psoDesc,
			IID_PPV_ARGS(&m_pipelineState)));
	}

	//Load Scene
	{
		std::array<Mesh::ColoredVertex, 4>* squareVertices = new std::array<Mesh::ColoredVertex, 4>{
			Mesh::ColoredVertex({ { 0.5f, 0.5f * GetAspectRatio(), 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } }),
			{ { 0.5f, -0.5f * GetAspectRatio(), 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { -0.5f, 0.5f * GetAspectRatio(), 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
			{ { -0.5f, -0.5f * GetAspectRatio(), 0.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } },
		};

		std::array<UINT, 6>* squareIndices = new std::array<UINT, 6>{
			0, 1, 2,
			1, 3, 2
		};


		UINT vertexBufferByteSize = static_cast<UINT>(4 * sizeof(Mesh::ColoredVertex));
		UINT vertexByteSize = sizeof(Mesh::ColoredVertex);
		UINT indexBufferByteSize = static_cast<UINT>(6 * sizeof(UINT));

		UINT squareVertexCount = 4;
		UINT squareIndexCount = 6;

		//Create upload heap.
		//Upload heaps used to upload data to the GPU, CPU can write to it, GPU can read from it.
		//The application upload this heap to default heap
		D3D12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		D3D12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		D3D12_RESOURCE_DESC vertexHeapDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferByteSize);
		D3D12_RESOURCE_DESC indexHeapDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferByteSize);

		ThrowIfFailed(m_device->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&vertexHeapDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,			//GPU will read from this buffer and copy its contents to the default map
			nullptr,									//optimized clear value pointer,
			IID_PPV_ARGS(&m_vertexBufferUpload)
		));
		m_vertexBufferUpload->SetName(L"vertex buffer upload");

		ThrowIfFailed(m_device->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&indexHeapDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,			//GPU will read from this buffer and copy its contents to the default map
			nullptr,									//optimized clear value pointer,
			IID_PPV_ARGS(&m_indexBufferUpload)
		));
		m_indexBufferUpload->SetName(L"index buffer upload");

		//Create default heap.
		//Default heap is memory on GPU. Only GPU has access to this memory.
		//To get data into this heap, the application using an upload heap.
		ThrowIfFailed(m_device->CreateCommittedResource(
			&defaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&vertexHeapDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_vertexBuffer)
		));
		m_vertexBuffer->SetName(L"vertex buffer default");

		ThrowIfFailed(m_device->CreateCommittedResource(
			&defaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&indexHeapDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_indexBuffer)
		));
		m_indexBuffer->SetName(L"index buffer default");

		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.StrideInBytes = sizeof(Mesh::ColoredVertex);
		m_vertexBufferView.SizeInBytes = vertexBufferByteSize;

		m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
		m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
		m_indexBufferView.SizeInBytes = indexBufferByteSize;

		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = reinterpret_cast<BYTE*>(squareVertices->data()); // pointer to our vertex array
		vertexData.RowPitch = vertexBufferByteSize; // size of all our triangle vertex data
		vertexData.SlicePitch = vertexBufferByteSize; // also the size of our triangle vertex data
		UpdateSubresources(m_commandList.Get(), m_vertexBuffer.Get(), m_vertexBufferUpload.Get(), 0, 0, 1, &vertexData);

		D3D12_SUBRESOURCE_DATA indexData = {};
		indexData.pData = reinterpret_cast<BYTE*>(squareIndices->data()); // pointer to our vertex array
		indexData.RowPitch = indexBufferByteSize; // size of all our triangle vertex data
		indexData.SlicePitch = indexBufferByteSize; // also the size of our triangle vertex data
		UpdateSubresources(m_commandList.Get(), m_indexBuffer.Get(), m_indexBufferUpload.Get(), 0, 0, 1, &indexData);

		//UpdateSubresources(m_commandList.Get(), m_testBuffer.Get(), m_indexBufferUpload.Get(), 0, 0, 1, &indexData);

		// transit the vertex buffer type from copy destination state to vertex buffer state
		D3D12_RESOURCE_BARRIER vertexBufferResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_vertexBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
		);
		m_commandList->ResourceBarrier(1, &vertexBufferResourceBarrier);

		D3D12_RESOURCE_BARRIER indexBufferResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_indexBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_INDEX_BUFFER
		);
		m_commandList->ResourceBarrier(1, &indexBufferResourceBarrier);
	}
	m_commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	//Create Synchronization Objects
	{
		// Create synchronization objects and wait until assets have been uploaded to the GPU.
		ThrowIfFailed(m_device->CreateFence(
			0,								//initial value
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(&m_fence)
		));
		m_fenceValue = 1;
		m_fenceEvent = CreateEvent(
			nullptr,						//event attributes
			FALSE,							//manual reset
			FALSE,							//initial state
			L"Fence event"
		);
		ThrowIfFailed(m_fenceEvent);
		WaitForPreviousFrame();
	}
	//Wait for the command list to execute.
	//The application is reusing the same command list in main loop.
	//Currently, we just wannt to wait for steup to complete before continuing.
	WaitForPreviousFrame();
}

void BxxCraft::PopulateCommandList() {
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	ThrowIfFailed(m_commandAllocator[m_frameIndex]->Reset());

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator[m_frameIndex].Get(), m_pipelineState.Get()));

	//Set necessary state.
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	// Indicate that the back buffer will be used as a render target.

	CD3DX12_RESOURCE_BARRIER backBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &backBarrier);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
		m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_frameIndex,
		m_rtvDescriptorSize
	);
	m_commandList->OMSetRenderTargets(
		1,					// number of render target descriptor
		&rtvHandle,
		FALSE,				//table range
		nullptr				//Depth Stencil Buffer
	);
	m_commandList->ClearRenderTargetView(
		rtvHandle,
		DirectX::Colors::Azure,
		0,					//number of Rect
		nullptr				//pRect
	);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->IASetVertexBuffers(
		0,					//start slot
		1,					//number of view
		&m_vertexBufferView
	);
	m_commandList->IASetIndexBuffer(&m_indexBufferView);
	//m_commandList->DrawInstanced(
	//	3,			//vertex count per instance
	//	1,			//instance count
	//	0,			//start vertex location
	//	0			//start instance location
	//);
	m_commandList->DrawIndexedInstanced(
		6,					//indexCount,
		1,					//instance count
		0,					//start index location
		0,					//base vertex location
		0					//start instance location
	);
	
	// Indicate that the back buffer will now be used to present.
	CD3DX12_RESOURCE_BARRIER presentBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);
	m_commandList->ResourceBarrier(1, &presentBarrier);

	ThrowIfFailed(m_commandList->Close());
}

void BxxCraft::WaitForPreviousFrame() {
	// Signal and increase the fence value.
	const UINT64 fence = m_fenceValue;
	ThrowIfFailed(m_commandQueue->Signal(
		m_fence.Get(),
		fence
	));
	m_fenceValue++;

	if (m_fence->GetCompletedValue() < fence) {
		ThrowIfFailed(m_fence->SetEventOnCompletion(
			fence,
			m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}
