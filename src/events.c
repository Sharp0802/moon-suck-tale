#include "events.h"
#include "error.h"

/* EXPORTS */
struct __Input Input;

/* IMPLEMENTATIONS */
static COORD s_dwMouseCoord;
static DWORD s_dwMouseButton;
static COORD s_dwWindowSize;

/* INTERNALS */
static DWORD s_dwPreferredFrequency;
static DWORD s_dwFrequencyOffset = 0; // should be zero

static volatile unsigned char s_input[UINT8_MAX];

BOOLEAN NanoSleep(LONGLONG ns)
{
	HANDLE timer;
	LARGE_INTEGER li;

	if(!(timer = CreateWaitableTimer(NULL, TRUE, NULL)))
		return FALSE;

	li.QuadPart = -ns;
	if(!SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE)){
		CloseHandle(timer);
		return FALSE;
	}

	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
	return TRUE;
}

void OnInput(INPUT_RECORD ir)
{
	switch (ir.EventType)
	{
	case KEY_EVENT:
	{
		if (ir.Event.KeyEvent.wVirtualKeyCode > UINT8_MAX)
			break;
		s_input[ir.Event.KeyEvent.wVirtualKeyCode] = TRUE;
		break;
	}

	case MOUSE_EVENT:
		s_dwMouseCoord = ir.Event.MouseEvent.dwMousePosition;
		s_dwMouseButton = ir.Event.MouseEvent.dwButtonState;
		break;

	case FOCUS_EVENT:
	case WINDOW_BUFFER_SIZE_EVENT:
	{
		HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
		HWND hWnd = GetConsoleWindow();

		CONSOLE_FONT_INFO cfi;
		RECT rect;

		if (!GetCurrentConsoleFont(out, FALSE, &cfi) ||
			!GetWindowRect(hWnd, &rect))
		{
			DialogWin32Code(GetLastError());
			break;
		}

		// buffer-size is not same as screen-size
		s_dwWindowSize.Y = (SHORT)((rect.bottom - rect.top) / cfi.dwFontSize.Y - 2);
		s_dwWindowSize.X = ir.Event.WindowBufferSizeEvent.dwSize.X;
		break;
	}
	}
}

void OnUpdateInternal(DOUBLE delta)
{
	OnUpdate(delta);

	DOUBLE slp = (1000.0 / (s_dwPreferredFrequency + s_dwFrequencyOffset)) - delta * 1000;
	if (slp > 1)
	{
		NanoSleep((int)(slp * 1000));
	}

	/* RESET STATE */
	memset((void*)s_input, 0, sizeof s_input);
}

void OnDestroyInternal()
{
	OnDestroy();
}


COORD Input_GetConsoleSize()
{
	return s_dwWindowSize;
}

RECT Input_GetWindowRect()
{
	RECT rect = {0, 0, 0, 0};
	HWND hWnd = GetConsoleWindow();

	if (!GetClientRect(hWnd, &rect))
		return rect;

	return rect;
}

BOOL Input_GetKey(DWORD key)
{
	return s_input[key];
}

BOOL Input_GetMouseButton(DWORD button)
{
	return (s_dwMouseButton & button) != 0;
}

COORD Input_GetMousePosition(void)
{
	return s_dwMouseCoord;
}


void OnInitInternal(void)
{
	DEVMODEA devMode;
	if (!EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &devMode))
		DialogWin32Code(GetLastError());

	s_dwPreferredFrequency = devMode.dmDisplayFrequency;

	Input.GetKey = Input_GetKey;
	Input.GetMouseButton = Input_GetMouseButton;
	Input.GetMousePosition = Input_GetMousePosition;
	Input.GetConsoleSize = Input_GetConsoleSize;
	Input.GetWindowRect = Input_GetWindowRect;

	OnInit();
}
