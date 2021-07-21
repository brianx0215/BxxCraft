#pragma once
#include "pch.h"
#include "BxxApplication.h"

class BxxApplication;

class Win32Application {
public:
	static int Run(
		_In_ BxxApplication* pApplication,
		_In_ HINSTANCE hInstance,
		_In_ int nCmdShow);

	static LRESULT CALLBACK WindowProc(
		_In_ HWND   hwnd,
		_In_ UINT   uMsg,
		_In_ WPARAM wParam,
		_In_ LPARAM lParam
	);
	static HWND GetHwnd() { return s_hWnd; };
private:
	static HWND s_hWnd;
};