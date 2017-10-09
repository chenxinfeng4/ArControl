#ifndef GLOBALPARAS_H
#define GLOBALPARAS_H
#include <QObject>
#include "main.h"

namespace  GLOBAL_PARA {
    const QStringList STR_L_LANG = QStringList()
            << "English" << "简体中文";
    const QStringList STR_L_LANG_TRANSLATOR = QStringList()
            << "" << ":/cn.qm";
    const QString PROFILE_FILE("profile.xml");
    const QString DOM_PROFILE("PROFILE"), DOM_LANG("LANG"), DOM_ARDUINO("ARDUINO_DEBUG");
}

int indexOf(const QStringList strlist, const QString str);
#endif // GLOBALPARAS_H
