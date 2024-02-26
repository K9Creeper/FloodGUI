#pragma once
// Includes / Dependecies
//
#include "flood_gui_draw.h"
#include "flood_gui_math.h"

#include <unordered_map>
#include <chrono>

enum FloodKey : uint16_t {
    FloodGuiKey_None = 0,
    FloodGuiKey_Tab = 512,             // == FloodGuiKey_NamedKey_BEGIN
    FloodGuiKey_LeftArrow,
    FloodGuiKey_RightArrow,
    FloodGuiKey_UpArrow,
    FloodGuiKey_DownArrow,
    FloodGuiKey_PageUp,
    FloodGuiKey_PageDown,
    FloodGuiKey_Home,
    FloodGuiKey_End,
    FloodGuiKey_Insert,
    FloodGuiKey_Delete,
    FloodGuiKey_Backspace,
    FloodGuiKey_Space,
    FloodGuiKey_Enter,
    FloodGuiKey_Escape,
    FloodGuiKey_LeftCtrl, FloodGuiKey_LeftShift, FloodGuiKey_LeftAlt, FloodGuiKey_LeftSuper,
    FloodGuiKey_RightCtrl, FloodGuiKey_RightShift, FloodGuiKey_RightAlt, FloodGuiKey_RightSuper,
    FloodGuiKey_Menu,
    FloodGuiKey_0, FloodGuiKey_1, FloodGuiKey_2, FloodGuiKey_3, FloodGuiKey_4, FloodGuiKey_5, FloodGuiKey_6, FloodGuiKey_7, FloodGuiKey_8, FloodGuiKey_9,
    FloodGuiKey_A, FloodGuiKey_B, FloodGuiKey_C, FloodGuiKey_D, FloodGuiKey_E, FloodGuiKey_F, FloodGuiKey_G, FloodGuiKey_H, FloodGuiKey_I, FloodGuiKey_J,
    FloodGuiKey_K, FloodGuiKey_L, FloodGuiKey_M, FloodGuiKey_N, FloodGuiKey_O, FloodGuiKey_P, FloodGuiKey_Q, FloodGuiKey_R, FloodGuiKey_S, FloodGuiKey_T,
    FloodGuiKey_U, FloodGuiKey_V, FloodGuiKey_W, FloodGuiKey_X, FloodGuiKey_Y, FloodGuiKey_Z,
    FloodGuiKey_F1, FloodGuiKey_F2, FloodGuiKey_F3, FloodGuiKey_F4, FloodGuiKey_F5, FloodGuiKey_F6,
    FloodGuiKey_F7, FloodGuiKey_F8, FloodGuiKey_F9, FloodGuiKey_F10, FloodGuiKey_F11, FloodGuiKey_F12,
    FloodGuiKey_Apostrophe,        // '
    FloodGuiKey_Comma,             // ,
    FloodGuiKey_Minus,             // -
    FloodGuiKey_Period,            // .
    FloodGuiKey_Slash,             // /
    FloodGuiKey_Semicolon,         // ;
    FloodGuiKey_Equal,             // =
    FloodGuiKey_LeftBracket,       // [
    FloodGuiKey_Backslash,         // \ (this text inhibit multiline comment caused by backslash)
    FloodGuiKey_RightBracket,      // ]
    FloodGuiKey_GraveAccent,       // `
    FloodGuiKey_CapsLock,
    FloodGuiKey_ScrollLock,
    FloodGuiKey_NumLock,
    FloodGuiKey_PrintScreen,
    FloodGuiKey_Pause,
    FloodGuiKey_Keypad0, FloodGuiKey_Keypad1, FloodGuiKey_Keypad2, FloodGuiKey_Keypad3, FloodGuiKey_Keypad4,
    FloodGuiKey_Keypad5, FloodGuiKey_Keypad6, FloodGuiKey_Keypad7, FloodGuiKey_Keypad8, FloodGuiKey_Keypad9,
    FloodGuiKey_KeypadDecimal,
    FloodGuiKey_KeypadDivide,
    FloodGuiKey_KeypadMultiply,
    FloodGuiKey_KeypadSubtract,
    FloodGuiKey_KeypadAdd,
    FloodGuiKey_KeypadEnter,
    FloodGuiKey_KeypadEqual,

    FloodGuiKey_GamepadStart,          // Menu (Xbox)      + (Switch)   Start/Options (PS)
    FloodGuiKey_GamepadBack,           // View (Xbox)      - (Switch)   Share (PS)
    FloodGuiKey_GamepadFaceLeft,       // X (Xbox)         Y (Switch)   Square (PS)        // Tap: Toggle Menu. Hold: Windowing mode (Focus/Move/Resize windows)
    FloodGuiKey_GamepadFaceRight,      // B (Xbox)         A (Switch)   Circle (PS)        // Cancel / Close / Exit
    FloodGuiKey_GamepadFaceUp,         // Y (Xbox)         X (Switch)   Triangle (PS)      // Text Input / On-screen Keyboard
    FloodGuiKey_GamepadFaceDown,       // A (Xbox)         B (Switch)   Cross (PS)         // Activate / Open / Toggle / Tweak
    FloodGuiKey_GamepadDpadLeft,       // D-pad Left                                       // Move / Tweak / Resize Window (in Windowing mode)
    FloodGuiKey_GamepadDpadRight,      // D-pad Right                                      // Move / Tweak / Resize Window (in Windowing mode)
    FloodGuiKey_GamepadDpadUp,         // D-pad Up                                         // Move / Tweak / Resize Window (in Windowing mode)
    FloodGuiKey_GamepadDpadDown,       // D-pad Down                                       // Move / Tweak / Resize Window (in Windowing mode)
    FloodGuiKey_GamepadL1,             // L Bumper (Xbox)  L (Switch)   L1 (PS)            // Tweak Slower / Focus Previous (in Windowing mode)
    FloodGuiKey_GamepadR1,             // R Bumper (Xbox)  R (Switch)   R1 (PS)            // Tweak Faster / Focus Next (in Windowing mode)
    FloodGuiKey_GamepadL2,             // L Trig. (Xbox)   ZL (Switch)  L2 (PS) [Analog]
    FloodGuiKey_GamepadR2,             // R Trig. (Xbox)   ZR (Switch)  R2 (PS) [Analog]
    FloodGuiKey_GamepadL3,             // L Stick (Xbox)   L3 (Switch)  L3 (PS)
    FloodGuiKey_GamepadR3,             // R Stick (Xbox)   R3 (Switch)  R3 (PS)
    FloodGuiKey_GamepadLStickLeft,     // [Analog]                                         // Move Window (in Windowing mode)
    FloodGuiKey_GamepadLStickRight,    // [Analog]                                         // Move Window (in Windowing mode)
    FloodGuiKey_GamepadLStickUp,       // [Analog]                                         // Move Window (in Windowing mode)
    FloodGuiKey_GamepadLStickDown,     // [Analog]                                         // Move Window (in Windowing mode)
    FloodGuiKey_GamepadRStickLeft,     // [Analog]
    FloodGuiKey_GamepadRStickRight,    // [Analog]
    FloodGuiKey_GamepadRStickUp,       // [Analog]
    FloodGuiKey_GamepadRStickDown,     // [Analog]

    FloodGuiKey_MouseLeft, FloodGuiKey_MouseRight, FloodGuiKey_MouseMiddle, FloodGuiKey_MouseX1, FloodGuiKey_MouseX2, FloodGuiKey_MouseWheelX, FloodGuiKey_MouseWheelY,

    FloodGuiKey_ReservedForModCtrl, FloodGuiKey_ReservedForModShift, FloodGuiKey_ReservedForModAlt, FloodGuiKey_ReservedForModSuper,
    FloodGuiKey_COUNT,


    FloodGuiMod_None = 0,
    FloodGuiMod_Ctrl = 1 << 12,
    FloodGuiMod_Shift = 1 << 13,
    FloodGuiMod_Alt = 1 << 14,
    FloodGuiMod_Super = 1 << 15,
    FloodGuiMod_Shortcut = 1 << 11,
    FloodGuiMod_Mask_ = 0xF800,

    FloodGuiKey_NamedKey_BEGIN = 512,
    FloodGuiKey_NamedKey_END = FloodGuiKey_COUNT,
    FloodGuiKey_NamedKey_COUNT = FloodGuiKey_NamedKey_END - FloodGuiKey_NamedKey_BEGIN,
}; 

enum FloodMouseButton : uint16_t {
    FLoodGuiButton_None = 0,
    FloodGuiButton_LeftMouse,
    FloodGuiButton_RightMouse,
    FloodGuiButton_MiddleMouse,
    FloodGuiButton_Extra3,
    FloodGuiButton_Extra4
};

struct FloodKeyInput {
	std::chrono::milliseconds time{}; // of last click
	std::chrono::milliseconds ms{}; // since last clicked
	uint32_t count = 0; // times pressed
};

struct FloodGuiD3D9Data;
struct FloodGuiWinData;

enum FloodRenderStage : uint16_t{
    FloodRenderStage_None = 0,
    FloodRenderStage_FrameStart,
    FloodRenderStage_FrameEnd,
    FloodRenderStage_FrameRenderStart,
    FloodRenderStage_FrameRenderEnd
};

struct FloodIO // Input / Output
{
	FloodGuiD3D9Data* BackendRendererData = nullptr;
    FloodGuiWinData* BackendPlatformData = nullptr;

	FloodVector2 mouse_pos; // Current mouse positon
    FloodVector2 pmouse_pos; // Prev mouse positon

	std::unordered_map<FloodMouseButton, bool > MouseInput{};
	void AddMouseMoveEvent(FloodVector2 mouse_pos);
	void AddMouseClickEvent(FloodMouseButton button, bool button_down);

	float KeyRepeatDelay = 275; // ms
	float KeyRepeatRate = 50; // ms
	std::unordered_map< FloodKey, FloodKeyInput > KeyboardInputs{};
	void AddKeyEventDown(FloodKey key);
};

class FloodDisplay {
public:
	FloodDisplay(){ }
	FloodVector2	DisplaySize;
	FloodVector2	DisplayPosition;
    D3DMATRIX matrix_project() {
        // Orthographic projection matrix
        // I HAVE NO IDEA WHAT IM DOING!!!!
        float L = DisplayPosition.x + 0.5f;
        float R = DisplayPosition.x + DisplaySize.x + 0.5f;
        float T = DisplayPosition.y + 0.5f;
        float B = DisplayPosition.y + DisplaySize.y + 0.5f;

        return { { {
            2.0f / (R - L),   0.0f,         0.0f,  0.0f,
            0.0f,         2.0f / (T - B),   0.0f,  0.0f,
            0.0f,         0.0f,         0.5f,  0.0f,
            (L + R) / (L - R),  (T + B) / (B - T),  0.5f,  1.0f
        } } };
    }
};

enum FloodGuiCol : uint16_t {
    FloodGuiCol_WinBkg = 0, 
    FloodGuiCol_WinTitleBar,
    FloodGuiCol_WinTitleBarActive,
    FloodGuiCol_Border,
    FloodGuiCol_Text,
};

class FloodDrawData;
struct FloodWindow;
class FloodContext // Holds current infomation about instance
{
public:
	bool Initalized = false;

	FloodDisplay	    Display;
	FloodIO			    IO;
    FloodDrawData*      DrawData;
    FloodRenderStage    FrameStage = FloodRenderStage_None;

    std::unordered_map<const char*, FloodWindow*>Windows{};

    FloodWindow* ActiveWindow = nullptr;
    FloodWindow* ActiveDrawingWindow = nullptr;
    int ActiveDrawingWindowZIndex = 0;

    std::unordered_map<FloodGuiCol, FloodColor>colors{};
};



namespace FloodGui {
	extern inline FloodContext Context{};

    extern inline void SetupColorStyle() {
        Context.colors[FloodGuiCol_WinBkg] = FloodColor(22, 2, 36, 225);
        Context.colors[FloodGuiCol_WinTitleBar] = FloodColor(16, 2, 26, 255);
        Context.colors[FloodGuiCol_WinTitleBarActive] = FloodColor(32, 5, 51, 255);
        Context.colors[FloodGuiCol_Border] = FloodColor(2, 2, 2, 255);
        Context.colors[FloodGuiCol_Text] = FloodColor(255, 255, 255, 255);
    }
}

constexpr unsigned int ColorToUint32(const FloodColor& color)
{
    return
        ((BYTE(color.a) << 24) +
            (BYTE(color.r) << 16) +
            (BYTE(color.g) << 8) +
            BYTE(color.b));
}