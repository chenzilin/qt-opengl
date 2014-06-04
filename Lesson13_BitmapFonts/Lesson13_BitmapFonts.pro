#-------------------------------------------------
#
# Project created by QtCreator 2014-05-05T15:05:23
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Lesson13_BitmapFonts
TEMPLATE = app

LIBS += libfreetype libz

SOURCES += main.cpp\
        myglwidget.cpp \
    freetypestruct.cpp

HEADERS  += myglwidget.h \
    freetypestruct.h

fonts.files += fonts/*
fonts.path = $$OUT_PWD/debug
INSTALLS += fonts
