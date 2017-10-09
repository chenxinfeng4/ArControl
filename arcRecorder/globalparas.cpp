#include "globalparas.h"

int indexOf(const QStringList strlist, const QString str){
    int i=-1;
    foreach(QString temp, strlist){
        ++i;
        if(temp == str)
            return i;
    }
    return -1;
}
