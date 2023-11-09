#include "events.h"
#include "sys.h"
#include "error.h"

#define EXIT_TEXT "Press any key to quit..."


HBITMAP g_hbmTest = NULL;


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
}

void OnUpdate(DOUBLE delta)
{
	COORD conSize = Input.GetConsoleSize();
	RECT winSize = Input.GetWindowRect();

	HWND hWnd = GetConsoleWindow();
	PAINTSTRUCT ps;
	HDC hdc, mem, old;
	BITMAP bm;

	RECT fpsRect = { 10, 10, 100, 20 };

	char buf[256];
	int wrt = snprintf(buf, sizeof buf, "fps:%5d", (int)(1 / delta));

	InvalidateRect(hWnd, &winSize, TRUE);

	hdc = BeginPaint(hWnd, &ps);
	mem = CreateCompatibleDC(hdc);
	old = SelectObject(mem, g_hbmTest);

	TextOutA(hdc, 10, 10, buf, wrt);
	DrawTextA(hdc, buf, wrt, fpsRect, );

	GetObject(g_hbmTest, sizeof bm, &bm);
	BitBlt(hdc, 100, 100, bm.bmWidth, bm.bmHeight, mem, 0, 0, SRCCOPY);

	SelectObject(mem, old);
	DeleteDC(mem);

	EndPaint(hWnd, &ps);

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
}
