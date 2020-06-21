QT          += core gui widgets
TEMPLATE    = app
TARGET      = GtPlayer
DESTDIR     = $$PWD/bin
CONFIG      += c++11
DEFINES     += QT_DEPRECATED_WARNINGS

SOURCES     += \
            main.cpp \
            mainwindow.cpp

HEADERS     += \
            mainwindow.h

FORMS       += \
            mainwindow.ui

INCLUDEPATH += $$PWD/include

LIBS        += -L$$PWD/lib -lavcodec -lSDL2

RESOURCES   += \
            res.qrc
