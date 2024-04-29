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
    uint16_t real_key = -1;
    bool raw_down = false; // This will be if it is down
    long long tClick{};
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
	void AddKeyEventDown(uint16_t key, bool down);
};

class FloodDisplay {
public:
	FloodDisplay(){ }
	FloodVector2	DisplaySize;
	FloodVector2	DisplayPosition;
    constexpr D3DMATRIX matrix_project() const {
        // Orthographic projection matrix
        // I HAVE NO IDEA WHAT IM DOING!!!!
        const float L = DisplayPosition.x + 0.5f;
        const float R = DisplayPosition.x + DisplaySize.x + 0.5f;
        const float T = DisplayPosition.y + 0.5f;
        const float B = DisplayPosition.y + DisplaySize.y + 0.5f;

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
    FloodGuiCol_Button,
    FloodGuiCol_ButtonHovered,
    FloodGuiCol_CheckboxActivated,
    FloodGuiCol_Resize,
    FloodGuiCol_ResizeActivated,
    FloodGuiCol_SliderBkg,
    FloodGuiCol_SliderSlider,
    FloodGuiCol_SliderSliderHover
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
    struct FloodFrameData{
        FloodRenderStage    FrameStage = FloodRenderStage_None;
        unsigned long long  FrameCount = 0;
        std::chrono::time_point<std::chrono::system_clock>  tFrameStart;
        std::chrono::time_point<std::chrono::system_clock>  tFrameEnd;
        std::chrono::duration<double> tElaspedFrame;
        constexpr unsigned int CalculateFPS() { return (1000.f / (tElaspedFrame.count()*1000.f)); }
    }FrameData;
    FloodWindow* FindWindowByName(const char* windowName) { return Windows.find(windowName) != Windows.end() ? Windows.find(windowName)->second : nullptr; }
    std::unordered_map<const char*, FloodWindow*>Windows{};

    FloodWindow* ActiveWindow = nullptr;
    FloodWindow* ActiveDrawingWindow = nullptr;
    int ActiveDrawingWindowZIndex = 0; // Try not to reset this...

    FloodDrawList* GetForegroundDrawList() { return DrawData->Foreground; }
    FloodDrawList* GetBackgroundDrawList() { return DrawData->Background; }

    std::unordered_map<FloodGuiCol, FloodColor>colors{};
};

namespace FloodGui {
	extern inline FloodContext Context{};

    extern inline void SetupColorStyle() {
        Context.colors[FloodGuiCol_WinBkg] = FloodColor(0.1f, 0.1f, 0.13f, 1.0f);
        Context.colors[FloodGuiCol_WinTitleBar] = FloodColor(0.16f, 0.16f, 0.21f, 1.0f);
        Context.colors[FloodGuiCol_WinTitleBarActive] = FloodColor(0.2f, 0.2f, 0.25f, 1.0f);
        
        Context.colors[FloodGuiCol_Border] = FloodColor(2, 2, 2, 255);
        Context.colors[FloodGuiCol_Text] = FloodColor(255, 255, 255, 255);
        
        Context.colors[FloodGuiCol_Button] = FloodColor(58, 58, 82, 255);
        Context.colors[FloodGuiCol_ButtonHovered] = FloodColor(73, 73, 104, 255);
    
        Context.colors[FloodGuiCol_CheckboxActivated] = FloodColor(0.74f, 0.58f, 0.98f, 1.f);
    
        Context.colors[FloodGuiCol_Resize] = FloodColor(0.44f, 0.37f, 0.61f, 0.29f);
        Context.colors[FloodGuiCol_ResizeActivated] = FloodColor(0.74f, 0.58f, 0.98f, 0.29f);
    
        Context.colors[FloodGuiCol_SliderBkg] = FloodColor(0.13f, 0.13, 0.17, 1.0f);
        Context.colors[FloodGuiCol_SliderSlider] = FloodColor(0.44f, 0.37f, 0.61f, 0.54f);
        Context.colors[FloodGuiCol_SliderSliderHover] = FloodColor(0.74f, 0.58f, 0.98f, 0.54f);
        
    }

    extern inline void SetupColorBlindStyle(int type) {
        // Maxwell's insane colors
        //
        switch (type) {
        case 1:
            Context.colors[FloodGuiCol_WinBkg] = FloodColor(0.11f, 0.09f, 0.08f, 1.0f); 
            Context.colors[FloodGuiCol_WinTitleBar] = FloodColor(0.17f, 0.13f, 0.11f, 1.0f); 
            Context.colors[FloodGuiCol_WinTitleBarActive] = FloodColor(0.23f, 0.17f, 0.15f, 1.0f); 
            Context.colors[FloodGuiCol_Border] = FloodColor(2, 2, 2, 255); 
            Context.colors[FloodGuiCol_Text] = FloodColor(255, 255, 255, 255); 
            Context.colors[FloodGuiCol_Button] = FloodColor(78, 68, 64, 255); 
            Context.colors[FloodGuiCol_ButtonHovered] = FloodColor(95, 83, 78, 255); 
            Context.colors[FloodGuiCol_CheckboxActivated] = FloodColor(0.75f, 0.54f, 0.52f, 1.f); 
            Context.colors[FloodGuiCol_Resize] = FloodColor(0.46f, 0.33f, 0.31f, 0.29f); 
            Context.colors[FloodGuiCol_ResizeActivated] = FloodColor(0.76f, 0.55f, 0.53f, 0.29f); 
            Context.colors[FloodGuiCol_SliderBkg] = FloodColor(0.12f, 0.10f, 0.09f, 1.0f); 
            Context.colors[FloodGuiCol_SliderSlider] = FloodColor(0.45f, 0.32f, 0.30f, 0.54f); 
            Context.colors[FloodGuiCol_SliderSliderHover] = FloodColor(0.75f, 0.54f, 0.52f, 0.54f);
            break;
        case 2:
            Context.colors[FloodGuiCol_WinBkg] = FloodColor(0.12f, 0.13f, 0.1f, 1.0f); 
            Context.colors[FloodGuiCol_WinTitleBar] = FloodColor(0.18f, 0.20f, 0.15f, 1.0f); 
            Context.colors[FloodGuiCol_WinTitleBarActive] = FloodColor(0.23f, 0.25f, 0.19f, 1.0f);
            Context.colors[FloodGuiCol_Border] = FloodColor(2, 2, 2, 255); 
            Context.colors[FloodGuiCol_Text] = FloodColor(255, 255, 255, 255);
            Context.colors[FloodGuiCol_Button] = FloodColor(63, 69, 59, 255); 
            Context.colors[FloodGuiCol_ButtonHovered] = FloodColor(81, 89, 76, 255); 
            Context.colors[FloodGuiCol_CheckboxActivated] = FloodColor(0.72f, 0.63f, 0.75f, 1.f); 
            Context.colors[FloodGuiCol_Resize] = FloodColor(0.43f, 0.47f, 0.37f, 0.29f); 
            Context.colors[FloodGuiCol_ResizeActivated] = FloodColor(0.73f, 0.68f, 0.75f, 0.29f); 
            Context.colors[FloodGuiCol_SliderBkg] = FloodColor(0.13f, 0.15f, 0.11f, 1.0f); 
            Context.colors[FloodGuiCol_SliderSlider] = FloodColor(0.45f, 0.51f, 0.39f, 0.54f); 
            Context.colors[FloodGuiCol_SliderSliderHover] = FloodColor(0.75f, 0.81f, 0.69f, 0.54f);
            break;
        case 3:
            Context.colors[FloodGuiCol_WinBkg] = FloodColor(0.13f, 0.1f, 0.1f, 1.0f); 
            Context.colors[FloodGuiCol_WinTitleBar] = FloodColor(0.21f, 0.16f, 0.16f, 1.0f);
            Context.colors[FloodGuiCol_WinTitleBarActive] = FloodColor(0.25f, 0.2f, 0.2f, 1.0f);
            Context.colors[FloodGuiCol_Border] = FloodColor(2, 2, 2, 255); 
            Context.colors[FloodGuiCol_Text] = FloodColor(255, 255, 255, 255); 
            Context.colors[FloodGuiCol_Button] = FloodColor(71, 71, 57, 255); 
            Context.colors[FloodGuiCol_ButtonHovered] = FloodColor(89, 89, 71, 255); 
            Context.colors[FloodGuiCol_CheckboxActivated] = FloodColor(0.77f, 0.69f, 0.93f, 1.f); 
            Context.colors[FloodGuiCol_Resize] = FloodColor(0.41f, 0.52f, 0.29f, 0.29f); 
            Context.colors[FloodGuiCol_ResizeActivated] = FloodColor(0.77f, 0.69f, 0.93f, 0.29f); 
            Context.colors[FloodGuiCol_SliderBkg] = FloodColor(0.17f, 0.17f, 0.13f, 1.0f); 
            Context.colors[FloodGuiCol_SliderSlider] = FloodColor(0.48f, 0.58f, 0.39f, 0.54f); 
            Context.colors[FloodGuiCol_SliderSliderHover] = FloodColor(0.77f, 0.69f, 0.93f, 0.54f);
            break;
        }
    }
}