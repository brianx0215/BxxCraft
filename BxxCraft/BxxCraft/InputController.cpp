#include "InputController.h"

#define USE_KEYBOARD_MOUSE

HWND InputController::s_hwnd;
bool InputController::s_buttons[2][InputController::DigitalInputCount];
float InputController::s_holdDuration[InputController::DigitalInputCount];

#ifdef USE_KEYBOARD_MOUSE
DIMOUSESTATE2 InputController::s_mouseState;
unsigned char InputController::s_keybuffer[256];
unsigned char InputController::s_dxKeyMapping[KeyCount];

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
IDirectInput8A* InputController::s_digitalInput;
IDirectInputDevice8A* InputController::s_keyboard;
IDirectInputDevice8A* InputController::s_mouse;
#endif
#endif

void InputController::Initialize(HWND hWnd) {
    s_hwnd = hWnd;
#ifdef USE_KEYBOARD_MOUSE
    KbmInitialize();
#endif
}

void InputController::Update(float frameDelta) {
	//Flush inputs and get new input state
	memcpy(s_buttons[1], s_buttons[0], sizeof(s_buttons[0]));
	memset(s_buttons[0], 0, sizeof(s_buttons[0]));

#ifdef USE_KEYBOARD_MOUSE
	KbmUpdate();

	for (uint32_t i = 0; i < KeyCount; ++i) {
        // If some keys are pressed, the corresponding values will be set to 128
		s_buttons[0][i] = (s_keybuffer[s_dxKeyMapping[i]] & 0x80) != 0;
	}

	for (uint32_t i = 0; i < 8; ++i) {
		if (s_mouseState.rgbButtons[i] > 0) s_buttons[0][kMouse0 + i] = true;
	}

    // Update time duration for buttons pressed
    for (uint32_t i = 0; i < KeyCount; i++) {
        if (s_buttons[0][i]) {
            if (!s_buttons[1][i])
                s_holdDuration[i] = 0.0f;
            else
                s_holdDuration[i] += frameDelta;
        }
    }

#endif
}

void InputController::Destory() {}

bool InputController::IsAnyPressed() {
	return s_buttons[0] != 0;
}

bool InputController::IsPressed(DigitalInput input) {
	return s_buttons[0][input];
}

bool InputController::IsFirstPressed(DigitalInput input) {
	return s_buttons[0][input] && s_buttons[1][input];
}

bool InputController::IsReleased(DigitalInput input) {
	return !s_buttons[0][input];
}

bool InputController::IsFirstReleased(DigitalInput input) {
	return !s_buttons[0][input] && s_buttons[1][input];
}

#ifdef USE_KEYBOARD_MOUSE

void InputController::KbmInitialize() {
    KbmBuildKeyMapping();
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    ThrowIfFailed(DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<LPVOID*> (&s_digitalInput), nullptr));
	ThrowIfFailed(s_digitalInput->CreateDevice(GUID_SysKeyboard, &s_keyboard, nullptr));
	ThrowIfFailed(s_keyboard->SetDataFormat(&c_dfDIKeyboard));
	ThrowIfFailed(s_keyboard->SetCooperativeLevel(InputController::s_hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));

	DIPROPDWORD dipdw;
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = 10;

	ThrowIfFailed(s_keyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph));
    ThrowIfFailed(s_digitalInput->CreateDevice(GUID_SysMouse, &s_mouse, nullptr));
    ThrowIfFailed(s_mouse->SetDataFormat(&c_dfDIMouse2));
	ThrowIfFailed(s_mouse->SetCooperativeLevel(InputController::s_hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE));
#endif
}

void InputController::KbmBuildKeyMapping() {
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    s_dxKeyMapping[Key_escape] = DIK_ESCAPE;
    s_dxKeyMapping[Key_1] = DIK_1;
    s_dxKeyMapping[Key_2] = DIK_2;
    s_dxKeyMapping[Key_3] = DIK_3;
    s_dxKeyMapping[Key_4] = DIK_4;
    s_dxKeyMapping[Key_5] = DIK_5;
    s_dxKeyMapping[Key_6] = DIK_6;
    s_dxKeyMapping[Key_7] = DIK_7;
    s_dxKeyMapping[Key_8] = DIK_8;
    s_dxKeyMapping[Key_9] = DIK_9;
    s_dxKeyMapping[Key_0] = DIK_0;
    s_dxKeyMapping[Key_minus] = DIK_MINUS;
    s_dxKeyMapping[Key_equals] = DIK_EQUALS;
    s_dxKeyMapping[Key_back] = DIK_BACK;
    s_dxKeyMapping[Key_tab] = DIK_TAB;
    s_dxKeyMapping[Key_q] = DIK_Q;
    s_dxKeyMapping[Key_w] = DIK_W;
    s_dxKeyMapping[Key_e] = DIK_E;
    s_dxKeyMapping[Key_r] = DIK_R;
    s_dxKeyMapping[Key_t] = DIK_T;
    s_dxKeyMapping[Key_y] = DIK_Y;
    s_dxKeyMapping[Key_u] = DIK_U;
    s_dxKeyMapping[Key_i] = DIK_I;
    s_dxKeyMapping[Key_o] = DIK_O;
    s_dxKeyMapping[Key_p] = DIK_P;
    s_dxKeyMapping[Key_lbracket] = DIK_LBRACKET;
    s_dxKeyMapping[Key_rbracket] = DIK_RBRACKET;
    s_dxKeyMapping[Key_return] = DIK_RETURN;
    s_dxKeyMapping[Key_lcontrol] = DIK_LCONTROL;
    s_dxKeyMapping[Key_a] = DIK_A;
    s_dxKeyMapping[Key_s] = DIK_S;
    s_dxKeyMapping[Key_d] = DIK_D;
    s_dxKeyMapping[Key_f] = DIK_F;
    s_dxKeyMapping[Key_g] = DIK_G;
    s_dxKeyMapping[Key_h] = DIK_H;
    s_dxKeyMapping[Key_j] = DIK_J;
    s_dxKeyMapping[Key_k] = DIK_K;
    s_dxKeyMapping[Key_l] = DIK_L;
    s_dxKeyMapping[Key_semicolon] = DIK_SEMICOLON;
    s_dxKeyMapping[Key_apostrophe] = DIK_APOSTROPHE;
    s_dxKeyMapping[Key_grave] = DIK_GRAVE;
    s_dxKeyMapping[Key_lshift] = DIK_LSHIFT;
    s_dxKeyMapping[Key_backslash] = DIK_BACKSLASH;
    s_dxKeyMapping[Key_z] = DIK_Z;
    s_dxKeyMapping[Key_x] = DIK_X;
    s_dxKeyMapping[Key_c] = DIK_C;
    s_dxKeyMapping[Key_v] = DIK_V;
    s_dxKeyMapping[Key_b] = DIK_B;
    s_dxKeyMapping[Key_n] = DIK_N;
    s_dxKeyMapping[Key_m] = DIK_M;
    s_dxKeyMapping[Key_comma] = DIK_COMMA;
    s_dxKeyMapping[Key_period] = DIK_PERIOD;
    s_dxKeyMapping[Key_slash] = DIK_SLASH;
    s_dxKeyMapping[Key_rshift] = DIK_RSHIFT;
    s_dxKeyMapping[Key_multiply] = DIK_MULTIPLY;
    s_dxKeyMapping[Key_lalt] = DIK_LMENU;
    s_dxKeyMapping[Key_space] = DIK_SPACE;
    s_dxKeyMapping[Key_capital] = DIK_CAPITAL;
    s_dxKeyMapping[Key_f1] = DIK_F1;
    s_dxKeyMapping[Key_f2] = DIK_F2;
    s_dxKeyMapping[Key_f3] = DIK_F3;
    s_dxKeyMapping[Key_f4] = DIK_F4;
    s_dxKeyMapping[Key_f5] = DIK_F5;
    s_dxKeyMapping[Key_f6] = DIK_F6;
    s_dxKeyMapping[Key_f7] = DIK_F7;
    s_dxKeyMapping[Key_f8] = DIK_F8;
    s_dxKeyMapping[Key_f9] = DIK_F9;
    s_dxKeyMapping[Key_f10] = DIK_F10;
    s_dxKeyMapping[Key_numlock] = DIK_NUMLOCK;
    s_dxKeyMapping[Key_scroll] = DIK_SCROLL;
    s_dxKeyMapping[Key_numpad7] = DIK_NUMPAD7;
    s_dxKeyMapping[Key_numpad8] = DIK_NUMPAD8;
    s_dxKeyMapping[Key_numpad9] = DIK_NUMPAD9;
    s_dxKeyMapping[Key_subtract] = DIK_SUBTRACT;
    s_dxKeyMapping[Key_numpad4] = DIK_NUMPAD4;
    s_dxKeyMapping[Key_numpad5] = DIK_NUMPAD5;
    s_dxKeyMapping[Key_numpad6] = DIK_NUMPAD6;
    s_dxKeyMapping[Key_add] = DIK_ADD;
    s_dxKeyMapping[Key_numpad1] = DIK_NUMPAD1;
    s_dxKeyMapping[Key_numpad2] = DIK_NUMPAD2;
    s_dxKeyMapping[Key_numpad3] = DIK_NUMPAD3;
    s_dxKeyMapping[Key_numpad0] = DIK_NUMPAD0;
    s_dxKeyMapping[Key_decimal] = DIK_DECIMAL;
    s_dxKeyMapping[Key_f11] = DIK_F11;
    s_dxKeyMapping[Key_f12] = DIK_F12;
    s_dxKeyMapping[Key_numpadenter] = DIK_NUMPADENTER;
    s_dxKeyMapping[Key_rcontrol] = DIK_RCONTROL;
    s_dxKeyMapping[Key_divide] = DIK_DIVIDE;
    s_dxKeyMapping[Key_sysrq] = DIK_SYSRQ;
    s_dxKeyMapping[Key_ralt] = DIK_RMENU;
    s_dxKeyMapping[Key_pause] = DIK_PAUSE;
    s_dxKeyMapping[Key_home] = DIK_HOME;
    s_dxKeyMapping[Key_up] = DIK_UP;
    s_dxKeyMapping[Key_pgup] = DIK_PRIOR;
    s_dxKeyMapping[Key_left] = DIK_LEFT;
    s_dxKeyMapping[Key_right] = DIK_RIGHT;
    s_dxKeyMapping[Key_end] = DIK_END;
    s_dxKeyMapping[Key_down] = DIK_DOWN;
    s_dxKeyMapping[Key_pgdn] = DIK_NEXT;
    s_dxKeyMapping[Key_insert] = DIK_INSERT;
    s_dxKeyMapping[Key_delete] = DIK_DELETE;
    s_dxKeyMapping[Key_lwin] = DIK_LWIN;
    s_dxKeyMapping[Key_rwin] = DIK_RWIN;
    s_dxKeyMapping[Key_apps] = DIK_APPS;
#endif
}

void InputController::KbmClean() {
	memset(&s_mouseState, 0, sizeof(DIMOUSESTATE2));
	memset(s_keybuffer, 0, sizeof(s_keybuffer));
}

void InputController::KbmUpdate() {
	HWND foreground = GetForegroundWindow();
	bool visiable = IsWindowVisible(foreground) == TRUE;
	if (foreground != InputController::s_hwnd || !visiable) {
		KbmClean();
	}
	else {
		s_mouse->Acquire();
		s_mouse->GetDeviceState(sizeof(DIMOUSESTATE2), &s_mouseState);
		s_keyboard->Acquire();
		s_keyboard->GetDeviceState(sizeof(s_keybuffer), s_keybuffer);
        //for (size_t i = 0; i < 256; i++) 		{
        //    if (s_keybuffer[i] != 0) {
        //        char* msgbuf = new char[100];
        //        sprintf_s(msgbuf, 50,"%d %d\n", s_dxKeyMapping[i], i);
        //        OutputDebugStringA(msgbuf);
        //    }
        //}

	}
}
#endif
