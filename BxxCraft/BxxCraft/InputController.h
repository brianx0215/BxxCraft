#pragma once
#include "pch.h"
#include <dinput.h>

#define USE_KEYBOARD_MOUSE
class InputController {
public:
	static HWND s_hwnd;
    enum DigitalInput {
        // The defination order is defined in dinput.h. See DIK_ESCAPE in the header for more details.
        Key_escape = 0,
        Key_1,
        Key_2,
        Key_3,
        Key_4,
        Key_5,
        Key_6,
        Key_7,
        Key_8,
        Key_9,
        Key_0,
        Key_minus,
        Key_equals,
        Key_back,
        Key_tab,
        Key_q,
        Key_w,
        Key_e,
        Key_r,
        Key_t,
        Key_y,
        Key_u,
        Key_i,
        Key_o,
        Key_p,
        Key_lbracket,
        Key_rbracket,
        Key_return,
        Key_lcontrol,
        Key_a,
        Key_s,
        Key_d,
        Key_f,
        Key_g,
        Key_h,
        Key_j,
        Key_k,
        Key_l,
        Key_semicolon,
        Key_apostrophe,
        Key_grave,
        Key_lshift,
        Key_backslash,
        Key_z,
        Key_x,
        Key_c,
        Key_v,
        Key_b,
        Key_n,
        Key_m,
        Key_comma,
        Key_period,
        Key_slash,
        Key_rshift,
        Key_multiply,
        Key_lalt,
        Key_space,
        Key_capital,
        Key_f1,
        Key_f2,
        Key_f3,
        Key_f4,
        Key_f5,
        Key_f6,
        Key_f7,
        Key_f8,
        Key_f9,
        Key_f10,
        Key_numlock,
        Key_scroll,
        Key_numpad7,
        Key_numpad8,
        Key_numpad9,
        Key_subtract,
        Key_numpad4,
        Key_numpad5,
        Key_numpad6,
        Key_add,
        Key_numpad1,
        Key_numpad2,
        Key_numpad3,
        Key_numpad0,
        Key_decimal,
        Key_f11,
        Key_f12,
        Key_numpadenter,
        Key_rcontrol,
        Key_divide,
        Key_sysrq,
        Key_ralt,
        Key_pause,
        Key_home,
        Key_up,
        Key_pgup,
        Key_left,
        Key_right,
        Key_end,
        Key_down,
        Key_pgdn,
        Key_insert,
        Key_delete,
        Key_lwin,
        Key_rwin,
        Key_apps,

        KeyCount,

        // mouse
        kMouse0 = KeyCount,
        kMouse1,
        kMouse2,
        kMouse3,
        kMouse4,
        kMouse5,
        kMouse6,
        kMouse7,

        DigitalInputCount
    };

	static void Initialize(HWND g_hWnd);
	static void Update(float frameDelta);
	static void Destory();

	static bool IsAnyPressed();
	static bool IsPressed(DigitalInput input);
	static bool IsFirstPressed(DigitalInput input);
	static bool IsReleased(DigitalInput input);
	static bool IsFirstReleased(DigitalInput input);

	
private:
	static bool s_buttons[2][InputController::DigitalInputCount];
	static float s_holdDuration[InputController::DigitalInputCount];
    
#ifdef USE_KEYBOARD_MOUSE
	static DIMOUSESTATE2 s_mouseState;
	static unsigned char s_keybuffer[256];
    static unsigned char s_dxKeyMapping[KeyCount];  // map DigitalInput enum to DX key codes

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	static IDirectInput8A* s_digitalInput;
	static IDirectInputDevice8A* s_keyboard;
	static IDirectInputDevice8A* s_mouse;
#endif

	static void KbmInitialize();
	static void KbmBuildKeyMapping();
	static void KbmClean();
	static void KbmUpdate();
#endif
};