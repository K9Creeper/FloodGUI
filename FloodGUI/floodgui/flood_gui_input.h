#pragma once
#include <Windows.h>

#include <cstdint>

#include "flood_gui.h"
#include "flood_gui_math.h"

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
    case VK_ADD: return FloodGuiKey_KeypadAdd; case (VK_RETURN + 256): return FloodGuiKey_KeypadEnter;  case VK_LSHIFT: case 16: return FloodGuiKey_LeftShift;
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

void FloodIO::AddKeyEventDown(uint16_t key, bool down)
{
    if (FloodGui::Context.FrameData.FrameStage != FloodRenderStage_FrameRenderEnd)
        return;
    FloodKey fKey = FloodGuiWinVirtualKeyToFloodGuiKey(key);
    if (KeyboardInputs.find(fKey) == KeyboardInputs.end())
        KeyboardInputs[fKey] = FloodKeyInput{ key, down , std::chrono::system_clock::now().time_since_epoch().count()};
    else {        
        KeyboardInputs[fKey].real_key = key;
        KeyboardInputs[fKey].raw_down = down;
        KeyboardInputs[fKey].tClick = std::chrono::system_clock::now().time_since_epoch().count();
    }
 }

void FloodIO::AddMouseMoveEvent(FloodVector2 mouse_pos)
{
    if (!FloodGui::Context.IO.MouseInput[FloodGuiButton_LeftMouse])
        this->pmouse_pos = mouse_pos;
    this->mouse_pos = mouse_pos;
}
void FloodIO::AddMouseClickEvent(FloodMouseButton button, bool button_down) {
    if (FloodGui::Context.FrameData.FrameStage != FloodRenderStage_FrameRenderEnd)
        return;
    this->MouseInput[button] = button_down;
}