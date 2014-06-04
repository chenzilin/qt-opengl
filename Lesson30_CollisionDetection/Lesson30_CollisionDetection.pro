#-------------------------------------------------
#
# Project created by QtCreator 2014-02-17T21:31:34
#
#-------------------------------------------------

QT       += core gui opengl multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Lesson30_CollisionDetection
TEMPLATE = app


SOURCES += main.cpp\
        myglwidget.cpp \
    tvector.cpp \
    tray.cpp

HEADERS  += myglwidget.h \
    tvector.h \
    tray.h \
    mathex.h

RESOURCES += \
    src.qrc
