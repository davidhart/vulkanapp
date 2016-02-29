#pragma once

#include <Windows.h>

class RenderWindow
{
private:
	HWND windowHandle;

public:
	RenderWindow();
	~RenderWindow();
	void Create();
	void Show();
	void Hide();
	bool IsOpen() const;
	void DispatchEvents();

	HWND GetNativeHandle() const;

private:
	LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lparam);
	static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

};