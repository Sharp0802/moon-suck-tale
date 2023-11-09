#include "fs.h"

ssize_t LoadFile(LPCTSTR lpszFile, void* data, size_t len)
{
	HANDLE fd;
	DWORD nb;

	fd = CreateFile(lpszFile, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
	if (fd == INVALID_HANDLE_VALUE)
		return -1;

	nb = GetFileSize(fd, NULL);
	if (!nb)
		return -1;

	if (nb > len)
		nb = len;

	if (!ReadFile(fd, data, nb, &nb, 0))
		return -1;

	CloseHandle(fd);
	return nb;
}
