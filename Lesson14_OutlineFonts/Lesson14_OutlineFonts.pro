#-------------------------------------------------
#
# Project created by QtCreator 2014-05-09T11:21:18
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Lesson14_OutlineFonts
TEMPLATE = app

LIBS += libftgl libfreetype libz libopengl32 libglu32

SOURCES += main.cpp\
        myglwidget.cpp

HEADERS  += myglwidget.h

fonts.files += fonts/*
fonts.path = $$OUT_PWD/debug
INSTALLS += fonts
