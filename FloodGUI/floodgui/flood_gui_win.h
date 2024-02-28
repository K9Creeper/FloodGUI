#pragma once
#include <Windows.h>
#include <windowsx.h>

#include <d3d9.h>
#include "flood_gui.h"
#include "flood_gui_input.h"

struct FloodGuiWinData
{
	HWND hwnd;

	FloodGuiWinData() {
		memset((void*)this, 0, sizeof(*this));
	}
};

bool FloodGuiWinInit(HWND hwnd) {
	if (FloodGui::Context.Initalized)
		return false;
	FloodGuiWinData* backend_data = new FloodGuiWinData();

	backend_data->hwnd = hwnd;
	FloodGui::Context.IO.BackendPlatformData = backend_data;
}
void FloodGuiWinNewFrame()
{
	if (!FloodGui::Context.Initalized)
		return;
	FloodGuiWinData* backend_data = FloodGui::Context.IO.BackendPlatformData;
	if (!backend_data)
		return;
	// Compisate for window resize and position change
	RECT hwndRect = { 0, 0, 0, 0 };
	GetClientRect(backend_data->hwnd, &hwndRect);	
	FloodGui::Context.Display.DisplaySize = FloodVector2{ static_cast<float>(hwndRect.right - hwndRect.left), static_cast<float>(hwndRect.bottom - hwndRect.top) };
	FloodGui::Context.Display.DisplayPosition = FloodVector2{ static_cast<float>(hwndRect.left), static_cast<float>(hwndRect.top) };
}

void FloodGuiWinShutdown()
{
	if (!FloodGui::Context.Initalized)
		return;
	FloodGuiWinData* backend_data = FloodGui::Context.IO.BackendPlatformData;
	if (!backend_data)
		return;

	delete backend_data;
}

LRESULT CALLBACK FloodGuiWindowWinProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_MOUSEMOVE:
		case WM_NCMOUSEMOVE:
		{
			FloodGui::Context.IO.AddMouseMoveEvent(FloodVector2{ (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam) });
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
				const int scancode = (int)LOBYTE(HIWORD(lParam));
				if (FloodGuiWinVirtualKeyToFloodGuiKey(vk) != FloodGuiKey_None) {
					// Send key event to IO
					FloodGui::Context.IO.AddKeyEventDown(vk, keydown);
				}
			}
			break;
		}
		case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
		case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
		{
			FloodMouseButton button = FLoodGuiButton_None;
			if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK) { button = FloodGuiButton_LeftMouse; }
			if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK) { button = FloodGuiButton_RightMouse; }
			if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK) { button = FloodGuiButton_MiddleMouse; }
			if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONDBLCLK) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? FloodGuiButton_Extra3 : FloodGuiButton_Extra4; }
			FloodGui::Context.IO.AddMouseClickEvent(button, true);
			break;
		}
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
		{
			FloodMouseButton button = FLoodGuiButton_None;
			if (msg == WM_LBUTTONUP) { button = FloodGuiButton_LeftMouse; }
			if (msg == WM_RBUTTONUP) { button = FloodGuiButton_RightMouse; }
			if (msg == WM_MBUTTONUP) { button = FloodGuiButton_MiddleMouse; }
			if (msg == WM_XBUTTONUP) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? FloodGuiButton_Extra3 : FloodGuiButton_Extra4; }

			FloodGui::Context.IO.AddMouseClickEvent(button, false);
			break;
		}
	}

	// This will be here for
	// convinence, but it is very cluttery
	if (FloodGui::Context.IO.MouseInput[FloodGuiButton_LeftMouse])
		FloodGui::Context.IO.pmouse_pos = FloodVector2(0, 0);

	return 0;
}