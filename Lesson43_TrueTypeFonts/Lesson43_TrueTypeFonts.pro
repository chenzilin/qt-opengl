#-------------------------------------------------
#
# Project created by QtCreator 2014-04-15T22:56:37
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Lesson43_TrueTypeFonts
TEMPLATE = app

LIBS += libfreetype libz

#INCLUDEPATH += freetype2

SOURCES += main.cpp\
        myglwidget.cpp

HEADERS  += myglwidget.h

fonts.files += fonts/*
fonts.path = $$OUT_PWD/debug
INSTALLS += fonts
