#-------------------------------------------------
#
# Project created by QtCreator 2014-05-14T10:17:52
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Lesson15_AddingTextures
TEMPLATE = app

LIBS += libftgl libfreetype libz libopengl32 libglu32

SOURCES += main.cpp\
        myglwidget.cpp

HEADERS  += myglwidget.h

fonts.files += fonts/*
fonts.path = $$OUT_PWD/debug
INSTALLS += fonts

RESOURCES += \
    src.qrc
