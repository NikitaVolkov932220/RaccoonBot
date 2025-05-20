#ifndef EMULATOR_H
#define EMULATOR_H

#include "BotLib_global.h"
#include "EmulatorComponents.h"
#include "Error.h"

class BOTLIB_EXPORT Emulator : public QObject{
    Q_OBJECT
public:
    virtual ~Emulator() = default;
    virtual void Start(bool* start) = 0; // запуск эмулятора
    virtual void Stop(HWND* main) = 0;
    virtual void Initialize(HWND* main, HWND* game, typeEmu *type) = 0; // main обязан быть непустым

    virtual void FixSize(HWND* main, HWND* game) = 0; // для каждого эмулятора свои цифры-фиксеры
    virtual void FixPos(HWND* main, HWND* game) = 0;
    virtual void FixAll(HWND* main, HWND* game) = 0;

signals:
    void Logging(const QString& msg);

protected:
    wstring cmd;
    int instance;
};

#endif // EMULATOR_H
