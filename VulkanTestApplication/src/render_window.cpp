#include "render_window.h"

RenderWindow::RenderWindow() :
	windowHandle(NULL)
{
	
}

RenderWindow::~RenderWindow()
{

}

void RenderWindow::Create()
{
	WNDCLASSEX windowClass;
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = &StaticWindowProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = (HINSTANCE)GetModuleHandle(NULL);
	windowClass.hIcon = NULL;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH)(COLOR_BACKGROUND + 1);
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = TEXT("WindowClass");
	windowClass.hIconSm = NULL;
	ATOM result = RegisterClassEx(&windowClass);

	windowHandle = CreateWindowEx(
		WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
		windowClass.lpszClassName, 
		TEXT("VulkanTestApplication"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		CW_USEDEFAULT, 
		CW_USEDEFAULT,
		800,
		600, 
		NULL,
		NULL,
		NULL,
		(LPVOID)this);
}

void RenderWindow::Show()
{
	ShowWindow(windowHandle, SW_NORMAL);
}

void RenderWindow::Hide()
{
	ShowWindow(windowHandle, SW_HIDE);
}

bool RenderWindow::IsOpen() const
{
	return windowHandle != NULL;
}

void RenderWindow::DispatchEvents()
{
	MSG message;

	while (PeekMessage(&message, NULL, NULL, NULL, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}

HWND RenderWindow::GetNativeHandle() const
{
	return windowHandle;
}

LRESULT RenderWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_CLOSE)
	{
		CloseWindow(windowHandle);
		windowHandle = NULL;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK RenderWindow::StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_NCCREATE)
	{
		SetWindowLongPtr(hwnd, GWL_USERDATA, (LONG_PTR)((LPCREATESTRUCT)lParam)->lpCreateParams);
	}
	else
	{
		RenderWindow* renderWindow = reinterpret_cast<RenderWindow*>(GetWindowLongPtr(hwnd, GWL_USERDATA));

		if (renderWindow != NULL)
		{
			return renderWindow->WindowProc(hwnd, uMsg, wParam, lParam);
		}
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}