// This is where drawing functions are pre-defined...
// later defined in it's respective .cpp file.

#pragma once
#include <Windows.h>
#include <dwmapi.h>
#include <d3d9.h>
#include <vector>
#include "flood_gui_math.h"

class FloodDrawData;

struct FloodWindow;
struct FloodDisplay;

struct FloodDrawList;

bool FloodGuiD3D9Init(IDirect3DDevice9* device);
//		|   Freelo
//      V
void FloodGuiD3D9Shutdown();

void FloodGuiD3D9NewFrame();
//  |      In this order
//  V
void FloodGuiD3D9RenderDrawData(FloodDrawData* draw_data);

void FloodGuiD3D9Shutdown();

namespace FloodGui {
	// Main Functions
	//
	extern inline void Render();
	extern inline void NewFrame();
	extern inline void EndFrame();

	extern inline void BeginWindow(const char* windowName);
	extern inline void EndWindow();
}
class FloodDrawData {
public:
	FloodDrawData(){}
	FloodDrawData(FloodDisplay* display) { this->Display = display;  }
	FloodDisplay* Display = nullptr; // From FloodGui::Context
	bool isMinimized();
};

struct FloodDrawList {
private:
	std::vector<FloodVector2> Paths{}; //
	void  PathLineTo(const FloodVector2& p) { Paths.push_back(p); }
	void  PathRect(const FloodVector2& a, const FloodVector2& b);

	void  AddPoly(const std::vector<FloodVector2> points, FloodColor col, float thickness);
	void  AddPolyFilled(const std::vector<FloodVector2> points, FloodColor col);
public:
	void  AddLine(const FloodVector2& p1, const FloodVector2& p2, FloodColor col, float thickness = 1.0f);
    void  AddRect(const FloodVector2& min, const FloodVector2& max, FloodColor col, float thickness = 1.0f);
    void  AddRectFilled(const FloodVector2& min, const FloodVector2& max, FloodColor col);
};

class FloodWindow {
private:
	FloodDrawList DrawList;

	bool isActive = false;

	uint16_t zIndex = 0; // 0 - top layer, 1,2,3.. are behind this index

	bool isRendering = false;
public:
	FloodWindow(FloodVector2 size, FloodVector2 position = FloodVector2(0,0)) {
		this->size = size;
		this->position = position;
	}

	FloodVector2 position;
	FloodVector2 size;

	FloodDrawList* GetDrawList() { return &DrawList; }

	bool WindowIsActive()const { return isActive; }
	void SetZIndex(uint16_t zIndex){ this->zIndex = zIndex; }
	uint16_t GetZIndex() { return zIndex; }
	void SetWindowActive() { isActive = true; SetZIndex(0);  }
};