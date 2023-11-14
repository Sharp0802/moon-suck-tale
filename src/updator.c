#include "events.h"
#include "sys.h"
#include "error.h"

#define EXIT_TEXT "Press any key to quit..."


HBITMAP g_hbmTest = NULL;


LPCTSTR g_fonts[] = {
		TEXT("res\\font\\DeterminationMonoK2.ttf"),
		TEXT("res\\font\\DeterminationSansK2.ttf")
};

HFONT g_font_sans;
HFONT g_font_mono;

void OnInit()
{
	g_hbmTest = LoadImage(
			GetModuleHandle(NULL),
			TEXT("res\\test.bmp"),
			IMAGE_BITMAP,
			0, 0,
			LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	if (!g_hbmTest)
		DialogWin32Code(GetLastError());

	/* REGISTER FONTS */
	for (int i = 0; i < sizeof g_fonts / sizeof *g_fonts; ++i)
		if (!AddFontResourceA(g_fonts[i]))
			DialogWin32Code(E_UNEXPECTED);

	g_font_mono = CreateFont(
			48, 0, 0, 0,
			FW_DONTCARE,
			false, false, false,
			DEFAULT_CHARSET,
			OUT_OUTLINE_PRECIS,
			CLIP_DEFAULT_PRECIS,
			CLEARTYPE_QUALITY,
			VARIABLE_PITCH,
			TEXT("DeterminationMonoK2"));
	if (!g_font_mono)
		DialogWin32Code(GetLastError());

	g_font_sans = CreateFont(
			48, 0, 0, 0,
			FW_DONTCARE,
			false, false, false,
			DEFAULT_CHARSET,
			OUT_OUTLINE_PRECIS,
			CLIP_DEFAULT_PRECIS,
			CLEARTYPE_QUALITY,
			VARIABLE_PITCH,
			TEXT("DeterminationSansK2"));
	if (!g_font_sans)
		DialogWin32Code(GetLastError());
}

void OnUpdate(DOUBLE delta)
{
	COORD conSize = Input.GetConsoleSize();

	HWND hWnd = GetConsoleWindow();
	PAINTSTRUCT ps;
	HDC hdc, mem;
	HFONT oldFnt;
	RECT winSize;

	GetClientRect(hWnd, &winSize);

	char buf[256];
	int wrt = snprintf(buf, sizeof buf, "fps:%5d", (int)(1 / delta));

	/* INITIALIZE */
	RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);

	hdc = BeginPaint(hWnd, &ps);
	mem = CreateCompatibleDC(hdc);

	/* FRAME*/
	oldFnt = SelectObject(hdc, g_font_sans);
	DrawTextA(hdc, buf, wrt, &winSize, DT_NOCLIP);
	SelectObject(hdc, oldFnt);

	/* FINALIZE */
	DeleteDC(mem);

	EndPaint(hWnd, &ps);

	/* EXIT GRACEFULLY */
	if (Input.GetKey(VK_ESCAPE))
	{
		printf("\x1b[0;0H\x1b[0J\x1b[%d;%dH" EXIT_TEXT,
				conSize.Y / 2,
				(INT)(conSize.X - strlen(EXIT_TEXT)) / 2);

		_getch();
		Exit();
	}
}

void OnDestroy()
{
	DeleteObject(g_hbmTest);
	DeleteObject(g_font_mono);
	DeleteObject(g_font_sans);

	/* UNREGISTER FONTS */
	for (int i = 0; i < sizeof g_fonts / sizeof *g_fonts; ++i)
		if (!RemoveFontResource(g_fonts[i]))
			DialogWin32Code(E_UNEXPECTED);
}
