#include "EmulatorComponents.h"
#pragma comment(lib, "wbemuuid.lib")

wstring GetCommandLineForHWND(HWND hwnd) {

    DWORD processID;
    GetWindowThreadProcessId(hwnd, &processID);

    HRESULT hres;

    // Инициализация COM
    hres = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
    if (FAILED(hres)) return L"";

    // Настройка безопасности
    hres = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL
        );

    if (FAILED(hres)) {
        CoUninitialize();
        return L"";
    }

    // Получение указателя на WMI
    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID*)&pLoc
        );

    if (FAILED(hres)) {
        CoUninitialize();
        return L"";
    }

    IWbemServices* pSvc = NULL;

    // Подключение к WMI
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL,
        NULL,
        0,
        NULL,
        0,
        0,
        &pSvc
        );

    if (FAILED(hres)) {
        pLoc->Release();
        CoUninitialize();
        return L"";
    }

    // Установка уровня безопасности для WMI
    hres = CoSetProxyBlanket(
        pSvc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE
        );

    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return L"";
    }

    // Формируем запрос для WMI
    IEnumWbemClassObject* pEnumerator = NULL;
    wstring query = L"SELECT CommandLine FROM Win32_Process WHERE ProcessId = " + to_wstring(processID);

    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t(query.c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator
        );

    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return L"";
    }

    // Читаем результат запроса
    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;
    wstring commandLine;

    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

        if (0 == uReturn) break;

        VARIANT vtProp;
        hr = pclsObj->Get(L"CommandLine", 0, &vtProp, 0, 0);

        if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) commandLine = vtProp.bstrVal; // Сохраняем результат

        VariantClear(&vtProp);
        pclsObj->Release();
    }
    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();

    return commandLine;
}

wstring sanitizePath(const wstring& input, typeEmu type, int& instance) {
    wstring result;
    result.reserve(input.size());

    wstring temp;
    for (wchar_t ch : input) {
        if (ch == L'"') continue;
        else temp += ch;
    }

    switch (type) {
        case typeEmu::ld_player: {
        // Заменяем dnplayer на ldconsole
        size_t pos = temp.find(L"dnplayer");
        if (pos != wstring::npos) temp.replace(pos, wcslen(L"dnplayer"), L"ldconsole");

        // Извлекаем число между = и |
        wregex pattern(L"index=(\\d+)\\|");
        wsmatch match;
        if (regex_search(temp, match, pattern)) {
            instance = stoi(match[1].str()); // Преобразуем найденное число в int
            temp = regex_replace(temp, pattern, L""); // Удаляем "index=1|" из строки
        } else {
            instance = 0; // Указываем ошибку, если число не найдено
            return L"";
        }
        result = temp;
        return result;
        }
        case typeEmu::nox_app: {
            return L"";
        }
        case typeEmu::memu_app: {
            return L"";
        }
        case typeEmu::mg_launcher: {
            return L"";
        }
    }

    return L"";
}

bool FindEmulator(const QString &windowName, HWND *main, HWND *game) {
    constexpr int INITIAL_DELAY_MS = 500;
    constexpr int GAME_DELAY_MS = 1000;
    Sleep(INITIAL_DELAY_MS);
    int x = 0;
    while (!(*main = FindWindowA(NULL, windowName.toUtf8().constData()))) {
        Sleep(INITIAL_DELAY_MS);
        x++;
        if (x >= 100) return false;
    }
    x = 0;
    while (!(*game = FindWindowExA(*main, NULL,"RenderWindow",NULL))) {
        Sleep(GAME_DELAY_MS);
        x++;
        if (x >= 300) return false;
    }
    // Установка параметров окна
    LONG_PTR style = GetWindowLongPtr(*main, GWL_EXSTYLE);
    style |= WS_POPUP | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    SetWindowLongPtr(*main, GWL_STYLE, style);
    ShowWindow(*main, SW_MINIMIZE);
    if (!SetWindowPos(*main, HWND_BOTTOM, 1, 1, 1, 1, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED)) return false;
    Sleep(INITIAL_DELAY_MS);
    ShowWindow(*main, SW_SHOWNOACTIVATE);
    if (!SetWindowPos(*main, HWND_BOTTOM, 1, 1, 900, 600+34, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED)) return false;
    Sleep(INITIAL_DELAY_MS);
    if (!SetWindowPos(*game, HWND_BOTTOM, 0, 34, 900, 600, SWP_NOZORDER | SWP_NOACTIVATE |SWP_FRAMECHANGED)) return false;
    Sleep(INITIAL_DELAY_MS);
    return true;
}
