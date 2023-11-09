#ifndef SPROG_EVENTS_H
#define SPROG_EVENTS_H

#include "pch.h"

void OnInitInternal();
void OnInput(INPUT_RECORD ir);
void OnUpdateInternal(DOUBLE delta);
void OnDestroyInternal();

void OnInit();
void OnUpdate(DOUBLE delta);
void OnDestroy();

struct __Input
{
	BOOL (*GetKey)(DWORD key);
	BOOL (*GetMouseButton)(DWORD button);
	COORD (*GetMousePosition)(void);
	COORD (*GetConsoleSize)(void);
	RECT (*GetWindowRect)(void);
};

extern struct __Input Input;

#endif //SPROG_EVENTS_H
