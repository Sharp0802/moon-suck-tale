#ifndef SPROG_ERROR_H
#define SPROG_ERROR_H

#include "pch.h"

void __DialogHRESULT(HRESULT hr, char* file, int line);

void __DialogWin32Code(DWORD dw, char* file, int line);

#define DialogHRESULT(hr) __DialogHRESULT(hr, __FILE__, __LINE__)
#define DialogWin32Code(hr) __DialogWin32Code(hr, __FILE__, __LINE__)

#endif //SPROG_ERROR_H
