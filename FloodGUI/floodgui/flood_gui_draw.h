// This is where drawing functions are pre-defined...
// later defined in it's respective .cpp file.

#pragma once
#include <Windows.h>
#include <dwmapi.h>
#include <d3d9.h>

class FloodDrawData;
struct FloodDisplay;

bool FloodGui_D3D9_Init(IDirect3DDevice9* device);
void FloodGui_D3D9_Shutdown();

void FloodGui_D3D9_NewFrame();
void FloodGui_D3D9_RenderDrawData(FloodDrawData* draw_data);


namespace FloodGui {
	// Helper Functions
	//
	

	// Main Functions
	//


}
class FloodDrawData {
public:
	FloodDisplay* Display; // From FloodGui::Context
	bool isMinimized();
};