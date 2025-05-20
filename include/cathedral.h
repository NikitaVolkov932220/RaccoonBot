#ifndef CATHEDRAL_H
#define CATHEDRAL_H

#include "Task.h"
#include "TaskSettings.h"

class BOTLIB_EXPORT Cathedral : public Task
{
public:
    Cathedral(Controller *g_controller);
    ~Cathedral();
    void Start(bool *result = nullptr) override;
    void Initialize(TaskSettings *setting, bool *result = nullptr) override;
    void Stop() override;
    void checkPower(const Mat &object, bool *result = nullptr) override;
    void setUnitsSet(bool *result = nullptr) override;//
    void confirmSquad(bool *result = nullptr);
    void checkStage(bool *result = nullptr);
    void checkSettings(bool *result = nullptr);
    void checkWarnings(bool *result = nullptr);
    void findWaypoint(bool *result = nullptr);
    void checkWaypoints(int &type, bool *result = nullptr);
    void attackWaypoints(int type, bool *result = nullptr);
    void fullGamePass(bool *result = nullptr);
    void safePower(bool *result = nullptr);//это при выставлении юнитов в mode собственный отряд
    void bossGamePass(bool *result = nullptr);//
private:
    QStringList nameWaypoints;
    CathedralSettings* settings;
    int currentStage;
};

#endif // CATHEDRAL_H
