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

INCLUDEPATH += D:/QtLibs/zlib-1.2.11
LIBS += -LD:/QtLibs/zlib-1.2.11 -lz
INCLUDEPATH += D:/QtLibs/quazip-0.7.3/quazip
LIBS += -LD:/QtLibs/quazip-0.7.3/quazip/release -lquazip
