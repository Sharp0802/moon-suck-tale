#include "events.h"
#include "sys.h"
#include "error.h"
#include "egdi/egdi.h"

#define EXIT_TEXT "Press any key to quit..."


HBITMAP g_hbmTest = NULL;

LPCTSTR g_fonts[] = {
		TEXT("res\\font\\DeterminationMonoK2.ttf"),
		TEXT("res\\font\\DeterminationSansK2.ttf")
};

HFONT g_font_sans;
HFONT g_font_mono;

HDC g_hdc, g_mem;

void OnInit()
{
	EGDI_Initialize();

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

	g_hdc = GetDC(GetConsoleWindow());
	g_mem = CreateCompatibleDC(g_hdc);
}

void OnUpdate(DOUBLE delta)
{
	COORD conSize = Input.GetConsoleSize();

	HWND hWnd = GetConsoleWindow();
	PAINTSTRUCT ps;
	HFONT oldFnt;
	RECT winSize;
	BITMAP bm;

	GetClientRect(hWnd, &winSize);


	char buf[256];
	int wrt = snprintf(buf, sizeof buf, "fps:%5d", (int)(1 / delta));

	/*
	HText ht = Text.Load(RenderContext->m_root, NULL);

	ht->Vtbl.SetValue(ht, L"hello!");
	ht->Vtbl.SetFont(ht, g_font_sans);

	ht->Vtbl.Release(ht);

	EGDI_Render();
*/

	/* FRAME */

	oldFnt = SelectObject(g_hdc, g_font_sans);
	TextOutA(g_hdc, 0, 0, buf, wrt);

	HBITMAP oldBbm = SelectObject(g_mem, g_hbmTest);
	GetObject(g_hbmTest, sizeof(bm), &bm);
	if (!BitBlt(g_hdc, 0, 100, bm.bmWidth, bm.bmHeight, g_mem, 0, 0, SRCCOPY))
		DialogWin32Code(GetLastError());

	SelectObject(g_mem, oldBbm);
	SelectObject(g_hdc, oldFnt);

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
	DeleteDC(g_mem);
	ReleaseDC(GetConsoleWindow(), g_hdc);

	DeleteObject(g_hbmTest);
	DeleteObject(g_font_mono);
	DeleteObject(g_font_sans);

	/* UNREGISTER FONTS */
	for (int i = 0; i < sizeof g_fonts / sizeof *g_fonts; ++i)
		if (!RemoveFontResource(g_fonts[i]))
			DialogWin32Code(E_UNEXPECTED);
}
