// THIS FILE IS ONLY USED FOR TESTING...
// THIS FILE IS NOT APART OF FloodGUI
// Flood GUI IS LOCATED IN THE FloodGUI FOLDER
#include <iostream>

#include <Windows.h>

#include "floodgui/flood_gui.h"
#include "floodgui/flood_gui_win.h"
#include "floodgui/flood_gui_math.h"

FloodColor clearColor(.8f, .8f, .8f, 1.f);

LPDIRECT3D9 d3d;    // the pointer to our Direct3D interface
LPDIRECT3DDEVICE9 d3ddev;
D3DPRESENT_PARAMETERS d3dpp;
MSG msg;
bool running = true;
HWND hwnd;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();


extern LRESULT CALLBACK FloodGuiWindowWinProcHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (FloodGuiWindowWinProcHandler(hwnd, message, wParam, lParam))
		return TRUE;

	switch (message)
	{
	case WM_PAINT:
	{

		break;
	}
	case WM_SIZE:
	{
		if(!FloodGui::Context.Initalized)
			break;

		RECT hwndRect = { 0, 0, 0, 0 };
		GetClientRect(hwnd, &hwndRect);
		FloodGui::Context.Display.DisplaySize = FloodVector2{ static_cast<float>(hwndRect.right - hwndRect.left), static_cast<float>(hwndRect.bottom - hwndRect.top) };
		d3dpp.BackBufferWidth = FloodGui::Context.Display.DisplaySize.x;
		d3dpp.BackBufferHeight = FloodGui::Context.Display.DisplaySize.y;

		//d3ddev->Reset(&d3dpp);
		break;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
		break;
	}
	}
	
	return DefWindowProc(hwnd, message, wParam, lParam);
}

int main()
{	
	// Set up test window
	WNDCLASSEXW wc;
	{
		wc = { sizeof(wc), CS_CLASSDC, MainWindowProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Flood Gui Example", nullptr };
		RegisterClassExW(&wc);
		hwnd = ::CreateWindowW(wc.lpszClassName, L"Flood Gui Example", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

		// Initialize Direct3D
		if (!CreateDeviceD3D(hwnd))
		{
			CleanupDeviceD3D();
			UnregisterClassW(wc.lpszClassName, wc.hInstance);
			return 1;
		}


		// Show the window
		ShowWindow(hwnd, SW_SHOWDEFAULT);
		UpdateWindow(hwnd);
	}

	FloodGui::SetupColorStyle();
	FloodGuiWinInit(hwnd);
	FloodGuiD3D9Init(d3ddev);
	
	FloodGui::Context.DrawData = new FloodDrawData(&FloodGui::Context.Display);

	std::cout << "Initalized Flood\n";

	D3DCOLOR clear_col_dx = clearColor.ToU32();
	bool b = false;
	bool b2 = false;
	int i = -1;
	while (running) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessage(&msg); if (msg.message == WM_QUIT) {CleanupDeviceD3D(); running = false; } }
		if (!running)
			break;

		FloodGuiWinNewFrame();
		FloodGuiD3D9NewFrame();

		FloodGui::NewFrame();
		{
			FloodGui::BeginWindow("Window 1");
			

			if (FloodGui::Button("Say Hello and Toggle")) {
				std::cout << "Hello From Button!\n";
				b2 = !b2;
			}
			if (FloodGui::Checkbox("Show Alphabet", &b))
				std::cout << "Checkbox was pressed!\n";

			FloodGui::IntSlider("Slider", &i, -5, 15);

			if (b)
			{
				// This is an example of using the Foreground Draw List
				FloodGui::Context.GetForegroundDrawList()->AddText("abcdefghijklmnopqrstuvwxyz 1234567890", FloodVector2(50, 500), FloodColor(255, 0, 0, 255), 15, 9);
			}			

			if (b2)
			{
				// This is an example of using the Background Draw List
				FloodGui::Context.GetBackgroundDrawList()->AddText("This is the background draw list", FloodVector2(200, 550), FloodColor(255, 255, 0, 255), 15, 9);
			}
			FloodGui::EndWindow();

		}
		FloodGui::EndFrame();

		d3ddev->SetRenderState(D3DRS_ZENABLE, FALSE);
		d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		d3ddev->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		
		
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

	DestroyWindow(hwnd);
	UnregisterClassW(wc.lpszClassName, wc.hInstance);

	return -1;
}

bool CreateDeviceD3D(HWND hWnd)
{
	if ((d3d = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
		return false;

	// Create the D3DDevice
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
	//g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
	if (d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &d3ddev) < 0)
		return false;

	return true;
}

void CleanupDeviceD3D()
{
	if (d3ddev) { d3ddev->Release(); d3ddev = nullptr; }
	if (d3d) { d3d->Release(); d3d = nullptr; }
}