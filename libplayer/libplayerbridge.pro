QT -= gui
QT += network xml

TARGET = playerbridge
TEMPLATE = lib
CONFIG += staticlib

HEADERS += \
    gplayerbridge.h \
    ../libproject/gdependencygraph.h \
    ../libproject/gprojectproperties.h

SOURCES += \
    gplayerbridge.cpp \
    ../libproject/gdependencygraph.cpp \
    ../libproject/gprojectproperties.cpp

INCLUDEPATH += ../libproject

