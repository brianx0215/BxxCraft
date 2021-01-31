#include "Win32Application.h"

HWND Win32Application::m_hWnd = nullptr;
_Use_decl_annotations_
int Win32Application::Run(BxxApplication* pApplication, HINSTANCE hInstance, int nCmdShow) {
	// Parse the command line parameters
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	pApplication->ParseCommandLineArgs(argv, argc);
	LocalFree(argv);

	//Initialize the window class
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = pApplication->GetAppName();
	ThrowIfFailed(RegisterClassEx(&windowClass));

	RECT windowRect = {
		0,
		0,
		static_cast<long>(pApplication->GetWidth()),
		static_cast<long>(pApplication->GetHeight())
	};
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	m_hWnd = CreateWindow(
		windowClass.lpszClassName,
		pApplication->GetAppTitle(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,	// no parent window
		nullptr,	// no menus
		hInstance,
		pApplication
	);

	//Initialize the application. Oninit is defined in child-implementation.
	pApplication->OnInit();

	ShowWindow(GetHwnd(), nCmdShow);

	//Main application loop
	MSG msg = {};
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	pApplication->OnDestory();

	// Return this part of the WM_QUIT message to Windows.
	return static_cast<char>(msg.wParam);
}

_Use_decl_annotations_
LRESULT CALLBACK Win32Application::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	BxxApplication* pApplication = reinterpret_cast<BxxApplication*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	switch (uMsg) {
	case WM_CREATE:
	{
		LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
	}
	return 0;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			if (MessageBox(0, L"Are you sure you want to exit?",
				L"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
				DestroyWindow(hwnd);
		}
		return 0;
	case WM_PAINT:
		if (pApplication) {
			pApplication->OnUpdate();
			pApplication->OnRender();
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		pApplication->OnDestory();
		return 0;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}
