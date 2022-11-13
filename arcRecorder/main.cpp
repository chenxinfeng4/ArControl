#include <QApplication>
#include <QDomDocument>
#include <QTranslator>
#include "mainwindow.h"
#include "deployment.hpp"
#include "globalparas.h"

static QTranslator* langRead();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //--- set GUI language ---
    a.installTranslator(langRead());

    //--- copy file from ":/deployment/*" to "./*" ---
    deployFile();

    //--- make up GUI ---
    MainWindow w;
    w.show();

    return a.exec();
}

QTranslator* langRead()
{
    const QString profilePf = GLOBAL_PARA::PROFILE_FILE;
    QTranslator* translator = new QTranslator();
    QString lang;
    try{
        QFile f(profilePf);
        QFileInfo finfo(profilePf);
        qDebug()<<"Read profile from "<<finfo.absoluteFilePath();
        QDomDocument doc("doc");
        SCPP_ASSERT_THROW(f.open(QFile::ReadOnly | QFile::Text),
                          "Cannot open file!");
        doc.setContent(&f);
        f.close();
        QDomElement root = doc.documentElement();
        SCPP_ASSERT_THROW(root.tagName()==GLOBAL_PARA::DOM_PROFILE,
                          "File contents error!");
        lang=root.firstChildElement(GLOBAL_PARA::DOM_LANG).text();
        SCPP_ASSERT_THROW(GLOBAL_PARA::STR_L_LANG.contains(lang),
                          "No support such language yet!");
    }
    catch(...){
        lang=GLOBAL_PARA::STR_L_LANG[0];
    }
    int ind = indexOf(GLOBAL_PARA::STR_L_LANG, lang);
    QString translator_qm = GLOBAL_PARA::STR_L_LANG_TRANSLATOR[ind];
    if(!translator_qm.isEmpty())
        translator->load(translator_qm);
    return translator;
}
