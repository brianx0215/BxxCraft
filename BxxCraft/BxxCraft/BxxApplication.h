#pragma once

#include "Win32Application.h"

class BxxApplication {
public:
	BxxApplication(UINT width, UINT height, std::wstring appName, std::wstring appTitle);
	virtual ~BxxApplication();

	virtual void OnInit() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnRender() = 0;
	virtual void OnDestory() = 0;

	UINT GetWidth() const { return m_width; }
	UINT GetHeight() const { return m_height; }
	float GetAspectRatio() const { return static_cast<float>(m_width) / static_cast<float>(m_height); }
	const wchar_t* GetAppTitle() const { return m_appTitle.c_str(); }
	const wchar_t* GetAppName() const { return m_appName.c_str(); }

	void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc);

protected:
	std::wstring GetAssetFullPath(_In_ LPCWSTR assetName);
	void GetHardwareAdapter(_In_ IDXGIFactory2* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter);
	void SetCustomWindowText(_In_ LPCWSTR text);

	UINT m_width;
	UINT m_height;

	bool m_useWarpDevice;

private:
	std::wstring m_assetsPath;
	std::wstring m_appTitle;
	std::wstring m_appName;
};