#include "dynamiccombobox.h"

DynamicComboBox::DynamicComboBox(QWidget* parent)
    : QComboBox(parent)
{
    addItem("Выберите эмулятор");
    setItemData(0, true, Qt::UserRole - 1); // Первый элемент недоступен для выбора
}

void DynamicComboBox::showPopup() {
    updateEmulatorList(); // обновляем перед открытием
    QComboBox::showPopup(); // вызов оригинального комбобоха
}

void DynamicComboBox::updateEmulatorList() {
    // Очистим всё, кроме первого (нулевого) элемента
    while (count() > 1) {removeItem(1);}

    // Здесь вызывается внешняя функция, получающая список
    QList<HWND> hwnds;
    QList<QString> emulatorNames;
    getEmulatorsList(hwnds, emulatorNames);

    if(emulatorNames.size() == 0) {
        addItem("Эмуляторы не найдены");
        setItemData(1,true,Qt::UserRole - 1);
        return;
    }
    for (const QString& name : emulatorNames) {addItem(name);}
}
