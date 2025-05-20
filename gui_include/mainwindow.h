#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTextEdit>
#include <QVector>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QGuiApplication>
#include <QScreen>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QListWidget>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLineEdit>
#include <QIntValidator>
#include <QThread>
#include <QDateTime>

#include "dynamiccombobox.h"
#include "Structs.h"
#include "Controller.h"
#include "Ocr.h"
#include "TaskSettings.h"
#include "cathedral.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setCentralWindow(int width, int height);

    void createGUI();

    //tab
    void createMainTab(QWidget* tab);
    void createGeneralTab(QWidget* tab);
    void createCathedralTab(QWidget* tab);

    void createLoggerTab(QWidget* tab);

    void appendToErrorLog(const QString& msg);

signals:
    void startBot(); // по этому сигналу со всех страниц все настройки впихиваются в структуры или что-то такое
    void startController(userProfile* user,bool* result = nullptr); // хз временно или нет
    void initCathedral(TaskSettings *setting, bool *result = nullptr);
    void startCathedral(bool *result = nullptr);
private:
    QWidget* mainWidget;

    QTabWidget *tabWidget;
    QVector<QTextEdit*> userLoggers;

    userProfile *user;
    QWidget *accountInfo;
    Controller *controller;
    QTextEdit *errorLog;
    Ocr *ocr;
    CathedralSettings *cathedralSettings;
    Task *cathedral;

    QThread *controllerThread;
};


#endif // MAINWINDOW_H
