#pragma once
// Pre-Defs
//
struct FloodWindow;

class FLOODVector3;
class FLOODVector2;
class FLOODColor;

class FloodContext; // Holds current infomation about instance']

struct FloodIO; // Input / Output
enum FloodKey;
struct FloodKeyInput;

struct FloodGuiD3D9ata;

struct FloodDisplay;

// Includes / Dependecies
//
#include "flood_gui_draw.h"
#include "flood_gui_math.h"

class FloodContext {
public:
	bool Initalized = false;

	FloodDisplay	Display;
	FloodIO			IO;
};

struct FloodDisplay {
	FLOODVector2	DisplaySize;
	FLOODVector2	DisplayPosition;
};

struct FloodIO {
	FloodGuiD3D9ata* BackendRendererUserData;
};

namespace FloodGui {
	FloodContext Context;
}