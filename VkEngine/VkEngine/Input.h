#ifndef	INPUT_H
#define INPUT_H

#include <Windows.h>
#include <stdint.h>
#include <vector>

class Input
{
public:
	enum INPUT_STATE
	{
		INPUT_STATE_IDLE = 0,
		INPUT_STATE_PRESSED = 1,
		INPUT_STATE_DOWN = 2,
		INPUT_STATE_RELEASED = 3,
	};
	enum KEYS
	{
		KEY_MOUSE_LEFT_BUTTON = VK_LBUTTON,		//	0x01
		KEY_MOUSE_RIGHT_BUTTON = VK_RBUTTON,	//	0x02
		// VK_CANCEL							//	0x03
		KEY_MOUSE_MIDDLE_BUTTON = VK_MBUTTON,	//	0x04
		//	VK_XBUTTON1							//	0x05
		//	VK_XBUTTON2							//	0x06
		//										//	0x07	reserved
		KEY_BACKSPACE = VK_BACK,				//	0x08
		KEY_TAB = VK_TAB,						//	0x09
		//										//	0x0A	reserved
		//										//	0x0B	reserved
		KEY_CLEAR = VK_CLEAR,					//	0x0C
		KEY_ENTER = VK_RETURN,					//	0x0D
		//										//	0x0E	unassigned
		//										//	0x0F	unassigned
		KEY_SHIFT = VK_SHIFT,					//	0x10
		KEY_CONTROL = VK_CONTROL,				//	0x11
		KEY_ALT = VK_MENU,						//	0x12
		KEY_PAUSE = VK_PAUSE,					//	0x13
		KEY_CAPSLOCK = VK_CAPITAL,				//	0x14
		//	VK_KANA								//	0x15
		//										//	0x16	unassigned
		//	VK_JUNJA							//	0x17
		//	VK_FINAL							//	0x18
		//	VK_KANJI							//	0x19
		//										//	0x1A	unassigned
		KEY_ESCAPE = VK_ESCAPE,					//	0x1B
		//	VK_CONVERT							//	0x1C
		//	VK_NONCONVERT						//	0x1D
		//	VK_ACCEPT							//	0x1E
		//	VK_MODECHANGE						//	0x1F
		KEY_SPACE = VK_SPACE,					//	0x20
		KEY_PAGE_UP = VK_PRIOR,					//	0x21
		KEY_PAGE_DOWN = VK_NEXT,				//	0x22
		KEY_END = VK_END,						//	0x23
		KEY_HOME = VK_HOME,						//	0x24
		KEY_ARROW_LEFT = VK_LEFT,				//	0x25
		KEY_ARROW_UP = VK_UP,					//	0x26
		KEY_ARROW_RIGHT = VK_RIGHT,				//	0x27
		KEY_ARROW_DOWN = VK_DOWN,				//	0x28
		//	VK_SELECT							//	0x29
		//	VK_PRINT							//	0x2A
		//	VK_EXECUTE							//	0x2B
		KEY_PRINTSCREEN = VK_SNAPSHOT,			//	0x2C
		KEY_INSERT = VK_INSERT,					//	0x2D
		KEY_DELETE = VK_DELETE,					//	0x2E
		KEY_HELP = VK_HELP,						//	0x2F
		KEY_0 = 0x30,							//	0x30
		KEY_1 = 0x31,							//	0x31
		KEY_2 = 0x32,							//	0x32
		KEY_3 = 0x33,							//	0x33
		KEY_4 = 0x34,							//	0x34
		KEY_5 = 0x35,							//	0x35
		KEY_6 = 0x36,							//	0x36
		KEY_7 = 0x37,							//	0x37
		KEY_8 = 0x38,							//	0x38
		KEY_9 = 0x39,							//	0x39
		//										//	0x3A	unassigned
		//										//	0x3B	unassigned
		//										//	0x3C	unassigned
		//										//	0x3D	unassigned
		//										//	0x3E	unassigned
		//										//	0x3F	unassigned
		//										//	0x40	unassigned
		KEY_A = 'A',							//	0x41
		KEY_B = 'B',							//	0x42
		KEY_C = 'C',							//	0x43
		KEY_D = 'D',							//	0x44
		KEY_E = 'E',							//	0x45
		KEY_F = 'F',							//	0x46
		KEY_G = 'G',							//	0x47
		KEY_H = 'H',							//	0x48
		KEY_I = 'I',							//	0x49
		KEY_J = 'J',							//	0x4A
		KEY_K = 'K',							//	0x4B
		KEY_L = 'L',							//	0x4C
		KEY_M = 'M',							//	0x4D
		KEY_N = 'N',							//	0x4E
		KEY_O = 'O',							//	0x4F
		KEY_P = 'P',							//	0x50
		KEY_Q = 'Q',							//	0x51
		KEY_R = 'R',							//	0x52
		KEY_S = 'S',							//	0x53
		KEY_T = 'T',							//	0x54
		KEY_U = 'U',							//	0x55
		KEY_V = 'V',							//	0x56
		KEY_W = 'W',							//	0x57
		KEY_X = 'X',							//	0x58
		KEY_Y = 'Y',							//	0x59
		KEY_Z = 'Z',							//	0x5A
		KEY_WINDOWS_LEFT = VK_LWIN,				//	0x5B
		KEY_WINDOWS_RIGHT = VK_RWIN,			//	0x5C
		KEY_APPS = VK_APPS,						//	0x5D
		//										//	0x5E	reserved
		KEY_SLEEP = VK_SLEEP,					//	0x5F
		KEY_PAD_0 = VK_NUMPAD0,					//	0x60
		KEY_PAD_1 = VK_NUMPAD1,					//	0x61
		KEY_PAD_2 = VK_NUMPAD2,					//	0x62
		KEY_PAD_3 = VK_NUMPAD3,					//	0x63
		KEY_PAD_4 = VK_NUMPAD4,					//	0x64
		KEY_PAD_5 = VK_NUMPAD5,					//	0x65
		KEY_PAD_6 = VK_NUMPAD6,					//	0x66
		KEY_PAD_7 = VK_NUMPAD7,					//	0x67
		KEY_PAD_8 = VK_NUMPAD8,					//	0x68
		KEY_PAD_9 = VK_NUMPAD9,					//	0x69
		KEY_PAD_MULTIPLY = VK_MULTIPLY,			//	0x6A
		KEY_PAD_ADD = VK_ADD,					//	0x6B
		//	VK_SEPARATOR						//	0x6C
		KEY_PAD_SUBTRACT = VK_SUBTRACT,			//	0x6D
		KEY_PAD_DOT = VK_DECIMAL,				//	0x6E
		KEY_PAD_DIVIDE = VK_DIVIDE,				//	0x6F
		KEY_F1 = VK_F1,							//	0x70
		KEY_F2 = VK_F2,							//	0x71
		KEY_F3 = VK_F3,							//	0x72
		KEY_F4 = VK_F4,							//	0x73
		KEY_F5 = VK_F5,							//	0x74
		KEY_F6 = VK_F6,							//	0x75
		KEY_F7 = VK_F7,							//	0x76
		KEY_F8 = VK_F8,							//	0x77
		KEY_F9 = VK_F9,							//	0x78
		KEY_F10 = VK_F10,						//	0x79
		KEY_F11 = VK_F11,						//	0x7A
		KEY_F12 = VK_F12,						//	0x7B
		KEY_F13 = VK_F13,						//	0x7C
		KEY_F14 = VK_F14,						//	0x7D
		KEY_F15 = VK_F15,						//	0x7E
		KEY_F16 = VK_F16,						//	0x7F
		KEY_F17 = VK_F17,						//	0x80
		KEY_F18 = VK_F18,						//	0x81
		KEY_F19 = VK_F19,						//	0x82
		KEY_F20 = VK_F20,						//	0x83
		KEY_F21 = VK_F21,						//	0x84
		KEY_F22 = VK_F22,						//	0x85
		KEY_F23 = VK_F23,						//	0x86
		KEY_F24 = VK_F24,						//	0x87
		//	VK_NAVIGATION_VIEW					//	0x88	UI navigation
		//	VK_NAVIGATION_MENU					//	0x89	UI navigation
		//	VK_NAVIGATION_UP					//	0x8A	UI navigation
		//	VK_NAVIGATION_DOWN					//	0x8B	UI navigation
		//	VK_NAVIGATION_LEFT					//	0x8C	UI navigation
		//	VK_NAVIGATION_RIGHT					//	0x8D	UI navigation
		//	VK_NAVIGATION_ACCEPT				//	0x8E	UI navigation
		//	VK_NAVIGATION_CANCEL				//	0x8F	UI navigation
		KEY_NUMLOCK = VK_NUMLOCK,				//	0x90
		KEY_SCROLLLOCK = VK_SCROLL,				//	0x91
		//	VK_OEM_NEC_EQUAL					//	0x92
		//	VK_OEM_FJ_MASSHOU					//	0x93
		//	VK_OEM_FJ_TOUROKU					//	0x94
		//	VK_OEM_FJ_LOYA						//	0x95
		//	VK_OEM_FJ_ROYA						//	0x96
		//										//	0x97	unassigned
		//										//	0x98	unassigned
		//										//	0x99	unassigned
		//										//	0x9A	unassigned
		//										//	0x9B	unassigned
		//										//	0x9C	unassigned
		//										//	0x9D	unassigned
		//										//	0x9E	unassigned
		//										//	0x9F	unassigned
		KEY_SHIFT_LEFT = VK_LSHIFT,				//	0xA0
		KEY_SHIFT_RIGHT = VK_RSHIFT,			//	0xA1
		KEY_CONTROL_LEFT = VK_LCONTROL,			//	0xA2
		KEY_CONTROL_RIGHT = VK_RCONTROL,		//	0xA3
		KEY_ALT_LEFT = VK_LMENU,				//	0xA4
		KEY_ALT_RIGHT = VK_RMENU,				//	0xA5
		//	VK_BROWSER_BACK						//	0xA6
		//	VK_BROWSER_FORWARD					//	0xA7
		KEY_REFRESH = VK_BROWSER_REFRESH,		//	0xA8
		//	VK_BROWSER_STOP						//	0xA9
		//	VK_BROWSER_SEARCH					//	0xAA
		//	VK_BROWSER_FAVORITES				//	0xAB
		KEY_HOMEPAGE = VK_BROWSER_HOME,			//	0xAC
		KEY_MUTE = VK_VOLUME_MUTE,				//	0xAD
		KEY_VOLUME_DOWN = VK_VOLUME_DOWN,		//	0xAE
		KEY_VOLUME_UP = VK_VOLUME_UP,			//	0xAF
		KEY_NEXT_TRACK = VK_MEDIA_NEXT_TRACK,	//	0xB0
		KEY_PREV_TRACK = VK_MEDIA_PREV_TRACK,	//	0xB1
		KEY_STOP_TRACK = VK_MEDIA_STOP,			//	0xB2
		KEY_PLAY_TRACK = VK_MEDIA_PLAY_PAUSE,	//	0xB3
		//	VK_LAUNCH_MAIL						//	0xB4
		//	VK_LAUNCH_MEDIA_SELECT				//	0xB5
		//	VK_LAUNCH_APP1						//	0xB6
		//	VK_LAUNCH_APP2						//	0xB7
		//										//	0xB8	reserved
		//										//	0xB9	reserved
		KEY_SEMICOLON = VK_OEM_1,				//	0xBA
		KEY_EQUAL = VK_OEM_PLUS,				//	0xBB
		KEY_COMMA = VK_OEM_COMMA,				//	0xBC
		KEY_UNDERSCORE = VK_OEM_MINUS,			//	0xBD
		KEY_DOT = VK_OEM_PERIOD,				//	0xBE
		KEY_QUESTION = VK_OEM_2,				//	0xBF
		//	VK_OEM_3							//	0xC0
		//										//	0xC1	reserved
		//										//	0xC2	reserved
		//	VK_GAMEPAD_A						//	0xC3	gamepad input
		//	VK_GAMEPAD_B						//	0xC4	gamepad input
		//	VK_GAMEPAD_X						//	0xC5	gamepad input
		//	VK_GAMEPAD_Y						//	0xC6	gamepad input
		//	VK_GAMEPAD_RIGHT_SHOULDER			//	0xC7	gamepad input
		//	VK_GAMEPAD_LEFT_SHOULDER			//	0xC8	gamepad input
		//	VK_GAMEPAD_LEFT_TRIGGER				//	0xC9	gamepad input
		//	VK_GAMEPAD_RIGHT_TRIGGER			//	0xCA	gamepad input
		//	VK_GAMEPAD_DPAD_UP					//	0xCB	gamepad input
		//	VK_GAMEPAD_DPAD_DOWN				//	0xCC	gamepad input
		//	VK_GAMEPAD_DPAD_LEFT				//	0xCD	gamepad input
		//	VK_GAMEPAD_DPAD_RIGHT				//	0xCE	gamepad input
		//	VK_GAMEPAD_MENU						//	0xCF	gamepad input
		//	VK_GAMEPAD_VIEW						//	0xD0	gamepad input
		//	VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON	//	0xD1	gamepad input
		//	VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON	//	0xD2	gamepad input
		//	VK_GAMEPAD_LEFT_THUMBSTICK_UP		//	0xD3	gamepad input
		//	VK_GAMEPAD_LEFT_THUMBSTICK_DOWN		//	0xD4	gamepad input
		//	VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT	//	0xD5	gamepad input
		//	VK_GAMEPAD_LEFT_THUMBSTICK_LEFT		//	0xD6	gamepad input
		//	VK_GAMEPAD_RIGHT_THUMBSTICK_UP		//	0xD7	gamepad input
		//	VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN	//	0xD8	gamepad input
		//	VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT	//	0xD9	gamepad input
		//	VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT	//	0xDA	gamepad input
		KEY_OPEN = VK_OEM_4,					//	0xDB
		KEY_SLASH = VK_OEM_5,					//	0xDC
		KEY_CLOSE = VK_OEM_6,					//	0xDD
		KEY_QUOTE = VK_OEM_7,					//	0xDE
		//	VK_OEM_8							//	0xDF
		//										//	0xE0	reserved
		//	VK_OEM_AX							//	0xE1
		//	VK_OEM_102							//	0xE2
		//	VK_ICO_HELP							//	0xE3
		//	VK_ICO_00							//	0xE4
		//	VK_PROCESSKEY						//	0xE5
		//	VK_ICO_CLEAR						//	0xE6
		//	VK_PACKET							//	0xE7
		//										//	0xE8	unassigned
		//	VK_OEM_RESET						//	0xE9
		//	VK_OEM_JUMP							//	0xEA
		//	VK_OEM_PA1							//	0xEB
		//	VK_OEM_PA2							//	0xEC
		//	VK_OEM_PA3							//	0xED
		//	VK_OEM_WSCTRL						//	0xEE
		//	VK_OEM_CUSEL						//	0xEF
		//	VK_OEM_ATTN							//	0xF0
		//	VK_OEM_FINISH						//	0xF1
		//	VK_OEM_COPY							//	0xF2
		//	VK_OEM_AUTO							//	0xF3
		//	VK_OEM_ENLW							//	0xF4
		//	VK_OEM_BACKTAB						//	0xF5
		//	VK_ATTN								//	0xF6
		//	VK_CRSEL							//	0xF7
		//	VK_EXSEL							//	0xF8
		//	VK_EREOF							//	0xF9
		KEY_PLAY = VK_PLAY,						//	0xFA
		//	VK_ZOOM								//	0xFB
		//	VK_NONAME							//	0xFC
		//	VK_PA1								//	0xFD
		//	VK_OEM_CLEAR						//	0xFE
		//										//	0xFF	reserved
	};

private:
	static INPUT_STATE inputPressData[256];
	static uint32_t mousePosition[2];
	static int32_t mouseDelta[2];
	static std::vector<KEYS>* activeKeys;

public:
	static void Init()
	{
		bool keyState;
		for (uint16_t i = 0; i != 256; ++i)
		{
			keyState = GetAsyncKeyState(i) == 0;

			if(keyState)
				inputPressData[i] = INPUT_STATE_DOWN;
			else
				inputPressData[i] = INPUT_STATE_IDLE;
		}

		POINT mousePos;
		GetCursorPos(&mousePos);

		mouseDelta[0] = 0;
		mouseDelta[1] = 0;

		mousePosition[0] = (uint16_t)mousePos.x;
		mousePosition[1] = (uint16_t)mousePos.y;
	}
	static void Update()
	{
		bool keyState;
		if (activeKeys == nullptr)
		{
			for (uint16_t iKey = 0; iKey != 256; ++iKey)
			{
				keyState = GetAsyncKeyState(iKey) == 0;

				if (keyState)
				{
					if (inputPressData[iKey] == INPUT_STATE_DOWN)
						;
					else if (inputPressData[iKey] == INPUT_STATE_IDLE)
						inputPressData[iKey] = INPUT_STATE_PRESSED;
					else if (inputPressData[iKey] == INPUT_STATE_PRESSED)
						inputPressData[iKey] = INPUT_STATE_DOWN;
					else if (inputPressData[iKey] == INPUT_STATE_RELEASED)
						inputPressData[iKey] = INPUT_STATE_PRESSED;
				}
				else
				{
					if (inputPressData[iKey] == INPUT_STATE_IDLE)
						;
					else if (inputPressData[iKey] == INPUT_STATE_DOWN)
						inputPressData[iKey] = INPUT_STATE_RELEASED;
					else if (inputPressData[iKey] == INPUT_STATE_RELEASED)
						inputPressData[iKey] = INPUT_STATE_IDLE;
					else if (inputPressData[iKey] == INPUT_STATE_PRESSED)
						inputPressData[iKey] = INPUT_STATE_RELEASED;
				}
			}
		}
		else
		{
			for (uint16_t iKey = 0; iKey != activeKeys->size(); ++iKey)
			{
				Input::KEYS key = (*activeKeys)[iKey];

				keyState = GetAsyncKeyState(key) == 0;

				if (keyState)
				{
					if (inputPressData[key] == INPUT_STATE_DOWN)
						;
					else if (inputPressData[key] == INPUT_STATE_IDLE)
						inputPressData[key] = INPUT_STATE_PRESSED;
					else if (inputPressData[key] == INPUT_STATE_PRESSED)
						inputPressData[key] = INPUT_STATE_DOWN;
					else if (inputPressData[key] == INPUT_STATE_RELEASED)
						inputPressData[key] = INPUT_STATE_PRESSED;
				}
				else
				{
					if (inputPressData[key] == INPUT_STATE_IDLE)
						;
					else if (inputPressData[key] == INPUT_STATE_DOWN)
						inputPressData[key] = INPUT_STATE_RELEASED;
					else if (inputPressData[key] == INPUT_STATE_RELEASED)
						inputPressData[key] = INPUT_STATE_IDLE;
					else if (inputPressData[key] == INPUT_STATE_PRESSED)
						inputPressData[key] = INPUT_STATE_RELEASED;
				}
			}
		}

		POINT mousePos;
		GetCursorPos(&mousePos);

		mouseDelta[0] = (uint16_t)mousePos.x - mousePosition[0];
		mouseDelta[1] = (uint16_t)mousePos.y - mousePosition[1];

		mousePosition[0] = (uint16_t)mousePos.x;
		mousePosition[1] = (uint16_t)mousePos.y;
	}
	
	static INPUT_STATE GetKeyInputState(uint8_t _key)
	{
		return inputPressData[_key];
	}
	
	static uint32_t GetMousePositionX()
	{
		return mousePosition[0];
	}
	static uint32_t GetMousePositionY()
	{
		return mousePosition[1];
	}

	static int32_t GetMouseDeltaX()
	{
		return mouseDelta[0];
	}
	static int32_t GetMouseDeltaY()
	{
		return mouseDelta[1];
	}

	static void SetActiveKeys(std::vector<KEYS>* _activeKeys)
	{
		activeKeys = _activeKeys;
	}
};

#endif