#-------------------------------------------------
#
# Project created by QtCreator 2014-05-14T14:40:05
#
#-------------------------------------------------

QT       += core gui multimedia opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Lesson35_PlayAnAVI
TEMPLATE = app


SOURCES += main.cpp\
        myglwidget.cpp \
        videowidgetsurface.cpp

HEADERS  += myglwidget.h \
        videowidgetsurface.h

avi.files += avi/*
avi.path = $$OUT_PWD/debug
INSTALLS += avi
