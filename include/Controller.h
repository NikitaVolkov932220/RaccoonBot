#ifndef CONTROLLER_H
#define CONTROLLER_H


#include "BotLib_global.h"
#include "Structs.h"
#include "EmulatorComponents.h"
#include "Error.h"
#include <QDir>
#include <QFile>

class BOTLIB_EXPORT Controller : public QObject {
    Q_OBJECT
public:
    Controller();
    ~Controller() = default;

    //Для тестов(потом удалить)
    void InitLight(userProfile user);
    /////
    /// \brief image
    ///
    void findObject(const Mat *finder = nullptr, bool *result = nullptr);
    void compareObject(double rightVal = 0.02, const Mat *object = nullptr, const Mat *sample = nullptr, bool *result = nullptr);
    void compareSample(const QString &pagePath, const QString &samplePath, const QString &maskPath, bool *result = nullptr, bool shoot = false, double rightVal = 0.02);
    void saveImage(const QString &savePath, const Mat &saveImage, bool *result = nullptr);
    void Screenshot();
    void convertImage(const QImage &imageOne, Mat *imageTwo, bool *result = nullptr);
    Mat cutImage();
    Mat getMatObject();
    void setMatObject(const Mat &image, bool *result = nullptr);
    void changeColor(const Mat &before, Mat *after, bool *result = nullptr);
    void setSample(const Mat &sample, bool *result = nullptr);
    void setSample(const QString &sample, bool *result = nullptr);
    void setMask(const Mat &mask, bool *result = nullptr);
    void setMask(const QString &mask, bool *result = nullptr);
    Rect& getRect();
    /////
    /// \brief emulator
    ///
    void isEmpty(bool *result = nullptr);
    void isValidSize(bool *result = nullptr);
    void isValidPos(bool *result = nullptr);
    /////
    /// \brief keyboard + mouse
    ///
    void click(bool *result = nullptr, int count = 1, int delay = 500);
    void clickPosition(const Rect &point, bool *result = nullptr, int count = 1, int delay = 500);
    void clickSwipe(const Rect &start,const Rect &finish, bool *result = nullptr);
    void clickButton(const QString &pagePath,const QString &buttonName, bool *result = nullptr, int count = 1, int delay = 500);
    void clickEsc(bool *result = nullptr);
    void clickReturn(bool *result = nullptr);
    /////
    /// \brief game
    ///
    //void Initialize(); // функция будет из файла подтягивать юзеров
    void userInitialize(userProfile *user, bool *result = nullptr);
    void fixGameError(bool *result = nullptr);
    void getGameError();
    void refreshMainPage(bool *result = nullptr);// доделать, не распознает правильно
    void skipEvent();
    ////
    /// \brief barracks
    ///
    void findBarracks(bool *result = nullptr);//
    void entryBarracks(bool *result = nullptr);//
    void scanSquadCount(userProfile *user, bool *result = nullptr);//
    ///
    /// \brief game checker доделать
    ///
    void checkMap(bool *result = nullptr);//сделать
    void checkLoading();
    void checkGameLoading();
    void checkMainPage(bool *result = nullptr);  // добавить проверку на гоблина с тыблоками
    void checkPreMainPage();
    void checkEvent(bool *result = nullptr);
    void checkSettings(bool *result = nullptr);
    /////
    /// \brief microTasks вынести в отдельный класс наследник от тасков обычных
    ///
    //void takeEmojis(bool *result = nullptr);
    /////
    /// \brief path
    ///
    QString &getMainPath();
    void setMainPath(const QString &path);
    /////
    /// \brief controller
    ///
    void Start(userProfile *user, bool *result = nullptr); // тут настройки проверяются в игре и сама игра дописать
    void Stop(bool *result = nullptr);
    void LocalLogging(const QString& msg); // внутри тупо emit errorLogging();
signals:
    void Recognize(const Mat &object, int &number);
    void emulatorStart(bool *result = nullptr);
    void emulatorStop(HWND *main);
    void emulatorFix(HWND *main, HWND *game);
    void Logging(const QString &msg);
    void errorLogging(const QString& msg);
    void endStart();
private:
    HWND m_main, m_game;
    Mat m_object, m_sample, m_mask;
    Rect m_rect; //при поиске в нём корды
    QString mainPath;
};

#endif // CONTROLLER_H
