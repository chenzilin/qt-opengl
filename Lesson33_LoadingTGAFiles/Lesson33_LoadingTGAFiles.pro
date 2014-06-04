#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T14:53:46
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Lesson33_LoadingTGAFiles
TEMPLATE = app


SOURCES += main.cpp\
        myglwidget.cpp \
    tgaloader.cpp

HEADERS  += myglwidget.h \
    tgaloader.h

CONFIG(debug, debug|release) {
    sourcefiles.files = $$PWD/image/*
    sourcefiles.path = $$OUT_PWD/debug/image/
} else {
    sourcefiles.files = $$PWD/Data/*
    sourcefiles.path = $$OUT_PWD/release/image/
}
INSTALLS += sourcefiles
