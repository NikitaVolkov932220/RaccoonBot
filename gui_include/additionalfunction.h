#ifndef ADDITIONALFUNCTION_H
#define ADDITIONALFUNCTION_H
#include "Structs.h"

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
void getEmulatorsList(QList<HWND>& hwndList, QList<QString>& nameList);

struct EmuSearchResult {
    QList<HWND>* hwndList;
    QList<QString>* nameList;
};
#endif // ADDITIONALFUNCTION_H
