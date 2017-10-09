#ifndef DEPLOYMENT_HPP
#define DEPLOYMENT_HPP
#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
const QString src(":/deployment");
const QString target("./");

void deployFile(); /* copy file from resource to target */
void copyDir(QString from, QString to, bool overWirte); /* copy dir recursively */


void copyDir(QString from, QString to, bool overWirte)
{
    QFileInfo fi(from);
    if(!fi.exists() || !fi.isDir()){
        return;
    }
    else{
        const QDir dir0(from);
        QString from_ = from;
        QString to_ = to;
        if(!from_.endsWith('/') && !from_.endsWith('\\'))
            from_ += "/";
        if(!to_.endsWith('/') && !to_.endsWith('\\'))
            to_ += "/";
        foreach(QString directoryName, dir0.entryList(QDir::Dirs | QDir::NoDotAndDotDot)){
            QString destinationPath,  sourcePath;
            destinationPath  = to_ + directoryName;
            sourcePath = from_ + directoryName;
            destinationPath = QFileInfo(destinationPath).absoluteFilePath();
            if(!QDir(destinationPath).exists()){
                QDir(destinationPath).mkpath(destinationPath);
            }
            copyDir(sourcePath, destinationPath, overWirte);
        }
        foreach(QString fileName, dir0.entryList(QDir::Files)){
            QString destinationPath,  sourcePath;
            destinationPath  = to_ + fileName;
            sourcePath = from_ + fileName;
            destinationPath = QFileInfo(destinationPath).absoluteFilePath();
            if(!QFile(destinationPath).exists() || overWirte){
                QFile::copy(sourcePath, destinationPath);
                qDebug()<<"File copy form"<<sourcePath <<"==>" <<destinationPath;
            }
        }
    }
}

void deployFile()
{
    copyDir(src, target, false);
}
#endif // DEPLOYMENT_HPP
