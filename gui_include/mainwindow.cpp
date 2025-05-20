#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("RaccoonBot");
    resize(390,400);
    setFixedSize(390,400);
    setCentralWindow(390,400);

    //Инициализация
    user = new userProfile();
    ocr = new Ocr((path)"G:\\Coding\\Photo\\test\\numbers");
    ocr->Initialize();
    controller = new Controller();
    cathedral = nullptr;
    cathedralSettings = nullptr;
    controllerThread = new QThread(this);
    errorLog = new QTextEdit();
    mainWidget = new QWidget(this);
    controller->moveToThread(controllerThread);
    ocr->moveToThread(controllerThread);
    setCentralWidget(mainWidget);

    tabWidget = new QTabWidget(mainWidget);
    QVBoxLayout* layout = new QVBoxLayout(mainWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(tabWidget);

    // Меню-бары
    QMenuBar* menuBar = new QMenuBar(mainWidget);

    //Аккаунты
    QMenu* generalMenu = new QMenu("Главная", mainWidget);

    QAction* addAccountAction = new QAction("Добавить аккаунт", mainWidget);
    QAction* removeAccountAction = new QAction("Удалить аккаунт", mainWidget);
    QAction* statisticAccountAction = new QAction("Статистика",mainWidget);
    QAction* saveAccountSetting = new QAction("Сохранить настройки",mainWidget);
    QAction* loadAccountSetting = new QAction("Загрузить настройки",mainWidget);

    generalMenu->addAction(addAccountAction);
    generalMenu->addAction(removeAccountAction);
    generalMenu->addAction(statisticAccountAction);
    generalMenu->addAction(saveAccountSetting);
    generalMenu->addAction(loadAccountSetting);

    //О программе

    QMenu* aboutProgram = new QMenu("О программе",mainWidget);

    //Инструменты бота
    QMenu* botMenu = new QMenu("Инструменты бота",this);

    QAction* clearLog = new QAction("Очистка лога",this);
    QAction* eyeBot = new QAction("Зрение бота",this);
    QAction* screenBot = new QAction("Сделать скриншот эмулятора",this);
    QAction* sizeFix = new QAction("Исправить размер эмулятора",this);
    QAction* winFix = new QAction("Исправить масштаб экрана",this);
    QAction* posFix = new QAction("Исправить местоположение эмулятора",this);
    QAction* fullFix = new QAction("Полное исправление бота с перезагрузкой игры",this);
    QAction* settingBot = new QAction("Настройки бота",this);

    botMenu->addAction(clearLog);
    botMenu->addAction(eyeBot);
    botMenu->addAction(screenBot);
    botMenu->addAction(sizeFix);
    botMenu->addAction(winFix);
    botMenu->addAction(posFix);
    botMenu->addAction(fullFix);
    botMenu->addAction(settingBot);//Там будет автозапуск, вырубание бота при бездействии и всякая хрень

    //Связь со мной
    QMenu* linkMenu = new QMenu("Связь со мной", this);

    QAction* TgChannel = new QAction("Телеграмм-канал бота",this);
    QAction* DsChannel = new QAction("Дискорд чат(недоступен в РФ)",this);

    linkMenu->addAction(TgChannel);
    linkMenu->addAction(DsChannel);

    // Лицензия программы
    QAction* myAbout = new QAction("Лицензии программы",mainWidget);

    aboutProgram->addMenu(botMenu);
    aboutProgram->addMenu(linkMenu);
    aboutProgram->addAction(myAbout);

    //Прочие действия без меню
    QAction* manual = new QAction("Инструкция",this);
    QAction* bugReport = new QAction("Сообщить об ошибке",this);


    menuBar->addMenu(generalMenu);
    menuBar->addAction(manual);
    menuBar->addMenu(aboutProgram);
    menuBar->addAction(bugReport);

    setMenuBar(menuBar);

    createGUI();

    errorLog->resize(600,600);
    errorLog->show();

    connect(controller, &Controller::errorLogging, this, &MainWindow::appendToErrorLog, Qt::QueuedConnection);
    connect(controller,&Controller::Recognize,ocr,&Ocr::Recognize);
    connect(this,&MainWindow::startController,controller,&Controller::Start);
}

MainWindow::~MainWindow() {
    controllerThread->quit();
    //аккуратно следить за памятью
}

void MainWindow::setCentralWindow(int width, int height){
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();

    int x = (screenGeometry.width() - width) / 2;
    int y = (screenGeometry.height() - height) / 2;

    this->move(x, y);
}

void MainWindow::createGUI() {
    QWidget* widget = new QWidget(tabWidget);

    // createAccount(widget);
    QVBoxLayout* layout = new QVBoxLayout(widget);
    QTabWidget* taskTabWidget = new QTabWidget(widget);

    // Добавление 4 poka chto под-вкладок
    for (int i = 0; i < 4; ++i) {
        QWidget* tab = new QWidget(taskTabWidget);
        switch(i) {
        case 0: {
            createGeneralTab(tab);
            taskTabWidget->addTab(tab, "Главная");
            break;
        }
        case 1:{
            createMainTab(tab);
            taskTabWidget->addTab(tab, "Общее");
            break;
        }
        case 2:{
            createCathedralTab(tab);
            taskTabWidget->addTab(tab, "Собор");
            break;
        }
        case 3:{
            createLoggerTab(tab);
            taskTabWidget->addTab(tab, "Лог-вкладка");
            break;
        }

        }
    }
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(taskTabWidget);
    widget->setLayout(layout);

    tabWidget->addTab(widget, "Новый аккаунт");
}

void MainWindow::createGeneralTab(QWidget *tab) {
    accountInfo = new QWidget(tab); // получение виджета инфы об аккаунте
    user->getInfo(accountInfo); // userProfile user
    accountInfo->setGeometry(0,0,380,200);

    DynamicComboBox* listEmulators = new DynamicComboBox(tab);
    listEmulators->setGeometry(5, 210, 250, 25);

    //через сигналы обновлять буду
    QLabel* statusLabel = new QLabel(tab);
    statusLabel->setText("Статус бота: Отключен");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setGeometry(5,245,250,25);

    QPushButton* startButton = new QPushButton("Старт",tab);
    startButton->setGeometry(5,280,120,30);
    startButton->setStyleSheet("QPushButton {"
                               "background-color:gray;"
                               "color:white;"
                               "margin: 0px;"
                               "padding: 0px;"
                               "border: none;"
                               "text-align: center;"
                               "}");
    startButton->setEnabled(false);

    QPushButton* stopButton = new QPushButton("Стоп",tab);
    stopButton->setGeometry(135,280,120,30);
    stopButton->setStyleSheet("QPushButton {"
                              "background-color:gray;"
                              "color:white;"
                              "margin: 0px;"
                              "padding: 0px;"
                              "border: none;"
                              "text-align: center;"
                              "}");
    stopButton->setEnabled(false);

    QLabel* labelRaccoon = new QLabel(tab);
    labelRaccoon->setGeometry(255,195,130,130);
    labelRaccoon->setPixmap(QPixmap(":/pages/what.png"));

    connect(controller,&Controller::Logging,this,[=](const QString& msg){
        statusLabel->setText("Статус бота: " + msg);
    });

    connect(listEmulators, &DynamicComboBox::currentIndexChanged, this, [=](int index){
        if (index > 0 && index < listEmulators->count()) {
            user->emulator_name = listEmulators->itemText(index);
            startButton->setEnabled(true);
            startButton->setStyleSheet("background-color:green; color:white;");
        }
        else {
            startButton->setEnabled(false);
            startButton->setStyleSheet("background-color:gray; color:white;");
            stopButton->setEnabled(false);
            stopButton->setStyleSheet("background-color:gray; color:white;");
        }
    });

    connect(startButton,&QPushButton::clicked,this,[=](){
        user->bot_ID = 0;
        labelRaccoon->setPixmap(QPixmap(":/pages/anger.png"));
        startButton->setEnabled(false);
        startButton->setStyleSheet("background-color:gray; color:white;");
        stopButton->setEnabled(true);
        stopButton->setStyleSheet("background-color:red; color:white;");
        if(cathedral == nullptr) {
            cathedral = new Cathedral(controller);
            connect(this,&MainWindow::initCathedral,cathedral,&Task::Initialize);
            connect(this,&MainWindow::startCathedral,cathedral,&Task::Start);
        }
        cathedral->moveToThread(controllerThread);
        controllerThread->start();
        emit startController(user,nullptr);
    });

    connect(controller,&Controller::endStart,this,[=](){
        // Удаляем старый виджет
        if (accountInfo) {
            accountInfo->hide();
            accountInfo->deleteLater();
            accountInfo = nullptr;
        }
        // Создаем новый
        accountInfo = new QWidget(tab);
        accountInfo->setGeometry(0,0,380,200);
        user->getInfo(accountInfo);
        accountInfo->show();

        if(user->subscribe == typeSub::admin) labelRaccoon->setPixmap(QPixmap(":/pages/cute.png"));

        //if(user->subscribe == typeSub::admin) errorLog->show();
        //пока что для теста собора так, потом через QVector все сделать адекватненько
        if(cathedralSettings == nullptr){
            cathedralSettings = new CathedralSettings();
            cathedralSettings->history_power = user->history_power;
            cathedralSettings->count = 1;
            cathedralSettings->premiumStatus = true;
            cathedralSettings->modeDifficult = 1;//hard 10 tz
            cathedralSettings->modeSquad = 1;
        }
        emit initCathedral(cathedralSettings,nullptr);
        emit startCathedral();
    },Qt::QueuedConnection);

    connect(stopButton,&QPushButton::clicked,this,[=]{
        labelRaccoon->setPixmap(QPixmap(":/pages/what.png"));
        statusLabel->setText("Cтатус бота: Бот остановлен");
        startButton->setEnabled(true);
        startButton->setStyleSheet("background-color:green");
        stopButton->setEnabled(false);
        stopButton->setStyleSheet("background-color:gray");
        //controller->Stop();
        cathedral->Stop();
        delete cathedralSettings;
        cathedral->deleteLater();
        cathedralSettings = nullptr;
        cathedral = nullptr;
    });
}

void MainWindow::createMainTab(QWidget *tab) {
    QListWidget* taskWidget = new QListWidget(tab);
    //тут загрузка с файла будет, пока что пустой
    taskWidget->setGeometry(5,5,160,255);

    QLabel* nameFile = new QLabel("Название сценария",tab);
    nameFile->setAlignment(Qt::AlignCenter);
    nameFile->setGeometry(5,265,160,20);

    QComboBox* listFiles = new QComboBox(tab);
    listFiles->addItem("standart.придумаю");
    listFiles->setGeometry(5,290,180,25);

    QPushButton* addTask = new QPushButton("Добавить\n"
                                           "задание",tab);
    addTask->setGeometry(175,0,95,40);

    QPushButton* removeTask = new QPushButton("Удалить\n"
                                              "задание",tab);
    removeTask->setGeometry(280,0,95,40);

    QWidget* subTab = new QWidget(tab);
    subTab->setGeometry(175,50,200,230);

    QVBoxLayout* layout = new QVBoxLayout(subTab);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QCheckBox* adsTask = new QCheckBox("Просмотр реклам ресурсов",subTab);
    QCheckBox* resourcesTask = new QCheckBox("Сбор ресурсов и эмоций",subTab);
    QCheckBox* invasionTask = new QCheckBox("Прохождение вторжений",subTab);
    QCheckBox* giftTask = new QCheckBox("Отправка и получение"
                                        "\nподарков друзей",subTab);
    QCheckBox* dailyTask = new QCheckBox("Забирать сундук\n"
                                         "ежедневных заданий",subTab);
    QWidget* taker = new QWidget(subTab);
    taker->setFixedSize(210,20);
    QLabel* takeTask = new QLabel("Запрос ресурсов",taker);
    takeTask->setAlignment(Qt::AlignCenter);
    takeTask->setGeometry(0,0,160,20);
    QComboBox* takeBox = new QComboBox(taker);
    takeBox->addItem("0");
    takeBox->setGeometry(150,0,50,20);
    QCheckBox* tourTask = new QCheckBox("Отправка путешествий",subTab);
    QCheckBox* foodTask = new QCheckBox("Открывать сундуки с едой",subTab);

    layout->addWidget(adsTask);
    layout->addWidget(resourcesTask);
    layout->addWidget(invasionTask);
    layout->addWidget(giftTask);
    layout->addWidget(dailyTask);
    layout->addWidget(taker);
    layout->addWidget(tourTask);
    layout->addWidget(foodTask);
    subTab->setLayout(layout);

    QPushButton* addFile = new QPushButton("Сохранить",tab);
    addFile->setGeometry(195,290,85,25);

    QPushButton* loadFile = new QPushButton("Загрузить",tab);
    loadFile->setGeometry(290,290,85,25);
}

void MainWindow::createCathedralTab(QWidget *tab) {
    QCheckBox* adsTask = new QCheckBox("Смотреть рекламу",tab);
    adsTask->setGeometry(5,0,175,20);

    QCheckBox* chestTask = new QCheckBox("Автооткрытие сундуков",tab);
    chestTask->setGeometry(190,0,185,20);

    QRadioButton* bestButton = new QRadioButton("Выставить лучших",tab);
    bestButton->setGeometry(5,25,175,20);

    QRadioButton* lastButton = new QRadioButton("Предыдущий отряд",tab);
    lastButton->setGeometry(5,50,175,20);

    QRadioButton* barrackButton = new QRadioButton("Отряд с казармы",tab);
    barrackButton->setGeometry(5,75,175,20);

    QButtonGroup* squad = new QButtonGroup(tab);
    squad->addButton(bestButton);
    squad->addButton(lastButton);
    squad->addButton(barrackButton);

    QRadioButton* fullButton = new QRadioButton("Полное прохождение",tab);
    fullButton->setGeometry(190,25,185,20);

    QRadioButton* fastButton = new QRadioButton("Боссы и миньоны",tab);
    fastButton->setGeometry(190,50,185,20);

    QButtonGroup* mode = new QButtonGroup(tab);
    mode->addButton(fullButton);
    mode->addButton(fastButton);

    QLabel* modeLabel = new QLabel("Сложность",tab);
    //modeLabel->setAlignment(Qt::AlignCenter);
    modeLabel->setGeometry(190,75,90,20);

    QComboBox* modeBox = new QComboBox(tab);
    modeBox->setGeometry(275,75,100,20);
    modeBox->addItem("Нормальная");
    modeBox->addItem("Сложная");
    modeBox->addItem("Кошмарная");

    QLabel* resourceLabel = new QLabel("Расходовать",tab);
    //resourceLabel->setAlignment(Qt::AlignCenter);
    resourceLabel->setGeometry(190,100,90,20);

    QComboBox* resourceBox = new QComboBox(tab);
    resourceBox->setGeometry(275,100,100,20);
    resourceBox->addItem("Яблоки");
    resourceBox->addItem("Ключи");
    resourceBox->addItem("Яблоки>Ключи");
    resourceBox->addItem("Ключи>Яблоки");

    QLabel* countLabel = new QLabel("Количество походов",tab);
    //countLabel->setAlignment(Qt::AlignCenter);
    countLabel->setGeometry(190,125,150,20);

    QComboBox* countBox = new QComboBox(tab);
    countBox->setGeometry(335,125,40,20);
    countBox->addItem("1");
    countBox->addItem("2");
    countBox->addItem("3");
    countBox->addItem("4");
    countBox->addItem("5");
    countBox->addItem("7");
    countBox->addItem("10");
    countBox->addItem("20");
    countBox->addItem("30");
    countBox->addItem("∞");

    QLabel* foodLabel = new QLabel("Сохранять еды",tab);
    foodLabel->setGeometry(190,150,130,20);

    QLineEdit* foodLine = new QLineEdit(tab);
    foodLine->setGeometry(305,150,70,20);
    foodLine->setText("0");
    QIntValidator* foodValid = new QIntValidator(0, 2000000,tab);
    foodLine->setValidator(foodValid);
}

void MainWindow::createLoggerTab(QWidget *tab) {
    user->logger = new QTextEdit(tab);
    user->logger->setGeometry(5,5,372,307);
    connect(controller,&Controller::Logging,[=](const QString& msg){
        user->logger->append("[" + QDateTime::currentDateTime().toString("HH:mm:ss.zzz") + "] " + msg);
    });
}

void MainWindow::appendToErrorLog(const QString& msg) {
    errorLog->append(msg);
}
