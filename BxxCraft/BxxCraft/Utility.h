#pragma once
#include <stdexcept>

#include <windows.h>

#include <string>
#include <wrl.h>
#include <shellapi.h>

//Helper functions for desktop app

inline void GetAssetsPath(_Out_writes_(pathSize) WCHAR* path, UINT pathSize) {
	if (path == nullptr) {
		throw std::exception("Asset path invalid");
	}

	DWORD size = GetModuleFileName(nullptr, path, pathSize);
	if (size == 0 || size == pathSize) {
		// Method failed or path was truncated.
		throw std::exception("Asset path invalid");
	}

	WCHAR* lastSlash = wcsrchr(path, L'\\');
	if (lastSlash) {
		*(lastSlash + 1) = L'\0';
	}
}

inline std::string HrToString(_In_ HRESULT hr) {
	char s_str[64] = {};
	sprintf_s(s_str, "HRESULT: 0x%08X", static_cast<UINT>(hr));
	return std::string(s_str);
}

class HrException : public std::runtime_error {
public:
	HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
	HRESULT GetError() const { return m_hr; }
private:
	HRESULT m_hr;
};

inline void ThrowIfFailed(_In_ HRESULT hr) {
	if (FAILED(hr)) {
		throw HrException(hr);
	}
}

inline void ThrowIfFailed(_In_ HANDLE hd) {
	if (hd == nullptr) {
		throw HrException(HRESULT_FROM_WIN32(GetLastError()));
	}
}