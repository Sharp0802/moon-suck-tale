#include "error.h"

void __DialogHRESULT(HRESULT hr, char* file, int line)
{
	static __declspec(thread) char buffer[BUFSIZ];

	if (!FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			HRESULT_CODE(hr),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			buffer + BUFSIZ / 2,
			BUFSIZ / 2,
			NULL))
	{
		snprintf(
				buffer,
				BUFSIZ,
				"%s,%d\nHRESULT: 0x%lX",
				file,
				line,
				hr);
	}
	else
	{
		snprintf(
				buffer,
				BUFSIZ / 2,
				"%s,%d\n%s",
				file,
				line,
				buffer + BUFSIZ / 2);
	}

	MessageBoxA(NULL, buffer, "Error!", MB_ICONERROR);
}

void __DialogWin32Code(DWORD dw, char* file, int line)
{
	static __declspec(thread) char buffer[BUFSIZ];

	if (!FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			buffer + BUFSIZ / 2,
			BUFSIZ / 2,
			NULL))
	{
		snprintf(
				buffer,
				BUFSIZ,
				"%s,%d\nCODE: 0x%lX",
				file,
				line,
				dw);
	}
	else
	{
		snprintf(
				buffer,
				BUFSIZ / 2,
				"%s,%d\n%s",
				file,
				line,
				buffer + BUFSIZ / 2);
	}

	MessageBoxA(NULL, buffer, "Error!", MB_ICONERROR);
}
