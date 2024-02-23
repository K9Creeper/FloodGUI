// THIS FILE IS ONLY USED FOR TESTING...
// THIS FILE IS NOT APART OF FloodGUI
// Flood GUI IS LOCATED IN THE FloodGUI FOLDER
#include <iostream>
#include <Windows.h>

#include "floodgui/flood_gui.h"
#include "floodgui/flood_gui_win.h"

FloodColor clearColor(0.45f, 0.55f, 0.60f, 1.f);

LPDIRECT3D9 d3d;    // the pointer to our Direct3D interface
LPDIRECT3DDEVICE9 d3ddev;
MARGINS  margin;
HWND overlay;
MSG msg;
bool running = true;

extern LRESULT CALLBACK FloodGuiWindowWinProcHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (FloodGuiWindowWinProcHandler(hwnd, message, wParam, lParam))
		return TRUE;

	switch (message)
	{
	case WM_PAINT:


		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		break;
	}
	
	return DefWindowProc(hwnd, message, wParam, lParam);
}

int main()
{
	std::cout << "Began\n";
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWindowProc;
	wc.hInstance = nullptr;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)RGB(0, 0, 0);
	wc.lpszClassName = L"WindowClass";
	
	if (!RegisterClassEx(&wc))
		return 0;
	std::cout << "Registered Class\n";
	overlay = CreateWindowExW(0,
		L"WindowClass",
		L"Test Window",
		WS_OVERLAPPEDWINDOW,
		200, 200,
		500, 500,
		NULL,
		NULL,
		wc.hInstance,
		NULL);

	std::cout << "Created Window - "<< overlay << "\n";
	std::cout << "Set Atrbiutes\n";
	ShowWindow(overlay, SW_SHOWDEFAULT);
	std::cout << "Showing window\n";
	d3d = Direct3DCreate9(D3D_SDK_VERSION);    // create the Direct3D interface
	std::cout << "Created D3D9\n";
	D3DPRESENT_PARAMETERS d3dpp;    // create a struct to hold various device information

	ZeroMemory(&d3dpp, sizeof(d3dpp));    // clear out the struct for use
	d3dpp.Windowed = TRUE;    // program windowed, not fullscreen
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
	d3dpp.hDeviceWindow = overlay;    // set the window to be used by Direct3D
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;//D3DFMT_X8R8G8B8;     // set the back buffer format to 32-bit
	d3dpp.BackBufferWidth = 500;    // set the width of the buffer
	d3dpp.BackBufferHeight = 500;    // set the height of the buffer

	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	// create a device class using this information and the info from the d3dpp stuct
	d3d->CreateDevice(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		overlay,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp,
		&d3ddev);
	std::cout << "Created Device\n";

	FloodGuiWinInit(overlay);
	FloodGuiD3D9Init(d3ddev);
	
	FloodGui::Context.DrawData = new FloodDrawData(&FloodGui::Context.Display);

	std::cout << "Initalized Flood\n";
	while (running) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);

			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
			{
				std::cout << "Quit" << "\n";
				// Free memory
				// Clean up


				// Everything before this below
				d3d->Release();
				d3ddev->Release();
				running = false;
			}
		}
		if (!running)
			break;
		// We render down here
		//
		FloodGuiWinNewFrame();
		FloodGuiD3D9NewFrame();

		FloodGui::NewFrame();
		{
			


		}
		FloodGui::EndFrame();

		d3ddev->SetRenderState(D3DRS_ZENABLE, FALSE);
		d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		d3ddev->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clearColor.r * clearColor.a), (int)(clearColor.g * clearColor.a), (int)(clearColor.b * clearColor.a), (int)(clearColor.a));
		d3ddev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
		if (d3ddev->BeginScene() >= 0)
		{
			FloodGui::Render();
			FloodGuiD3D9RenderDrawData(FloodGui::Context.DrawData);
			d3ddev->EndScene();
		}
		HRESULT result = d3ddev->Present(nullptr, nullptr, nullptr, nullptr);
	}

	FloodGuiD3D9Shutdown();
	FloodGuiWinShutdown();

	std::cout << "Uninitalized Flood\n";

	DestroyWindow(overlay);
	UnregisterClassW(wc.lpszClassName, wc.hInstance);
	std::cout << "Destroyed Window\n";
	return -1;
}