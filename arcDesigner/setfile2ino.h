#ifndef SETFILE2INO_H
#define SETFILE2INO_H

#include <QObject>
namespace SETFILE2INO_PARA
{
    const QString INO_SUFFIX = ".ino";
}
class Setfile2INO : public QObject
{
    Q_OBJECT
public:
    explicit Setfile2INO(QObject *parent = 0):QObject(parent){}

signals:

public slots:
    void printIno(QString filePath);
};

#endif // SETFILE2INO_H
