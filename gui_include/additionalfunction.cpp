#include "additionalfunction.h"

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    auto* result = reinterpret_cast<EmuSearchResult*>(lParam);
    if (!result || !result->hwndList || !result->nameList) {
        return TRUE; // Continue enumeration
    }

    wchar_t className[256] = {0};
    wchar_t windowTitle[256] = {0};

    if (GetClassNameW(hwnd, className, sizeof(className)/sizeof(wchar_t))) {
        GetWindowTextW(hwnd, windowTitle, sizeof(windowTitle)/sizeof(wchar_t));

        if (wcscmp(className, L"Qt5154QWindowIcon") == 0 ||
            wcscmp(className, L"LDPlayerMainFrame") == 0) {

            QString name = QString::fromWCharArray(windowTitle);
            if (!name.isEmpty()) {
                result->hwndList->append(hwnd);
                result->nameList->append(name);
            }
        }
    }
    return TRUE;
}

void getEmulatorsList(QList<HWND>& hwndList, QList<QString>& nameList) {
    EmuSearchResult result{ &hwndList, &nameList };
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&result));
}
