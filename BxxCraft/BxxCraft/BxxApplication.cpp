#include "Utility.h"
#include "BxxApplication.h"

BxxApplication::BxxApplication(UINT width, UINT height, std::wstring appName, std::wstring appTitle) :
	m_width(width),
	m_height(height),
	m_appTitle(appTitle),
	m_appName(appName),
	m_useWarpDevice(false) {
	WCHAR assetsPath[512];
	GetAssetsPath(assetsPath, _countof(assetsPath));
	m_assetsPath = assetsPath;
}

BxxApplication::~BxxApplication() {

}

// Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
// If no such adapter can be found, *ppAdapter will be set to nullptr.
_Use_decl_annotations_
void BxxApplication::ParseCommandLineArgs(WCHAR* argv[], int argc) {
	for (int i = 0; i < argc; i++) {
		if (_wcsnicmp(argv[i], L"-warp", wcslen(argv[i])) == 0) {
			m_useWarpDevice = true;
			m_appTitle += L"(Warp Mode)";
		}
	}
}

_Use_decl_annotations_
std::wstring BxxApplication::GetAssetFullPath(LPCWSTR assetName) {
	return m_assetsPath + assetName;
}

_Use_decl_annotations_
void BxxApplication::GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter) {
	Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; pFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND; adapterIndex++) {
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
			//Skip the basic render driver adapter.
			continue;
		}

		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
			break;
		}
	}
	*ppAdapter = adapter.Detach();
}

_Use_decl_annotations_
void BxxApplication::SetCustomWindowText(LPCWSTR text) {
	std::wstring windowText = wcslen(text) == 0 ? m_appTitle : m_appTitle + L": " + text;
	SetWindowTextW(Win32Application::GetHwnd(), windowText.c_str());
}
