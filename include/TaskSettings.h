#ifndef TASKSETTINGS_H
#define TASKSETTINGS_H
#include "BotLib_global.h"
#include "Structs.h"

struct BOTLIB_EXPORT TaskSettings {
    int history_power = 0;
    int count = 1;
    bool watchADS = false;
    bool premiumStatus = false;
    virtual void loadSettings(const QString &filePath) = 0;
    virtual void saveSettings(const QString &filePath) = 0;
};

struct BOTLIB_EXPORT CathedralSettings : public TaskSettings {
    int modeDifficult = 0; // 0 - easy, 1 - hard, 2 - insane
    int modeKey = 1; // 0 - apple, 1 - Key, 2 - apple > Key, 3 - Key > apple
    int modeSquad = 0; // 0 - better unit, 1 - last squad, 2 - my squad
    bool fullGamePass = true;
    squad m_squad;
    bool openChest = false;
    int saveApple = 0;

    void loadSettings(const QString &filePath) override {}
    void saveSettings(const QString &filePath) override {}
};

#endif // TASKSETTINGS_H
