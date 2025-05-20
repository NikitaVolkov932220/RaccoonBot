#ifndef LDPLAYER_H
#define LDPLAYER_H

#include "Emulator.h"
#include <QObject>

class BOTLIB_EXPORT LDPlayer : public Emulator
{
public:
    LDPlayer();
    void Start(bool *start) override;
    void Stop(HWND *main) override;
    void Initialize(HWND *main, HWND *game, typeEmu *type) override;

    LDPlayer& operator=(const Emulator &other);

    void FixSize(HWND *main, HWND *game) override;
    void FixPos(HWND *main, HWND *game) override;
    void FixAll(HWND *main, HWND *game) override;
};

#endif // LDPLAYER_H
