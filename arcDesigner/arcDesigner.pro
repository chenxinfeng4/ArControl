#-------------------------------------------------
#
# Project created by QtCreator 2017-04-03T16:35:06
#
#-------------------------------------------------

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TRANSLATIONS+=cn.ts

TARGET = arcDesigner
TEMPLATE = app

CONFIG += c++11

RC_FILE = res.rc

SOURCES += main.cpp\
        mainwindow.cpp \
    codeeditor.cpp \
    pinassigndialog.cpp \
    varassigndialog.cpp \
    designersetfile.cpp \
    vendor_modify/scpp_assert.cpp \
    profiledialog.cpp \
    statecontent.cpp \
    uistateframe/myboxui.cpp \
    uistateframe/statestripbar.cpp \
    uistateframe/statebox.cpp \
    uistateframe/component.cpp \
    uistateframe/session.cpp \
    globalparas.cpp \
    setfile2ino.cpp

HEADERS  += mainwindow.h \
    codeeditor.h \
    pinassigndialog.h \
    varassigndialog.h \
    designersetfile.h \
    vendor_modify/scpp_assert.h \
    main.h \
    profiledialog.h \
    statecontent.h \
    uistateframe/myboxui.h \
    uistateframe/statestripbar.h \
    uistateframe/statebox.h \
    globalparas.h \
    uistateframe/component.h \
    uistateframe/session.h \
    setfile2ino.h \
    deployment.hpp

FORMS    += mainwindow.ui \
    pinassigndialog.ui \
    varassigndialog.ui \
    profiledialog.ui

RESOURCES += \
    resource.qrc

DISTFILES += \
    deployment/Readme/Arduino_Mega_Future_Pin.png \
    deployment/Readme/Arduino_Mega_Pin.png \
    deployment/Readme/Arduino_Nano_Pin.png \
    deployment/Readme/Arduino_UNO_Pin.png
