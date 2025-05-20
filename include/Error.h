#ifndef ERROR_H
#define ERROR_H
#include "BotLib_global.h"
#include <QDateTime>
class BOTLIB_EXPORT m_error : public QObject { // подключать к логгеру ошибок
    Q_OBJECT
public:
    explicit m_error(bool* result = nullptr, QObject* parent = nullptr): QObject(parent), ptr(result) {}
    ~m_error() {
        if(ptr) *ptr = value;
        if(errorMessage != "") emit Logging("[" + QDateTime::currentDateTime().toString("HH:mm:ss.zzz") + "] " + errorMessage);
    }
    bool* ptr = nullptr;
    bool value = true;
    QString errorMessage;
signals:
    void Logging(const QString &msg);
};
#endif // ERROR_H
