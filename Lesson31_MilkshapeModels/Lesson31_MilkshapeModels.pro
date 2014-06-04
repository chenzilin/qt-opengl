#-------------------------------------------------
#
# Project created by QtCreator 2014-02-19T10:23:49
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Lesson31_MilkshapeModels
TEMPLATE = app

SOURCES += main.cpp\
        myglwidget.cpp \
        model.cpp \
        milkshapemodel.cpp

HEADERS  += myglwidget.h \
            model.h \
            milkshapemodel.h

CONFIG(debug, debug|release) {
    sourcefiles.files = $$PWD/Data/*
    sourcefiles.path = $$OUT_PWD/debug/Data/
} else {
    sourcefiles.files = $$PWD/Data/*
    sourcefiles.path = $$OUT_PWD/release/Data/
}
INSTALLS += sourcefiles
