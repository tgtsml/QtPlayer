QT          += core gui widgets
TEMPLATE    = app
TARGET      = GtsmlPlayer
DESTDIR     = $$PWD/bin
CONFIG      += c++11
DEFINES     += QT_DEPRECATED_WARNINGS

SOURCES     += \
            main.cpp \
            mainwindow.cpp \
            playthread.cpp \
            xslider.cpp

HEADERS     += \
            mainwindow.h \
            playthread.h \
            xslider.h

FORMS       += \
            mainwindow.ui

INCLUDEPATH += $$PWD/include

LIBS        += -L$$PWD/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale -lSDL2 -lSDL2main

RESOURCES   += \
            res.qrc
