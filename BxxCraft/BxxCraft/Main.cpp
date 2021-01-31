#include "pch.h"
#include"BxxCraft.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int nCmdShow) {
	std::wstring windowName = L"LYZL craft 12";
	std::wstring windowTitle = L"LYZL craft 12";

	UINT width = 800;
	UINT height = 600;
	bool fullScreen = false;

	BxxCraft application(width, height, windowName, windowTitle);
	return Win32Application::Run(&application, hInstance, nCmdShow);

}