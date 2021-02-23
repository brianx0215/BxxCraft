#include "pch.h"
#include"BxxCraft.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int nCmdShow) {
	std::wstring windowName = L"Bxx Craft";
	std::wstring windowTitle = L"Bxx Craft";

	UINT width = 800;
	UINT height = 600;
	bool fullScreen = false;

	BxxCraft application(width, height, windowName, windowTitle);
	return Win32Application::Run(&application, hInstance, nCmdShow);

}