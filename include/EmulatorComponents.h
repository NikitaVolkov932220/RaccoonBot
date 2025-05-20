#ifndef EMULATORCOMPONENTS_H
#define EMULATORCOMPONENTS_H
#include "BotLib_global.h"
#include "Structs.h"
#include "Error.h"

wstring GetCommandLineForHWND(HWND hwnd);

wstring sanitizePath(const wstring& input, typeEmu type, int& instance);

bool FindEmulator(const QString &windowName, HWND* main, HWND* game); // need bool?

#endif // EMULATORCOMPONENTS_H
