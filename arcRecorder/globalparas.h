#ifndef GLOBALPARAS_H
#define GLOBALPARAS_H
#include <QObject>
#include "main.h"

namespace  GLOBAL_PARA {
    const QStringList STR_L_LANG = QStringList()
            << "English" << "简体中文";
    const QStringList STR_L_LANG_TRANSLATOR = QStringList()
            << "" << ":/cn.qm";
    const QStringList STR_L_ARDUINOBOARD = QStringList()
            << "Uno" << "Mega" << "Nano";
    const QString PROFILE_FILE("profile.xml");
    const QString DOM_PROFILE("PROFILE"), DOM_LANG("LANG"),
                  DOM_ARDUINOBOARD("ARDUINO_BOARD"), DOM_ARDUINO("ARDUINO_DEBUG");
}
namespace VERSION_INFO {
    const QString VERSION_NUM("V0.2");
    const QString VERSION_TIME("2019-05-24");
}
int indexOf(const QStringList strlist, const QString str);
#endif // GLOBALPARAS_H
