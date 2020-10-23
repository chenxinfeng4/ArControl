#ifndef PROFILEREADER_H
#define PROFILEREADER_H

#include "main.h"
#include <QObject>
#include <QString>

class QDomDocument;

namespace PROFILEREADER_PARA
{

}
namespace PROFILEREADER_PRIVATE
{
}
class ProfileReader : public QObject
{
    Q_OBJECT
public:
    QString getArduino();
    QString getBoard();
    static ProfileReader * getInstance();
    void checkProfile(bool newapath=false);

private:
    QString arduino_debug;
    QString arduino_board;  //["uno" | "mega" | "nano"]
    bool hasChecked;

    static ProfileReader * instance;
    explicit ProfileReader(QObject *parent = 0);
    explicit ProfileReader(const ProfileReader&);
    ProfileReader& operator=(const ProfileReader&);

signals:

public slots:
    void reArduinoPath();
};

#endif // PROFILEREADER_H
