#pragma once
#include <wrl.h>
#include <string>
#include <d3dcompiler.h>
#include <d3d12.h>
#include <unordered_map>
#include <DirectXMath.h>

namespace Mesh {
	struct ColoredVertex {
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT4 Color;
	};

	struct SubMeshView {
		UINT IndexCount;
		UINT StartIndexLocation;
		UINT BaseVertexLocation;
	};

	struct Mesh {
		UINT m_vertexByteSize = 0;
		UINT m_vertexBufferByteSize = 0;
		UINT m_indexBufferByteSize = 0;
		DXGI_FORMAT m_indexFormat = DXGI_FORMAT_UNKNOWN;

		std::unordered_map<std::wstring, SubMeshView>SubMeshViews;

		Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferGPU;
		Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferGPU;
	};
}