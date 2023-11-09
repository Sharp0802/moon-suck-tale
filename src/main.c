#include "pch.h"
#include "audio.h"
#include "error.h"
#include "events.h"
#include "sys.h"

static volatile BOOL g_exit = FALSE;

void Exit()
{
	g_exit = TRUE;
}

int main()
{
	/* ENFORCE ENCODING */
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);

	/* COM INITIALIZATION */
	HEAP_OPTIMIZE_RESOURCES_INFORMATION hori;
	hori.Version = HEAP_OPTIMIZE_RESOURCES_CURRENT_VERSION;
	hori.Flags = 0;

	if (!HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0) ||
		!HeapSetInformation(NULL, HeapOptimizeResources, &hori, sizeof hori))
		goto E_WIN32;

	HRESULT hr;
	if (FAILED(hr = CoInitializeEx(NULL, COINIT_SPEED_OVER_MEMORY)))
	{
		DialogHRESULT(hr);
		return hr;
	}

	/* ENABLE VIRTUAL PROCESSING */
	DWORD outMode;
	DWORD inMode;
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);

	const DWORD reqOut =
			ENABLE_VIRTUAL_TERMINAL_PROCESSING |
			ENABLE_PROCESSED_OUTPUT |
			DISABLE_NEWLINE_AUTO_RETURN;
	const DWORD reqIn =
			ENABLE_VIRTUAL_TERMINAL_INPUT |
			ENABLE_PROCESSED_INPUT |
			ENABLE_WINDOW_INPUT |
			ENABLE_MOUSE_INPUT;

	if (hIn == INVALID_HANDLE_VALUE ||
		hOut == INVALID_HANDLE_VALUE)
		goto E_WIN32_COM;
	if (!GetConsoleMode(hOut, &outMode) ||
		!GetConsoleMode(hIn, &inMode))
		goto E_WIN32_COM;
	if (!SetConsoleMode(hOut, outMode | reqOut) ||
		!SetConsoleMode(hIn, inMode | reqIn))
		goto E_WIN32_COM;

	/* DISABLE CURSOR */
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 1;
	info.bVisible = FALSE;
	SetConsoleCursorInfo(hOut, &info);

	/* CONFIGURE TIME-SYS */
	LARGE_INTEGER frq, start, end;
	DOUBLE preDelta = 0;
	LONGLONG elapsed = 0;
	if (!QueryPerformanceFrequency(&frq))
		goto E_WIN32_COM;

	/* RUNTIME */
	OnInitInternal();
	if (!QueryPerformanceCounter(&start))
		goto E_WIN32_COM;
	while (!g_exit)
	{
		/* INITIALIZE FRAME */
		DWORD cnt;
		if (GetNumberOfConsoleInputEvents(hIn, &cnt) && cnt)
		{
			INPUT_RECORD irv[256];
			DWORD wrt;
			if (!ReadConsoleInputA(hIn, irv, 256, &wrt))
				goto E_WIN32_COM;
			for (DWORD i = 0; i < wrt; ++i)
				OnInput(irv[i]);
		}

		/* FRAME */
		if (preDelta)
		{
			preDelta += (double)elapsed / (double)frq.QuadPart;
			preDelta /= 2;
		}
		else
		{
			preDelta = (double)elapsed / (double)frq.QuadPart;
		}
		OnUpdateInternal(preDelta);

		/* UPDATE TIME */
		if (!QueryPerformanceCounter(&end))
			goto E_WIN32_COM;
		elapsed = end.QuadPart - start.QuadPart;
		start = end;
	}
	OnDestroy();

	/* FINALIZE */
	CoUninitialize();
	return 0;

E_WIN32_COM:
	CoUninitialize();
E_WIN32:
	DWORD dw = GetLastError();
	DialogWin32Code(dw);
	return (int)dw;
}
