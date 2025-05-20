#include "Controller.h"

Controller::Controller() {
    m_main = 0;
    m_game = 0;
    m_rect = { 0,0,0,0 };
    mainPath = ":/pages";
}

void Controller::findObject(const Mat *finder, bool *result) {
    emit errorLogging("===Поиск объекта===");
    if(finder != nullptr) finder->copyTo(m_mask);
    m_error err(result);
    connect(&err, &m_error::Logging, this, &Controller::LocalLogging);
    if (m_mask.empty()) {
        err.errorMessage = "Ошибка поиска: Область не найдена.";
        err.value = false;
        return;
    }
    // Преобразуем изображение в оттенки серого
    Mat grayImage;
    cvtColor(m_mask, grayImage, COLOR_BGR2GRAY);
    // Бинаризуем изображение
    Mat binaryImage;
    threshold(grayImage, binaryImage, 127, 255, THRESH_BINARY_INV);
    // Находим контуры
    vector<vector<Point>> contours;
    findContours(binaryImage, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    // Находим прямоугольник, описывающие контур
    m_rect = boundingRect(contours[0]);
    if (m_rect.width == 0 || m_rect.height == 0) {
        err.errorMessage = "Ошибка поиска: Область распознана неправильно.";
        err.value = false;
        return;
    }
}

void Controller::compareObject(double rightVal, const Mat *object, const Mat *sample, bool *result) {
    emit errorLogging("===Сравнение объектов===");
    Mat img1, sample1;
    if (sample != nullptr) sample->copyTo(sample1);
    else m_sample.copyTo(sample1);
    if (object != nullptr) object->copyTo(img1);
    else m_object.copyTo(img1);
    m_error err(result);
    connect(&err, &m_error::Logging, this, &Controller::LocalLogging);
    if (img1.empty()) {
        err.errorMessage = "Ошибка сравнения объектов: img1.";
        err.value = false;
        return;
    }
    if (sample1.empty()) {
        err.errorMessage = "Ошибка сравнения объектов: sample.";
        err.value = false;
        return;
    }
    m_rect.x = 0;
    m_rect.y = 0;

    cvtColor(img1, img1, COLOR_BGR2GRAY);
    cvtColor(sample1, sample1, COLOR_BGR2GRAY);

    // Создание матрицы результата
    int res_col = sample1.cols - img1.cols + 1;
    int res_row = sample1.rows - img1.rows + 1;
    Mat resultMat(res_row, res_col, CV_32FC1);

    // Сравнение шаблона с изображением
    matchTemplate(img1, sample1, resultMat, TM_SQDIFF_NORMED);

    // Нахождение наилучшего совпадения
    double minVal = 0;
    Point minLoc;
    minMaxLoc(resultMat, &minVal, nullptr, &minLoc, nullptr);
    emit errorLogging("matchTemplate: result - " + QString::number(minVal) + " and right value - " + QString::number(rightVal));
    //потом логгер для этого отдельно добавлю...
    // Проверка наилучшего совпадения
    if (minVal <= rightVal) {
        m_rect.x = minLoc.x;
        m_rect.y = minLoc.y;
        return;
    }
    err.value = false;
    return;
}

void Controller::compareSample(const QString &pagePath, const QString &samplePath, const QString &maskPath, bool *result, bool shoot, double rightVal) {
    emit errorLogging("===Поиск и сравнение===");
    if(shoot) Screenshot();
    m_error err(result);
    connect(&err, &m_error::Logging, this, &Controller::LocalLogging);
    if (m_object.empty()) {
        err.errorMessage = "Пустой экран. Проверьте эмулятор.";
        err.value = false;
        return;
    }
    bool l_result = false;
    //Нахождение координат обрезаемой области(findObject)
    emit errorLogging("Convert: " + maskPath);
    convertImage(QImage((mainPath + "/" + pagePath + "/" + maskPath + ".png")), &m_mask,&l_result);
    if(!l_result) {err.value = false;return;}

    findObject(nullptr,&l_result);
    if(!l_result) {err.value = false;return;}

    emit errorLogging("Convert: " + samplePath);
    convertImage(QImage((mainPath + "/" + pagePath + "/" + samplePath + ".png")), &m_sample,&l_result);
    if(!l_result) {err.value = false;return;}
    if(m_object.empty()) {
        err.errorMessage = "m_object empty";
        err.value = false;
        return;
    }
    Mat img1 = m_sample(m_rect);
    compareObject(rightVal,&img1,&m_object,&l_result);
    if(!l_result) {err.value = false;return;}
}

void Controller::saveImage(const QString &savePath, const Mat &saveImage, bool *result) {
    m_error err(result);
    connect(&err, &m_error::Logging, this, &Controller::LocalLogging);
    if(!imwrite(savePath.toStdString(),saveImage)) {
        err.errorMessage = "Ошибка сохранения изображения. Проверьте изображение или путь.";
        err.value = false;
        return;
    }
}

void Controller::Screenshot() {
    int width = 900;
    int height = 600;
    HDC hdcWindow = GetDC(m_game);
    Mat res(height, width, CV_8UC4);
    RECT rc;
    GetClientRect(m_game, &rc);
    HDC hdcMem = CreateCompatibleDC(hdcWindow);
    HBITMAP bitmap = CreateCompatibleBitmap(hdcWindow, rc.right - rc.left, rc.bottom - rc.top);
    HGDIOBJ OBJ = SelectObject(hdcMem, bitmap);
    SelectObject(hdcMem, bitmap);
    BitBlt(hdcMem, 0, 0, rc.right - rc.left, rc.bottom - rc.top, hdcWindow, 0, 0, SRCCOPY);
    SelectObject(hdcMem, OBJ);
    HDC hdcRar = GetDC(NULL);
    SelectObject(hdcRar, bitmap);
    StretchBlt(hdcRar, 0, 0, width, height, hdcMem, 0, 0, width, height, SRCCOPY);
    BITMAPINFOHEADER bi = { sizeof(BITMAPINFOHEADER), width, height, 1, 32 };
    GetDIBits(hdcMem, bitmap, 0, height, res.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    DeleteObject(bitmap);
    DeleteDC(hdcMem);
    DeleteDC(hdcRar);
    ReleaseDC(m_game, hdcWindow);
    flip(res, m_object, 0);
}

void Controller::convertImage(const QImage &imageOne, Mat *imageTwo, bool *result) {
    m_error err(result);
    connect(&err, &m_error::Logging, this, &Controller::LocalLogging);
    // Проверка входных данных
    if (imageOne.isNull()) {
        err.errorMessage = "Пустое изображение на входе конвертации, проверьте путь.";
        err.value = false;
        return;
    }

    if (!imageTwo) {  // Проверка указателя
        err.errorMessage = "Выходное изображение не инициализировано (nullptr).";
        err.value = false;
        return;
    }

    // Определяем формат QImage и конвертируем в соответствующий Mat
    Mat cvImg;
    switch (imageOne.format()) {
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
        cvImg = Mat(imageOne.height(), imageOne.width(), CV_8UC4, (void*)imageOne.bits(), imageOne.bytesPerLine());
        cvtColor(cvImg, cvImg, COLOR_RGBA2BGR);
        break;
    case QImage::Format_Grayscale8:
        cvImg = Mat(imageOne.height(), imageOne.width(), CV_8UC1, (void*)imageOne.bits(), imageOne.bytesPerLine());
        break;
    default:
        err.errorMessage = "Неподдерживаемый формат QImage: " + QString::number(imageOne.format());
        err.value = false;
        return;
    }

    // Копируем результат
    cvImg.copyTo(*imageTwo);
}

Mat Controller::cutImage() {
    return m_object(m_rect);
}

Mat Controller::getMatObject() {
    return m_object;
}

void Controller::setMatObject(const Mat &image, bool *result) {
    m_error err(result);
    connect(&err, &m_error::Logging, this, &Controller::LocalLogging);

    m_object.release();
    image.copyTo(m_object);

    if(m_object.empty()){
        err.value = false;
        err.errorMessage = "Ошибка установления объекта m_object";
        return;
    }
}

void Controller::changeColor(const Mat &before, Mat *after, bool *result) {
    emit errorLogging("===Смена цвета изображения===");
    m_error err(result);
    connect(&err, &m_error::Logging, this, &Controller::LocalLogging);

    Mat temp;
    before.copyTo(temp);

    //почему? не знаю
    cvtColor(temp, temp, COLOR_RGB2BGR);
    cvtColor(temp, temp, COLOR_BGR2RGB);

    // Определяем диапазоны цветов
    Scalar lowerWhite(200, 200, 200); // Нижний предел для белого
    Scalar upperWhite(255, 255, 255); // Верхний предел для белого

    Scalar lowerBlackBlue(0, 0, 0); // Нижний предел для черного
    Scalar upperBlackBlue(100, 100, 100); // Верхний предел для черного

    Scalar lowerBlue(100, 150, 0); // Нижний предел для голубого
    Scalar upperBlue(255, 255, 100); // Верхний предел для голубого

    // Цвета для замены
    Vec3b dullGreen(144, 238, 144); // светло-зеленый
    Vec3b white(255, 255, 255); // белый

    // Проходим по каждому пикселю изображения
    for (int y = 0; y < temp.rows; y++)
    {
        for (int x = 0; x < temp.cols; x++)
        {
            Vec3b pixel = temp.at<Vec3b>(y, x);
            if (pixel[0] >= lowerWhite[0] && pixel[0] <= upperWhite[0] &&
                pixel[1] >= lowerWhite[1] && pixel[1] <= upperWhite[1] &&
                pixel[2] >= lowerWhite[2] && pixel[2] <= upperWhite[2])
            {
                temp.at<Vec3b>(y, x) = dullGreen;
            }
            else if ((pixel[0] >= lowerBlackBlue[0] && pixel[0] <= upperBlackBlue[0] &&
                      pixel[1] >= lowerBlackBlue[1] && pixel[1] <= upperBlackBlue[1] &&
                      pixel[2] >= lowerBlackBlue[2] && pixel[2] <= upperBlackBlue[2]) ||
                     (pixel[0] >= lowerBlue[0] && pixel[0] <= upperBlue[0] &&
                      pixel[1] >= lowerBlue[1] && pixel[1] <= upperBlue[1] &&
                      pixel[2] >= lowerBlue[2] && pixel[2] <= upperBlue[2]))
            {
                // Заменяем черный или голубой на белый
                temp.at<Vec3b>(y, x) = white;
            }
        }
    }
    temp.copyTo(*after);
    if(after->empty()) {
        err.value = false;
        err.errorMessage = "Ошибка конвертации цветов. Изображение пустое.";
        return;
    }
}

void Controller::setSample(const Mat &sample, bool *result) {
    m_error err(result);
    connect(&err, &m_error::Logging, this, &Controller::LocalLogging);
    m_sample.release();
    sample.copyTo(m_sample);
    if(m_sample.empty()) {
        err.value = false;
        err.errorMessage = "Ошибка установления объекта m_sample";
        return;
    }
}

void Controller::setSample(const QString &sample, bool *result) {
    m_sample.release();
    QString path = QDir::cleanPath(mainPath + "/" + sample + ".png");
    QImage image(path);
    convertImage(image, &m_sample,result);
}

void Controller::setMask(const Mat &sample, bool *result) {
    m_error err(result);
    connect(&err, &m_error::Logging, this, &Controller::LocalLogging);
    m_mask.release();
    sample.copyTo(m_mask);
    if(m_mask.empty()) {
        err.value = false;
        err.errorMessage = "Ошибка установления объекта m_mask";
        return;
    }
}

void Controller::setMask(const QString &mask, bool *result) {
    m_mask.release();
    QString path = QDir::cleanPath(mainPath + "/" + mask + ".png");

    QImage image(path);
    convertImage(image, &m_mask, result);
}

Rect& Controller::getRect() {
    return m_rect;
}

void Controller::isEmpty(bool *result) {
    m_error err(result);
    connect(&err, &m_error::Logging, this, &Controller::LocalLogging);

    if(m_main == NULL || m_game == NULL) {
        err.errorMessage = "Не удалось найти эмулятор.";
        err.value = false;
        return;
    }
}

void Controller::isValidSize(bool *result) {
    m_error err(result);
    connect(&err, &m_error::Logging, this, &Controller::LocalLogging);
    RECT temp;
    int width,height;
    GetWindowRect(m_game,&temp);
    width = temp.right - temp.left;
    height = temp.bottom - temp.top;
    if(width != 900 || height != 600) {
        err.value = false;
        err.errorMessage = "Неправильные размеры эмулятора.";
        return;
    }
}

void Controller::isValidPos(bool *result) {
    m_error err(result);
    connect(&err, &m_error::Logging, this, &Controller::LocalLogging);
    RECT temp;
    GetWindowRect(m_game,&temp);
    if(temp.left != 1 || temp.top != 33) {
        err.value = false;
        err.errorMessage = "Неправильное положение эмулятора.";
        return;
    }
}

void Controller::click(bool *result, int count, int delay) {
    emit errorLogging("===Клик=== [" + QString::number(m_rect.x) + ";" + QString::number(m_rect.y) + "]");
    m_error err(result);
    connect(&err, &m_error::Logging, this, &Controller::LocalLogging);
    bool l_result = false;
    Mat before,after;
    m_object.copyTo(before);
    Rect click = m_rect;
    int x = 0;
    SendMessage(m_main, WM_SETFOCUS, 0, 0);
    Sleep(15);
    SendMessage(m_main, WM_MOUSEACTIVATE, (WPARAM)m_main, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    Sleep(15);
    SendMessage(m_game, WM_SETCURSOR, (WPARAM)m_game, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    Sleep(15);
    do {
        after.release();
        PostMessage(m_game, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(click.x + 5, click.y + 5));
        Sleep(delay);
        PostMessage(m_game, WM_LBUTTONUP, 0, MAKELPARAM(click.x + 5, click.y + 5));
        Sleep(200);
        Screenshot();
        m_object.copyTo(after);
        compareObject(0.0001, &after,&before,&l_result);
        if (l_result) Sleep(delay);
        else return;
        x++;
    } while (x < count);
    //Если цикл вышел сюда, то значит что клики прошли а изображение не поменялось
    err.value = false;
    err.errorMessage = "Клик не был успешен. Проверьте параметры заново.";
    return;
}

void Controller::clickPosition(const Rect &point, bool *result, int count, int delay) {
    emit errorLogging("===Клик по точке=== [" + QString::number(point.x) + ";" + QString::number(point.y) + "]");
    m_error err(result);
    connect(&err, &m_error::Logging, this, &Controller::LocalLogging);
    bool l_result = false;
    Mat before,after;
    m_object.copyTo(before);
    int x = 0;
    SendMessage(m_main, WM_SETFOCUS, 0, 0);
    Sleep(15);
    SendMessage(m_main, WM_MOUSEACTIVATE, (WPARAM)m_main, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    Sleep(15);
    SendMessage(m_game, WM_SETCURSOR, (WPARAM)m_game, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    Sleep(15);
    do {
        after.release();
        PostMessage(m_game, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(point.x + 5, point.y + 5));
        Sleep(delay);
        PostMessage(m_game, WM_LBUTTONUP, 0, MAKELPARAM(point.x + 5, point.y + 5));
        Sleep(200);
        Screenshot();
        m_object.copyTo(after);
        compareObject(0.0001, &after,&before,&l_result);
        if (l_result) {
            Sleep(delay);
        }
        else return;
        x++;
    } while (x < count);
    //Если цикл вышел сюда, то значит что клики прошли а изображение не поменялось
    err.value = false;
    err.errorMessage = "Клик не был успешен. Проверьте параметры заново.";
    return;
}

void Controller::clickSwipe(const Rect &start, const Rect &finish, bool *result) {
    emit errorLogging("===Свайп===");
    m_error err(result);
    connect(&err, &m_error::Logging, this, &Controller::LocalLogging);

    auto l_start = Vec2i(start.x, start.y);
    auto l_stop = Vec2i(finish.x, finish.y);
    auto path = l_stop - l_start;
    Vec2f path_dir = path;
    path_dir = normalize(path_dir);
    SendMessage(m_main, WM_SETFOCUS, 0, 0);
    BYTE arr[256];
    memset(arr, 0, sizeof(256));
    GetKeyboardState(arr);
    auto old = arr[VK_LBUTTON];
    arr[VK_LBUTTON] = 128;
    SetKeyboardState(arr);

    SendMessage(m_main, WM_MOUSEACTIVATE, (WPARAM)m_main, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    Sleep(15);
    SendMessage(m_game, WM_SETCURSOR, (WPARAM)m_game, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    Sleep(15);
    PostMessage(m_game, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(l_start[0], l_start[1]));
    Sleep(15);

    const int count = int(norm(path));
    Vec2i last_pos;

    for (int i = 0; i < count; ++i)
    {
        auto pos = Vec2i(path_dir * float(i));
        last_pos = l_start + pos;

        SetKeyboardState(arr);
        PostMessage(m_game, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(last_pos[0], last_pos[1]));

        const int pause = 50;
        const int offset = 15;
        if (i < offset)
        {
            auto p = 10 + ((offset - (i + 1)) * pause) / offset;
            Sleep(p);
        }
        else if (i > count - offset)
        {
            auto p = 30 + ((offset - (count - i)) * pause) / offset;
            Sleep(p);
        }
        else if (i % 5 == 0)
            Sleep(1);
    }

    PostMessage(m_game, WM_LBUTTONUP, 0, MAKELPARAM(last_pos[0], last_pos[1]));
    Sleep(15);
    SendMessage(m_game, WM_SETCURSOR, (WPARAM)m_game, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
    Sleep(15);

    arr[VK_LBUTTON] = old;
    SetKeyboardState(arr);
}

void Controller::clickButton(const QString &pagePath, const QString &buttonName, bool *result, int count, int delay) {
    emit errorLogging("===Клик по кнопке=== : \"" + mainPath + "/" + pagePath + "/" + buttonName + ".png" + "\"");
    m_error err(result);
    connect(&err, &m_error::Logging, this, &Controller::LocalLogging);

    m_mask.release();
    convertImage(QImage((mainPath + "/" + pagePath + "/" + buttonName + ".png")), &m_mask,result);
    if(result != nullptr && *result == false) return;
    findObject(nullptr,result);
    if(result != nullptr && *result == false) return;
    click(result,count,delay);
}

void Controller::clickEsc(bool *result) {
    emit errorLogging("===Клик Esc===");
    m_error err(result);
    connect(&err, &m_error::Logging, this, &Controller::LocalLogging);
    bool l_result = false;
    Mat before,after;
    m_object.copyTo(before);
    SendMessage(m_main, WM_SETFOCUS, 0, 0);
    Sleep(15);
    SendMessage(m_main, WM_MOUSEACTIVATE, (WPARAM)m_main, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    Sleep(15);
    SendMessage(m_game, WM_SETCURSOR, (WPARAM)m_game, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    Sleep(15);
    PostMessage(m_game, WM_KEYDOWN, VK_ESCAPE, MapVirtualKey(VK_ESCAPE, MAPVK_VK_TO_VSC));
    Sleep(100);
    PostMessage(m_game, WM_KEYUP, VK_ESCAPE, MapVirtualKey(VK_ESCAPE, MAPVK_VK_TO_VSC));
    Sleep(500);
    Screenshot();
    m_object.copyTo(after);
    compareObject(0.0001, &after,&before,&l_result);
    if(!l_result) return;
    //Если цикл вышел сюда, то значит что клики прошли а изображение не поменялось
    err.value = false;
    err.errorMessage = "ESC не был успешен.";
    return;
}

void Controller::clickReturn(bool *result) {
    emit errorLogging("===Клик Return===");
    m_error err(result);
    connect(&err, &m_error::Logging, this, &Controller::LocalLogging);
    bool l_result = false;
    Mat before,after;
    m_object.copyTo(before);
    SendMessage(m_main, WM_SETFOCUS, 0, 0);
    Sleep(15);
    SendMessage(m_main, WM_MOUSEACTIVATE, (WPARAM)m_main, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    Sleep(15);
    SendMessage(m_game, WM_SETCURSOR, (WPARAM)m_game, MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN));
    Sleep(15);
    PostMessage(m_game, WM_KEYDOWN, VK_RETURN, 0);
    Sleep(100);
    PostMessage(m_game, WM_KEYUP, VK_RETURN, 0);
    Sleep(500);
    Screenshot();
    m_object.copyTo(after);
    compareObject(0.0001, &after,&before,&l_result);
    if(!l_result) return;
    //Если цикл вышел сюда, то значит что клики прошли а изображение не поменялось
    err.value = false;
    err.errorMessage = "RETURN не был успешен.";
    return;
}

//void Controller::Initialize(userProfile *user, bool *result){}

void Controller::userInitialize(userProfile *user, bool *result) {
    emit Logging("Инициализация пользователя");
    m_error err(result);
    connect(&err, &m_error::Logging, this, &Controller::LocalLogging);
    bool l_result = false;
    clickButton("main","button_user",&l_result);
    if(!l_result) {
        err.value = false;
        err.errorMessage = "Проблема с нажатием на кнопку.";
        return;
    }
    int x = 0;
    do{
        compareSample("user","sample","compare",&l_result,true);
        if(l_result) break;
        x++;
        Sleep(1000);
    } while(x < 10);
    if(!l_result) {
        err.value = false;
        err.errorMessage = "Не найден профиль.";
        return;
    }
    setMask("user/user_id",&l_result);
    if(!l_result) {
        err.value = false;
        err.errorMessage = "Ошибка пути маски user/user_id";
        return;
    }
    findObject(nullptr,&l_result);
    if(!l_result) return;
    emit Recognize(cutImage(),user->user_ID);
    emit errorLogging(QString::number(user->user_ID));
    if(user->user_ID <=0) {
        err.value = false;
        err.errorMessage = "Ошибка распознавания айди.";
        return;
    }
    user->subscribe = typeSub::admin;
    setMask("user/user_power",&l_result);
    if(!l_result) {
        err.value = false;
        err.errorMessage = "Ошибка пути маски user/user_power";
        return;
    }
    findObject(nullptr,&l_result);
    if(!l_result) return;
    emit Recognize(cutImage(),user->history_power);
        emit errorLogging(QString::number(user->history_power));
    if(user->history_power <=0) {
        err.value = false;
        err.errorMessage = "Ошибка распознавания айди.";
        return;
    }
    user->state_premium = true;
    user->state_ads = false;
    user->count_units = 6;//потом сделать адекватное распознавание после получения инфы о подписке, условно
    user->leftover_time = "9999999 days for Raccoons";
    clickButton("user","button_close");
}

void Controller::fixGameError(bool *result) {}

void Controller::getGameError() {
    if(m_object.empty()) emit errorLogging("Пустой m_object");
    else imshow("1",m_object);
    if(m_mask.empty()) emit errorLogging("Пустой m_mask");
    else imshow("2",m_mask);
    if(m_sample.empty()) emit errorLogging("Пустой m_sample");
    else imshow("3",m_sample);
}

QString& Controller::getMainPath() {
    return mainPath;
}

void Controller::setMainPath(const QString &path) {
    mainPath.clear();
    mainPath = path;
}

void Controller::Start(userProfile *user, bool *result) {
    emit Logging("Бот запущен");
    m_error err(result);
    connect(&err, &m_error::Logging, this, &Controller::LocalLogging);
    bool l_result = false;

    FindEmulator(user->emulator_name,&m_main,&m_game);
    if(m_main == NULL){
        err.value = false;
        err.errorMessage = "Произошла ошибка в обнаружении эмулятора.";
        return;
    }
    checkPreMainPage();
    checkMainPage(&l_result);
    if(!l_result){
        err.value = false;
        err.errorMessage = "Не обнаружена главная страница.";
        return;
    }
    checkSettings(&l_result);
    if(!l_result){
        err.value = false;
        err.errorMessage = "Беда у настроек игры.";
        return;
    }
    // добавить проверку настроек перед этим
    userInitialize(user,&l_result);
    if(!l_result) {
        err.value = false;
        err.errorMessage = "Проблема с обнаружением профиля игрока.";
        return;
    }
    //refreshMainPage(&l_result); // poka chto
    if(!l_result) {
        err.value = false;
        err.errorMessage = "Не получилось сбросить положение экрана(.";
        return;
    }
    //пока что временно это всё, потом добавить
    emit endStart();
}

void Controller::Stop(bool *result) {
    m_error err(result);
    connect(&err, &m_error::Logging, this, &Controller::LocalLogging);
    if(m_main == NULL) {
        err.value = false;
        err.errorMessage = "Попытка закрыть несуществующий эмулятор.";
        return;
    }
    PostMessage(m_main,WM_CLOSE,0,0);
    m_main = 0;
    m_game = 0;
}

void Controller::LocalLogging(const QString &msg) {
    //getGameError();//пока что пустой
    emit errorLogging(msg);
}

void Controller::checkLoading() {
    bool l_result = false;
    do{
        compareSample("load","sample","compare",&l_result,true);
        if(l_result) Sleep(1000);
    } while(l_result);
    Sleep(1000);
}

void Controller::checkGameLoading() {
    emit errorLogging("===Проверяю загрузку===");
    bool l_result = false;
    do {
        compareSample("load","sample_open","compare_open",&l_result,true);
        if(l_result) Sleep(1000);
    } while(l_result);
    do {
        compareSample("load","sample_logo","compare_logo",&l_result,true);
        if(l_result) Sleep(1000);
    } while(l_result);
    checkLoading();
}

void Controller::checkPreMainPage() {
    emit errorLogging("===Проверка предзагрузки главной страницы===");
    checkGameLoading();
    bool l_result = false;
    checkEvent(&l_result);
    if(l_result) skipEvent();
    compareSample("load", "sample_mail", "compare_mail", &l_result,true);
    if (l_result) {
        clickButton("load","button_close_mail",&l_result);
        Sleep(500);
    }

    //ne pomny gde pass viskakivaet + сделать выходы из циклов
    do {
        compareSample("load", "sample_pass", "compare_pass", &l_result, true);
        if(l_result) {
            clickEsc();
            Sleep(3000);
        }
    } while(l_result);
    do {
        compareSample("load", "sample_offer", "compare_offer", &l_result, true);
        if(l_result) {
            clickEsc();
            Sleep(3000);
        }
    } while(l_result);
    do {
        compareSample("load", "sample_pass", "compare_pass", &l_result, true);
        if(l_result) {
            clickEsc();
            Sleep(3000);
        }
    } while(l_result);
}

void Controller::checkMainPage(bool *result) {
    Sleep(500);
    emit errorLogging("===Проверка главной страницы===");
    m_error err(result);
    connect(&err, &m_error::Logging, this, &Controller::LocalLogging);
    bool l_result = false;
    compareSample("main","sample","compare",&l_result,true);
    if(!l_result){
        emit errorLogging("Первая проверка провалена.");
        compareSample("main","sample_1","compare_1",&l_result,true);
        if(!l_result){
            err.value = false;
            err.errorMessage = "Главная страница не найдена. Ошибка на 2-х этапной проверке.";
            return;
        }
    }
}

void Controller::checkEvent(bool *result) {
    m_error err(result);
    connect(&err, &m_error::Logging, this, &Controller::LocalLogging);
    emit errorLogging("===Проверка ивента===");
    int x = 0;
    bool l_result = false;
    setMask("event/compare",&l_result);
    if(!l_result) {
        err.value = false;
        err.errorMessage = "Ошибка установления маски event/compare";
        return;
    }
    convertImage(QImage((mainPath + "/event/sample.png")), &m_object,&l_result);
    if(!l_result) {
        err.value = false;
        err.errorMessage = "Ошибка установления изображения event/sample";
        return;
    }
    Mat find = cutImage();
    do {
        Screenshot();
        compareObject(0.02,&find,&m_object,&l_result);
        if(l_result) break;
        else {
            x++;
            Sleep(500);
        }
    } while(x < 2);

    if(x == 2) {
        err.value = false;
        return;
    }
}

void Controller::checkSettings(bool *result) {
    emit errorLogging("===Проверка настроек===");
    bool l_result = false;
    m_error err(result);
    connect(&err, &m_error::Logging, this, &Controller::LocalLogging);
    clickButton("main","button_settings",&l_result);
    if(!l_result) {
        err.value = false;
        err.errorMessage = "Кнопка настроек не была нажата";
        return;
    }
    int x = 0;
    do {
        compareSample("settings","sample","compare",&l_result,true);
        if(l_result) break;
        else {
            x++;
            Sleep(500);
        }
    } while(x < 10);
    if(x == 10) {
        err.value = false;
        err.errorMessage = "Настройки не найдены.";
        return;
    }
    compareSample("settings","sample","state_fps",&l_result);
    if(!l_result) clickButton("settings","button_fps",&l_result);
    if(!l_result) {
        err.value = false;
        err.errorMessage = "Не удалось нажать на кнопку FPS.";
        return;
    }
    compareSample("settings","sample","state_lang",&l_result);
    if(!l_result) {
        clickButton("settings","button_lang",&l_result);
        Sleep(1000);
        if(!l_result) {
            err.value = false;
            err.errorMessage = "Не удалось нажать на кнопку lang.";
            return;
        }
        compareSample("settings","sample_change_lang","compare_change_lang",&l_result,true);
        if(!l_result){
            err.value = false;
            err.errorMessage = "Не удалось найти окно смены языка.";
            return;
        }
        clickButton("settings","button_en",&l_result);
        if(!l_result) {
            err.value = false;
            err.errorMessage = "Не удалось нажать на кнопку lang|en.";
            return;
        }
        compareSample("settings","sample_confirm","compare_confirm",&l_result,true);
        if(!l_result){
            err.value = false;
            err.errorMessage = "Не удалось найти окно подтверждения смены языка.";
            return;
        }
        clickButton("settings","button_yes",&l_result);
        if(!l_result) {
            err.value = false;
            err.errorMessage = "Не удалось нажать на кнопку смены языка.";
            return;
        }
    } else clickEsc();
}

void Controller::refreshMainPage(bool *result) {
    emit errorLogging("===Обновление главной страницы===");
    m_error err(result);
    connect(&err, &m_error::Logging, this, &Controller::LocalLogging);
    bool l_result = false;
    checkMainPage(&l_result);
    if(!l_result) {
        err.value = false;
        err.errorMessage = "Ошибка: не на главное странице.";
        return;
    }
    clickButton("main","button_friends",&l_result);
    if(!l_result){
        err.value = false;
        err.errorMessage = "Ошибка: не удалось нажать на кнопку друзей.";
        return;
    }
    int x = 0;
    do {
        compareSample("top_players","sample","compare",&l_result,true);
        if(l_result) break;
        else {
            x++;
            Sleep(500);
        }
    } while(x < 100);
    if(x == 100) {
        err.value = false;
        err.errorMessage = "Ошибка: не прогрузился список топ-игроков.";
        return;
    }
    Sleep(500);//?
    clickPosition(Rect(480,200,0,0));
    Sleep(500);
    clickPosition(Rect(480,200,0,0));
    do {
        compareSample("top_players","sample_top","compare_top",&l_result,true);
        Sleep(500);
    } while(l_result);
    clickEsc();
    checkMainPage(&l_result);
    if(!l_result){
        err.value = false;
        err.errorMessage = "Ошибка сброса положения экрана.";
        return;
    }
}

void Controller::skipEvent() {
    emit errorLogging("===Пропуск ивентовых наград===");
    bool l_result = false;
    do {
        checkEvent(&l_result);
        if(l_result) {
            clickEsc();
            Sleep(1000);
        }
    } while(l_result);
}

void Controller::checkMap(bool *result) {
    emit errorLogging("Проверка карты");
    m_error err(result);
    connect(&err, &m_error::Logging, this, &Controller::LocalLogging);
    bool l_result = false;
    int x = 0;
    while(x < 5){
        compareSample("map","sample","compare",&l_result,true);
        if(!l_result){
            compareSample("map","sample_right","compare_right",&l_result);
            if(l_result) {
                do {
                    clickSwipe({100,100,0,0},{200,100,0,0},&l_result);
                    compareSample("map","sample","compare",&l_result,true);
                } while(!l_result);
                return;
            }
            else x++;
        }
        else return;
        Sleep(1000);
    }
    err.value = false;
    return;
}

void Controller::findBarracks(bool *result) {}

void Controller::entryBarracks(bool *result) {}

void Controller::scanSquadCount(userProfile *user, bool *result) {}
