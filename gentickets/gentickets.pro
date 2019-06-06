QT       += sql xml

QT       -= gui

TARGET = gentickets
TEMPLATE = lib

DEFINES += GENTICKETS_LIBRARY

SOURCES += gentickets.cpp

HEADERS += gentickets.h\
        gentickets_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

win32 {
    INCLUDEPATH += D:/QtLibs/zlib-1.2.11
    LIBS += -LD:/QtLibs/zlib-1.2.11 -lz
    INCLUDEPATH += D:/QtLibs/quazip-0.7.3/quazip
    LIBS += -LD:/QtLibs/quazip-0.7.3/quazip/release -lquazip
}

unix:!macx {
    INCLUDEPATH += /home/user/Документы/QtLibs/zlib-1.2.11
    LIBS += -L/home/user/Документы/QtLibs/zlib-1.2.11 -lz
    INCLUDEPATH += /home/user/Документы/QtLibs/quazip-0.7.3/quazip
    LIBS += -L/home/user/Документы/QtLibs/quazip-0.7.3 -lquazip
}
