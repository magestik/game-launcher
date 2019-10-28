#include "splashscreen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Windows.h>

#include "splashscreen_common.h"

static volatile bool done = false;

LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM  wParam, LPARAM  lParam)
{
	switch (Msg)
	{
		case WM_CLOSE:
		{
			DestroyWindow(hWnd);
		}
		break;

		case WM_DESTROY:
		{
			PostQuitMessage(0);
		}
		break;

		default:
		{
			return (DefWindowProc(hWnd, Msg, wParam, lParam));
		}
	}

	return 0;
}

bool show_splashscreen_win32(HINSTANCE hInstance, const char * title, const char * background, const char * animation, const char * icon)
{
	int width = DEFAULT_WINDOW_WIDTH;
	int height = DEFAULT_WINDOW_HEIGHT;

	if (background)
	{
		load_background(background, &width, &height);
	}

	if (animation)
	{
		load_animation(animation, ANIM_W, ANIM_H);
	}

	static TCHAR szClassName[] = TEXT("SplashscreenClass");

	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = szClassName;

	if (!RegisterClass(&wc))
	{
		return false;
	}

	RECT DesktopRect;
	GetWindowRect(GetDesktopWindow(), &DesktopRect);

	HWND hWnd = CreateWindowEx(
		WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
		szClassName,
		title,
		WS_POPUP | WS_VISIBLE,
		(DesktopRect.right - width) / 2,
		(DesktopRect.bottom - height) / 2,
		width, height,
		NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		MessageBox(NULL, "Splash Window Creation Failed.", "Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	HDC hDC = GetDC(hWnd);
	cairo_surface_t * surface = cairo_win32_surface_create(hDC);
	cairo_t * cr = cairo_create(surface);

	MSG Msg;

	while (!done)
	{
		while (PeekMessage(&Msg, NULL, 0, 0, 1))
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}

		bool bAnimUpdated = update_animation();

		if (/*bExposeEventReceived ||*/ bAnimUpdated)
		{
			redraw(cr, surface, width, height);
		}
	}
	
	return true;
}

bool hide_splashscreen(void)
{
	done = true;
	return true;
}
