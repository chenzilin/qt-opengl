# QT Modules
QT += opengl widgets

# Target
TEMPLATE = app
TARGET = Lesson

CONFIG += c++11

# Sources and Headers
SOURCE_FOLDER = src
HEADER_FOLDER = src/include

INCLUDEPATH += $$HEADER_FOLDER
HEADERS += $$HEADER_FOLDER/myglwidget.h
SOURCES += $$SOURCE_FOLDER/main.cpp \
    $$SOURCE_FOLDER/myglwidget.cpp

# Resources
RESOURCES += Lesson8.qrc
QMAKE_RESOURCE_FLAGS += -no-compress

# Libs of opengl
LIBS += -lglut -lGLU

# Objects Pathes
DESTDIR = bin
UI_DIR = build
MOC_DIR = build
RCC_DIR = build
OBJECTS_DIR = build
