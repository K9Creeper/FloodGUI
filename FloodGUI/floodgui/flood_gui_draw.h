#pragma once

#include <Windows.h>
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

namespace FloodGui {
	// Main Functions
	//
	extern inline void Render();
	extern inline void NewFrame();
	extern inline void EndFrame();

	//								//
	// High Level Drawing Functions //
	//								//
	extern inline void BeginWindow(const char* windowName);
	extern inline void EndWindow();

	extern inline bool Button(const char* id);
	extern inline bool Checkbox(const char* id, bool* val);
	extern inline bool Hotkey(const char* id, uint16_t key, bool global = true);
}
class FloodDrawData {
public:
	FloodDrawData(){}
	FloodDrawData(FloodDisplay* display) { this->Display = display;  }
	FloodDisplay* Display = nullptr; // From FloodGui::Context

	std::vector<FloodDrawList*>DrawLists{};

	unsigned int GetVertexCount() const;

	unsigned int GetIndexCount() const;

	bool isMinimized();
};

struct FloodDrawMaterial {
	std::vector<FloodVector2>Points{};
	FloodColor col;
	float thinkness;

	int index_count; 
	int vertex_count; // equal to points size
};

typedef unsigned short FloodDrawIndex;
struct FloodDrawVertex {
	FloodVector2 position;
	FloodVector2 uv;

	unsigned int col = 0x00000000;
};

struct FloodDrawList {
public:
	std::vector< FloodDrawMaterial >Elements{};

	std::vector< FloodDrawVertex > VertexBuffer;
	std::vector< FloodDrawIndex > IndexBuffer;

	FloodDrawIndex* IndexWrite;
	FloodDrawVertex* VertexWrite;

	unsigned int VertexCurrentIdx = 0;

	int GetElementCount() const {
		int ttl = 0;
		for (const FloodDrawMaterial& mat : Elements)
			ttl += mat.Points.size();
		return ttl;
	}

	int GetVertexCount() const {
		return VertexBuffer.size();
	}

	int GetIndexCount() const {
		return IndexBuffer.size();
	}

	void AddLine(const FloodVector2& p1, const FloodVector2& p2, FloodColor col, float thickness = 1.f);
	void AddRect(const FloodVector2& min, const FloodVector2& max, FloodColor col, float thickness = 1.f);
	void AddPolyLine(const std::vector<FloodVector2> points, FloodColor col, float thickness = 1.f);
	void AddRectFilled(const FloodVector2& min, const FloodVector2& max, FloodColor col);
	
	void AddText(const char* text, const FloodVector2& position, FloodColor col, float font_size, float spacing=16);

	void Clear()
	{
		IndexWrite = nullptr;
		VertexWrite = nullptr;
		VertexBuffer.clear();
		IndexBuffer.clear();
		Elements.clear();
		VertexCurrentIdx = 0;
	}
private:
	void AllocRectFilled(const FloodVector2& min, const FloodVector2& max, FloodColor col);
	void AllocChar(char text, const FloodVector2& position, FloodColor col, float font_size);

	void ReserveGeo(const int& index_count, const int& vertex_count);
};

class FloodWindow {
private:
	FloodDrawList DrawList;

	bool isActive = false;

	uint16_t zIndex = 0; // 0 - top layer, 1,2,3.. are behind this index

	const char* sName;
public:
	FloodWindow(const char* sName, FloodVector2 size, FloodVector2 position = FloodVector2(0, 0), float titlebar_size = 25) {
		this->sName = sName;
		this->size = size;
		this->position = position;
		this->titlebar_size = FloodVector2(0, titlebar_size);
	}

	//						id			ver size
	std::vector<std::pair<const char*, float>>content{};

	uint16_t CurrentContentCount() { return content.size(); }

	FloodVector2 position;
	FloodVector2 size;
	FloodVector2 titlebar_size;

	FloodDrawList* GetDrawList() { return &DrawList; }

	void Clear() { DrawList.Clear(); content.clear(); }

	FloodVector2 GetFullBoundingMin() { return position; }
	FloodVector2 GetFullBoundingMax() { return position+size+ FloodVector2{ 0, titlebar_size.y }; }

	FloodVector2 GetBoundingTitleMin() { return position; }
	FloodVector2 GetBoundingTitleMax() { return position + titlebar_size + FloodVector2{ size.x, 0 }; }

	FloodVector2 GetBoundingContentMin() { return position + FloodVector2{0, titlebar_size.y}; }
	FloodVector2 GetBoundingContentMax() { return position + size + FloodVector2{ 0, titlebar_size.y }; }

	bool WindowIsActive()const { return isActive; }
	void SetZIndex(uint16_t zIndex){ this->zIndex = zIndex; }
	uint16_t GetZIndex() { return zIndex; }
	void SetWindowActive(bool s=true) { this->isActive = s; if(s)SetZIndex(0);  }
	void MoveWindow(const FloodVector2& newPos) { position = newPos; }
	void ResizeWindow(const FloodVector2& newSize) { size = newSize; }
};