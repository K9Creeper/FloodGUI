#pragma once
#include <Windows.h>
#include <windowsx.h>

#include <d3d9.h>
#include "flood_gui.h"
#include "flood_gui_input.h"

LRESULT CALLBACK FloodGuiWindowWinProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_MOUSEMOVE:
		case WM_NCMOUSEMOVE:
		{
			POINT mouse_pos = { (LONG)GET_X_LPARAM(lParam), (LONG)GET_Y_LPARAM(lParam) };

			break;
		}
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		{
			const bool keydown = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
			int vk = (int)wParam;
			if (wParam < 256) // Within ASCII range
			{
				// Submit key event
				const FloodKey key = FloodGuiWinVirtualKeyToFloodGuiKey(vk);
				const int scancode = (int)LOBYTE(HIWORD(lParam));
				if (key != FloodGuiKey_None) {
					// Send key event to IO
					
				}
			}
			break;
		}
		// This is where we will now handle input
	}

	return 0;
}