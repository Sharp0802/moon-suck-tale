#ifndef SPROG_PCH_H
#define SPROG_PCH_H

#include <initguid.h>
#include <windows.h>
#include <objbase.h>
#include <conio.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

#include <wingdi.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define _USE_MATH_DEFINES
#include <math.h>
#undef _USE_MATH_DEFINES

#ifdef _MSC_VER
#if _WIN64
typedef int64_t ssize_t;
#else
typedef int32_t ssize_t;
#endif
#endif

typedef uint64_t QWORD;

#endif
