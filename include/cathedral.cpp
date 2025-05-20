#include "cathedral.h"

Cathedral::Cathedral(Controller *g_controller) {
    settings = nullptr;
    controller = g_controller;
    localPath = g_controller->getMainPath();
    nameWaypoints << "altar" << "camp" << "game" << "reflection";
    currentStage = 0;
    stop_flag = false;
}
Cathedral::~Cathedral() {}

void Cathedral::Initialize(TaskSettings *setting, bool *result) {
    emit controller->Logging("Инициализация настроек собора.");
    m_error err(result);
    connect(&err, &m_error::Logging, controller, &Controller::LocalLogging);
    if (!setting) {
        err.value = false;
        err.errorMessage = "Передан нулевой указатель на настройки.";
        return;
    }
    if(auto* cathedral = dynamic_cast<CathedralSettings*>(setting)){
        settings = cathedral;
    }
    else {
        err.value = false;
        err.errorMessage = "Неправильный тип настроек подан на вход.";
        emit controller->Logging("Произошла ошибка. Отправьте отчёт об ошибке.");
        settings = nullptr;
    }
    return;
}

void Cathedral::Stop() {
    stop_flag = true;
    controller->errorLogging("Остановка задания [Собор].");
}

void Cathedral::Start(bool *result) {
    emit controller->Logging("Задание [Собор] начато.");
    m_error err(result);
    connect(&err, &m_error::Logging, controller, &Controller::LocalLogging);
    bool l_result = false;
    controller->checkMainPage(&l_result);
    if(!l_result){
        err.value = false;
        err.errorMessage = "Фиг знает где мы. Вызывайте чинилыча.";
        return;
    }
    controller->clickButton("main","button_map");
    controller->checkMap(&l_result);
    if(!l_result){
        err.value = false;
        err.errorMessage = "Карта не открылась.";
        return;
    }
    controller->clickButton("map","button_dark");
    int k = 0;
    while(!stop_flag) {
        currentStage = 0;
        checkStage();
        if(currentStage == 0) {
            checkSettings(&l_result);
            if(l_result) {
                controller->clickButton("dark","button_start");
                confirmSquad(&l_result);
                if(!l_result){
                    err.value = false;
                    err.errorMessage = "Произошла какая-то ошибка в момент подтверждения отряда.";
                    return;
                }
            }
            else {
                err.value = false;
                controller->Logging("Задание прервано. Ошибка в настройках");
                return;
            }
            checkStage(&l_result);
            if(!l_result){
                err.value = false;
                err.errorMessage = "Этаж не обнаружен";
                return;
            }
        }
        if(settings->fullGamePass) fullGamePass(&l_result);
        else bossGamePass(&l_result);
        Sleep(1000);
        controller->compareSample("dark","sample_end","compare_end",&l_result,true);
        if(l_result) {
            controller->clickButton("dark","button_end",&l_result,2);
            Sleep(500);
        }
        if(!l_result) {
            err.value = false;
            err.errorMessage = "Не удалось завершить собор: Не нажалась кнопка";
            return;
        }
        controller->checkLoading();
        controller->checkEvent(&l_result);
        if(l_result) controller->skipEvent();
        k++;
        if(k == settings->count) stop_flag = true;
    }
}

void Cathedral::checkPower(const Mat &object, bool *result) {
    emit controller->errorLogging("Проверка мощи");
    m_error err(result);
    connect(&err, &m_error::Logging, controller, &Controller::LocalLogging);
    Mat l_object;
    bool l_result = false;
    controller->changeColor(object,&l_object,&l_result);
    if(!l_result) {
        err.value = false;
        return;
    }
    int power = 0;
    emit controller->Recognize(l_object,power);
    if(power <= 0) {
        err.value = false;
        err.errorMessage = "Не удалось распознать мощь: " + QString::number(power);
        imshow("1",l_object);
        return;
    }
    if(power > settings->history_power) {
        controller->clickSwipe({468,194,0,0},{467,394,0,0},&l_result);
        if(!l_result) {
            err.value = false;
            err.errorMessage = "Возможное повышение ИМ, запрет атаки";
            return;
        }
    }
    return;
}

void Cathedral::confirmSquad(bool *result) {
    emit controller->errorLogging("Утверждение отряда");
    m_error err(result);
    connect(&err, &m_error::Logging, controller, &Controller::LocalLogging);
    bool l_result = false;
    while(!l_result) {
        controller->compareSample("squad/dark","sample","compare",&l_result,true);
        if(!l_result) Sleep(1000);
    }
    switch (settings->modeSquad) {
        case 0: {
            controller->clickButton("squad/dark","button_best",&l_result,2);
            break;
        }
        case 1: {
            controller->clickButton("squad/dark","button_previous",&l_result,2);
            break;
        }
        case 2: {
            break;
        }
        default:{
            err.value = false;
            err.errorMessage = "Неправильная настройка.";
            return;
        }
    }
    controller->clickButton("squad/dark","button_start",&l_result,2);
    checkWarnings(&l_result);
    if(!l_result) {
        err.value = false;
        err.errorMessage = "Не удалось начать собор.";
        return;
    }
}

void Cathedral::checkStage(bool *result) {
    if (currentStage <= 0 && currentStage >= 4) return;
    emit controller->errorLogging("Проверка этажа");
    m_error err(result);
    connect(&err, &m_error::Logging, controller, &Controller::LocalLogging);
    bool l_result = false;

    int x = 0;
    while(x < 3) {
        controller->checkLoading();
        controller->compareSample("dark/waypoints/blessing","sample","compare",&l_result,true);
        if(l_result) {
            controller->clickButton("dark/waypoints/blessing","button" + QString::number(1 + rand() % 3));
            controller->clickButton("dark/waypoints/blessing","button_confirm");
        }
        controller->Screenshot();
        for(int i = 0; i < 3; i++) {
            controller->compareSample("dark/waypoints","stage_" + QString::number(i+1),
                                      "compare_stage",&l_result,false,0.03);
            if(l_result) {
                currentStage = i+1;
                return;
            }
        }
        x++;
    }
    err.value = false;
    err.errorMessage = "Любой этаж собора не был найден";
    return;
}

void Cathedral::checkSettings(bool *result) {
    emit controller->errorLogging("Проверка настроек");
    m_error err(result);
    connect(&err, &m_error::Logging, controller, &Controller::LocalLogging);
    bool l_result = false;
    if(settings->history_power < 1) {
        err.value = false;
        err.errorMessage = "Неправильные настройки. ИМ < 1";
        return;
    }
    switch(settings->modeDifficult) {
    case 0:{
        controller->compareSample("dark","sample","state_normal",&l_result,true);
        if(!l_result) controller->clickButton("dark","button_normal");
        break;
    }
    case 1:{
        controller->compareSample("dark","sample","state_hard",&l_result,true);
        if(!l_result) controller->clickButton("dark","button_hard");
        break;
    }
    case 2:{
        controller->compareSample("dark","sample","state_insane",&l_result,true);
        if(!l_result) controller->clickButton("dark","button_insane");
        break;
    }
    default:{
        err.value = false;
        err.errorMessage = "Неправильные настройки сложности";
        return;
    }
    }
    switch(settings->modeKey) {
    case 0:{
        controller->compareSample("dark","sample","state_apples",&l_result);
        if(!l_result) controller->clickButton("dark","button_apples");
        break;
    }
    case 1:{
        controller->compareSample("dark","sample","state_keys",&l_result);
        if(!l_result) controller->clickButton("dark","button_keys");
        break;
    }
    case 2://{пока что не работает}
    case 3://туда же
    default:{
        err.value = false;
        err.errorMessage = "Неправильные настройки расходников";
        return;
    }
    }
}

void Cathedral::checkWarnings(bool *result) {
    emit controller->errorLogging("Проверка варнингов");
    m_error err(result);
    connect(&err, &m_error::Logging, controller, &Controller::LocalLogging);
    bool l_result = false;
    int x = 0;
    while(x < 4) {
        controller->compareSample("warnings/dark","sample_1","compare",&l_result,true,0.01);
        if(!l_result){
            controller->compareSample("warnings/dark","sample_2","compare",&l_result,false,0.01);
            if(!l_result) {
                x++;
                Sleep(500);
                continue;
            }
        }
        controller->clickButton("warnings/dark","button_yes");
        return;
    }
    err.value = false;
    err.errorMessage = "Ошибка распознавания варнинга";
    return;
}

void Cathedral::findWaypoint(bool *result) {
    emit controller->errorLogging("===Поиск точки===");
    m_error err(result);
    connect(&err, &m_error::Logging, controller, &Controller::LocalLogging);
    bool l_result = false;
    int x = 0;
    while(x<3) {
        Sleep(500);
        controller->Screenshot();
        controller->setSample(controller->getMatObject(),&l_result);
        if(!l_result) {
            err.value = false;
            return;
        }
        for(int i = 0; i < 7; i++) {
            Mat l_object;
            controller->convertImage(QImage((localPath + "/dark/map/sample_" + QString::number(i) + ".png")), &l_object,&l_result);
            if(!l_result) {
                err.value = false;
                err.errorMessage = "Конвертация " + (localPath + "/dark/map/sample_ " + QString::number(i) + ".png") + " failed";
                return;
            }
            controller->setMatObject(l_object,&l_result);
            if(!l_result) {
                err.value = false;
                return;
            }
            controller->setMask("dark/map/find_" + QString::number(i));
            controller->findObject(nullptr,&l_result);
            if(!l_result){
                err.value = false;
                return;
            }
            Mat temp = controller->cutImage();
            controller->compareObject(0.12,&temp,nullptr,&l_result);//0.12?
            if(l_result) return;
        }
        x++;
    }
    controller->compareSample("dark/waypoints","sample_next","state_next",&l_result,true);
    if(l_result){
        controller->clickButton("dark/waypoints","button_next",&l_result);
        checkWarnings(&l_result);
        err.value = false;
        return;
    }
    controller->compareSample("dark/waypoints","sample_end","state_end",&l_result);
    if(l_result) {
        controller->clickButton("dark/waypoints","button_end",&l_result);
        checkWarnings(&l_result);
        currentStage = 4;
        err.value = false;
        return;
    }
    err.value = false;
    err.errorMessage = "Не удалось найти точку для атаки или кнопки переходов.";
    return;
}

void Cathedral::checkWaypoints(int &type, bool *result) {
    emit controller->errorLogging("Распозонавание точки");
    m_error err(result);
    connect(&err, &m_error::Logging, controller, &Controller::LocalLogging);
    bool l_result = false;
    int x = 0;
    while (x < 10){
        emit controller->errorLogging("check waypoint: battle/dark");
        controller->compareSample("battle/dark","sample","compare",&l_result,true); // для баттла 0.04? вроде как 0.006 хватает
        if(l_result) {
            type = 4;
            return;
        }
        for(int i = 0; i < 4; i++){
            emit controller->errorLogging("check waypoint: dark/waypoints/" + nameWaypoints.at(i));
            controller->compareSample("dark/waypoints/" + nameWaypoints.at(i),"sample","compare",&l_result); // 0.065??
            if(l_result){
                type = i;
                return;
            }
        }
        Sleep(500);
    }
    err.value = false;
    err.errorMessage = "Не удалось понять что за точка открыта.";
    type = -1;
    return;
}

void Cathedral::attackWaypoints(int type, bool *result) {
    emit controller->errorLogging("Распознавание точки");
    m_error err(result);
    connect(&err, &m_error::Logging, controller, &Controller::LocalLogging);
    bool l_result = false;
    switch (type) {
    case 0: {//altar
        controller->clickButton("dark/waypoints/" + nameWaypoints.at(type),"button_move");
        controller->clickButton("dark/waypoints/" + nameWaypoints.at(type),"button_confirm");
        break;
    }
    case 1: {//camp
        controller->clickButton("dark/waypoints/" + nameWaypoints.at(type),"button_reward");
        controller->clickButton("dark/waypoints/" + nameWaypoints.at(type),"button_confirm");
        checkWarnings();
        break;
    }
    case 2: {//game
        controller->clickButton("dark/waypoints/" + nameWaypoints.at(type),"button_skip");
        controller->clickButton("dark/waypoints/" + nameWaypoints.at(type),"button_confirm");
        break;
    }
    case 3: {//reflection
        controller->clickButton("dark/waypoints/" + nameWaypoints.at(type),"button_confirm");
        break;
    }
    case 4: {//battle
        controller->clickButton("battle/dark","button_best",&l_result,2);
        int x = 0;
        if(!l_result) {
            err.value = false;
            return;
        }
        do{
            Sleep(1000);
            controller->Screenshot();
            controller->setMask("battle/dark/state_power",&l_result);
            if(!l_result) {
                err.value = false;
                return;
            }
            controller->findObject();
            checkPower(controller->cutImage(),&l_result);
            if(!l_result) x++;
            else x = 3;
        } while(x < 3);
        x = 0;
        l_result = false;
        if(settings->premiumStatus){
            while (!l_result) controller->clickButton("battle/dark","button_qstart",&l_result);
            checkWarnings(&l_result);
            Sleep(500);
            while(x<10){
                controller->checkEvent(&l_result);
                if(l_result) controller->skipEvent();
                controller->compareSample("battle/end/quick","sample","compare",&l_result,true,0.025);
                if(!l_result) x++;
                else {
                    controller->clickEsc();
                    Sleep(500);
                    break;
                }
            }
            if(x == 10){
                err.value = false;
                err.errorMessage = "Не загрузились итоги битвы.";
                return;
            }
        }
        else{
            //тут пока что пусто, лень
        }
        x = 0;
        while(x < 3){
            controller->compareSample("dark/waypoints/blessing","sample","compare",&l_result,true);
            if(l_result) break;
            x++;
            Sleep(500);
        }
        if(l_result) {
            controller->clickButton("dark/waypoints/blessing","button_" + QString::number(1 + rand() % 3));
            controller->clickButton("dark/waypoints/blessing","button_confirm");
        }
        else {
            err.value = false;
            err.errorMessage = "Не загрузились благословления после битвы.";
        }
        return;
    }
    default:{
        err.value = false;
        err.errorMessage = "На вход пришла ошибочная точка: " + QString::number(type);
        return;
    }
    }
    Sleep(500);
    controller->compareSample("dark/waypoints","sample_complete","compare_complete",&l_result,true);
    if(l_result) controller->clickButton("dark/waypoints","button_close");
    else {
        err.value = false;
        err.errorMessage = "Не обнаружилось завершение точки.";
    }
    return;
}



void Cathedral::fullGamePass(bool *result) {
    emit controller->Logging("Начато полное прохождение собора");
    m_error err(result);
    connect(&err, &m_error::Logging, controller, &Controller::LocalLogging);
    bool l_result = false;
    while(currentStage != 4) {
        emit controller->Logging("Прохождение " + QString::number(currentStage) + " этажа собора.");
        while(true) {
            findWaypoint(&l_result);
            if(!l_result) break;
            emit controller->errorLogging("Обнаружил точку на координатах: " + QString::number(controller->getRect().x)
                                          + ";" + QString::number(controller->getRect().y));
            controller->click(&l_result);
            if(!l_result){
                err.value = false;
                return;
            }
            int type = -1;
            checkWaypoints(type,&l_result);
            if(!l_result){
                err.value = false;
                return;
            }
            attackWaypoints(type,&l_result);
        }
        checkStage(&l_result);
        if(!l_result){
            err.value = false;
            err.errorMessage = "Этаж не обнаружен";
            return;
        }
    }
}
void Cathedral::safePower(bool *result) {}
void Cathedral::setUnitsSet(bool *result) {}
void Cathedral::bossGamePass(bool *result) {}


