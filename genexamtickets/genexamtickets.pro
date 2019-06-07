#-------------------------------------------------
#
# Project created by QtCreator 2018-11-06T16:19:46
#
#-------------------------------------------------

QT       += core gui sql printsupport xml xmlpatterns testlib help

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets sql

TARGET = genexamtickets
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    tableform.cpp \
    generateform.cpp \
    saveform.cpp \
    dropdialog.cpp \
    dragdroplistmodel.cpp \
    comboboxdelegate.cpp \
    qcustomplot.cpp

HEADERS += \
        mainwindow.h \
    tableform.h \
    generateform.h \
    genlib/include/gentickets.h \
    genlib/include/gentickets_global.h \
    saveform.h \
    dropdialog.h \
    dragdroplistmodel.h \
    comboboxdelegate.h \
    qcustomplot.h

FORMS += \
        mainwindow.ui \
    tableform.ui \
    generateform.ui \
    saveform.ui \
    dropdialog.ui

RESOURCES += \
    resources.qrc


win32 {
    INCLUDEPATH += D:/QtLibs/zlib-1.2.11
    LIBS += -LD:/QtLibs/zlib-1.2.11 -lz
    INCLUDEPATH += D:/QtLibs/quazip-0.7.3/quazip
    LIBS += -LD:/QtLibs/quazip-0.7.3/quazip/release -lquazip

    win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../build-GEN-Desktop_Qt_5_9_0_MinGW_32bit-Release/gentickets/release/ -lgentickets
    else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../build-GEN-Desktop_Qt_5_9_0_MinGW_32bit-Debug/gentickets/debug/ -lgentickets
}

unix:!macx {
    INCLUDEPATH += /home/user/Документы/QtLibs/zlib-1.2.11
    LIBS += -L/home/user/Документы/QtLibs/zlib-1.2.11 -lz
    INCLUDEPATH += /home/user/Документы/QtLibs/quazip-0.7.3/quazip
    LIBS += -L/home/user/Документы/QtLibs/quazip-0.7.3 -lquazip

    unix:!macx: LIBS += -L$$PWD/../../build-GEN-Desktop_Qt_5_9_0_GCC_64bit-Release/gentickets/ -lgentickets
}

INCLUDEPATH += $$PWD/../gentickets
DEPENDPATH += $$PWD/../gentickets
