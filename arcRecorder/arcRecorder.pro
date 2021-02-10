#-------------------------------------------------
#
# Project created by QtCreator 2017-03-13T15:14:30
#
#-------------------------------------------------

QT       += core gui  serialport charts xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

RC_FILE = res.rc
TRANSLATIONS+=cn.ts

TARGET = arcRecorder
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    onlinemanagerbar.cpp \
    serialcheck.cpp \
    serialflowcontrol.cpp \
    tasktoarduino.cpp \
    onlinechart.cpp \
    onlinetable.cpp \
    onlinesetfile.cpp \
    profilereader.cpp \
    vendor_modify/scpp_assert.cpp \
    globalparas.cpp \
    arcfirmata/arcfirmata.cpp \
    arcfirmata/pin_ai.cpp \
    arcfirmata/pin_do.cpp \
    arcfirmata/src/qfirmata.cpp \
    onlineled.cpp


HEADERS  += mainwindow.h \
    onlinemanagerbar.h \
    serialcheck.h \
    serialflowcontrol.h \
    tasktoarduino.h \
    onlinechart.h \
    onlinetable.h \
    onlinesetfile.h \
    main.h \
    profilereader.h \
    vendor_modify/scpp_assert.h \
    deployment.hpp \
    globalparas.h \
    arcfirmata/arcfirmata.h \
    arcfirmata/pin_ai.h \
    arcfirmata/pin_do.h \
    arcfirmata/src/qfirmata.h \
    arcfirmata/src/qfirmatadefines.h \
    onlineled.h


FORMS    += mainwindow.ui

RESOURCES += \
    resource.qrc
