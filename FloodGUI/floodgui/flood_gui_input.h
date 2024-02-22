#pragma once

#include <Windows.h>
#include <chrono>
#include <cstdint>
#include <vector>

#include "flood_gui.h"
#include "flood_gui_math.h"

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
constexpr uint16_t WindowsInputs[] = { VK_TAB, VK_LEFT, VK_RIGHT,  VK_UP,  VK_DOWN, VK_PRIOR,  VK_NEXT,
 VK_HOME, VK_END,  VK_INSERT, VK_DELETE,  VK_BACK,  VK_SPACE,  VK_RETURN,  VK_OEM_7, VK_OEM_COMMA,
 VK_OEM_MINUS, VK_OEM_PERIOD, VK_OEM_2,  VK_OEM_1, VK_OEM_PLUS,  VK_OEM_4, VK_OEM_5, VK_OEM_6, VK_CAPITAL,  VK_SCROLL,
 VK_NUMLOCK,  VK_SNAPSHOT, VK_PAUSE,  VK_NUMPAD0,  VK_NUMPAD1, VK_NUMPAD2, VK_NUMPAD3, VK_NUMPAD4, VK_NUMPAD5,
 VK_NUMPAD6, VK_NUMPAD7,  VK_NUMPAD8, VK_NUMPAD9, VK_DECIMAL, VK_DIVIDE, VK_MULTIPLY, VK_SUBTRACT,  VK_ADD,  (VK_RETURN + 256),  VK_LSHIFT,
 VK_LCONTROL, VK_LMENU,  VK_LWIN, VK_RSHIFT, VK_RCONTROL, VK_RMENU,  VK_RWIN, VK_APPS,
'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U',
 'V','W','X','Y','Z',VK_F1,VK_F2,VK_F3,VK_F4,VK_F5,VK_F6,VK_F7,VK_F8,VK_F9,VK_F10,VK_F11,VK_F12
};
constexpr FloodKey FloodGuiWinVirtualKeyToFloodGuiKey(WPARAM wParam)
{
    switch (wParam)
    {
    case VK_TAB: return FloodGuiKey_Tab; case VK_LEFT: return FloodGuiKey_LeftArrow; case VK_RIGHT: return FloodGuiKey_RightArrow;
    case VK_UP: return FloodGuiKey_UpArrow; case VK_DOWN: return FloodGuiKey_DownArrow; case VK_PRIOR: return FloodGuiKey_PageUp;
    case VK_NEXT: return FloodGuiKey_PageDown; case VK_HOME: return FloodGuiKey_Home; case VK_END: return FloodGuiKey_End;
    case VK_INSERT: return FloodGuiKey_Insert;  case VK_DELETE: return FloodGuiKey_Delete; case VK_BACK: return FloodGuiKey_Backspace;
    case VK_SPACE: return FloodGuiKey_Space; case VK_RETURN: return FloodGuiKey_Enter; case VK_ESCAPE: return FloodGuiKey_Escape;
    case VK_OEM_7: return FloodGuiKey_Apostrophe; case VK_OEM_COMMA: return FloodGuiKey_Comma;  case VK_OEM_MINUS: return FloodGuiKey_Minus;
    case VK_OEM_PERIOD: return FloodGuiKey_Period; case VK_OEM_2: return FloodGuiKey_Slash; case VK_OEM_1: return FloodGuiKey_Semicolon;
    case VK_OEM_PLUS: return FloodGuiKey_Equal; case VK_OEM_4: return FloodGuiKey_LeftBracket; case VK_OEM_5: return FloodGuiKey_Backslash;
    case VK_OEM_6: return FloodGuiKey_RightBracket; case VK_OEM_3: return FloodGuiKey_GraveAccent; case VK_CAPITAL: return FloodGuiKey_CapsLock;
    case VK_SCROLL: return FloodGuiKey_ScrollLock;  case VK_NUMLOCK: return FloodGuiKey_NumLock; case VK_SNAPSHOT: return FloodGuiKey_PrintScreen;
    case VK_PAUSE: return FloodGuiKey_Pause; case VK_NUMPAD0: return FloodGuiKey_Keypad0; case VK_NUMPAD1: return FloodGuiKey_Keypad1;
    case VK_NUMPAD2: return FloodGuiKey_Keypad2;  case VK_NUMPAD3: return FloodGuiKey_Keypad3;  case VK_NUMPAD4: return FloodGuiKey_Keypad4;
    case VK_NUMPAD5: return FloodGuiKey_Keypad5; case VK_NUMPAD6: return FloodGuiKey_Keypad6; case VK_NUMPAD7: return FloodGuiKey_Keypad7;
    case VK_NUMPAD8: return FloodGuiKey_Keypad8; case VK_NUMPAD9: return FloodGuiKey_Keypad9; case VK_DECIMAL: return FloodGuiKey_KeypadDecimal;
    case VK_DIVIDE: return FloodGuiKey_KeypadDivide; case VK_MULTIPLY: return FloodGuiKey_KeypadMultiply; case VK_SUBTRACT: return FloodGuiKey_KeypadSubtract;
    case VK_ADD: return FloodGuiKey_KeypadAdd; case (VK_RETURN + 256): return FloodGuiKey_KeypadEnter;  case VK_LSHIFT: return FloodGuiKey_LeftShift;
    case VK_LCONTROL: return FloodGuiKey_LeftCtrl; case VK_LMENU: return FloodGuiKey_LeftAlt; case VK_LWIN: return FloodGuiKey_LeftSuper;
    case VK_RSHIFT: return FloodGuiKey_RightShift; case VK_RCONTROL: return FloodGuiKey_RightCtrl;  case VK_RMENU: return FloodGuiKey_RightAlt;
    case VK_RWIN: return FloodGuiKey_RightSuper; case VK_APPS: return FloodGuiKey_Menu; case '0': return FloodGuiKey_0; case '1': return FloodGuiKey_1;
    case '2': return FloodGuiKey_2; case '3': return FloodGuiKey_3; case '4': return FloodGuiKey_4; case '5': return FloodGuiKey_5;
    case '6': return FloodGuiKey_6; case '7': return FloodGuiKey_7; case '8': return FloodGuiKey_8; case '9': return FloodGuiKey_9;
    case 'A': return FloodGuiKey_A; case 'B': return FloodGuiKey_B; case 'C': return FloodGuiKey_C; case 'D': return FloodGuiKey_D;
    case 'E': return FloodGuiKey_E; case 'F': return FloodGuiKey_F; case 'G': return FloodGuiKey_G; case 'H': return FloodGuiKey_H; case 'I': return FloodGuiKey_I;
    case 'J': return FloodGuiKey_J; case 'K': return FloodGuiKey_K; case 'L': return FloodGuiKey_L; case 'M': return FloodGuiKey_M;
    case 'N': return FloodGuiKey_N; case 'O': return FloodGuiKey_O; case 'P': return FloodGuiKey_P; case 'Q': return FloodGuiKey_Q;
    case 'R': return FloodGuiKey_R; case 'S': return FloodGuiKey_S; case 'T': return FloodGuiKey_T; case 'U': return FloodGuiKey_U; case 'V': return FloodGuiKey_V;
    case 'W': return FloodGuiKey_W;  case 'X': return FloodGuiKey_X; case 'Y': return FloodGuiKey_Y; case 'Z': return FloodGuiKey_Z;
    case VK_F1: return FloodGuiKey_F1; case VK_F2: return FloodGuiKey_F2;  case VK_F3: return FloodGuiKey_F3;  case VK_F4: return FloodGuiKey_F4;
    case VK_F5: return FloodGuiKey_F5; case VK_F6: return FloodGuiKey_F6; case VK_F7: return FloodGuiKey_F7;
    case VK_F8: return FloodGuiKey_F8; case VK_F9: return FloodGuiKey_F9; case VK_F10: return FloodGuiKey_F10;
    case VK_F11: return FloodGuiKey_F11; case VK_F12: return FloodGuiKey_F12; default: return FloodGuiKey_None;
    }
}
struct FloodKeyInput {
    FloodKey key;
    std::chrono::milliseconds time{}; // of last click
    std::chrono::milliseconds ms{}; // since last clicked
    uint32_t count = 0; // times pressed
};
struct FloodIO {
    FLOODVector2 mouse_pos;
    void AddMouseMoveEvent(FLOODVector2 mouse_pos);

    std::vector< FloodKeyInput > KeyboardInputs{};
    void AddKeyEvent(FloodKey key, bool down);
};