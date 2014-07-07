#-------------------------------------------------
#
# Project created by QtCreator 2014-07-07T17:39:35
#
#-------------------------------------------------

QT       += core network xml

QT       -= gui

TARGET = gdrbridge
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    gemitter.cpp \
    ../libplayer/gmd5.cpp \
    ../libplayer/gplatformutil.cpp \
    ../libplayer/gutil.cpp

INCLUDEPATH += ../libplayer

LIBS += -L../libplayer/release -lplayerbridge

HEADERS += \
    gemitter.h \
    ../libplayer/gmd5.h \
    ../libplayer/gplatformutil.h \
    ../libplayer/gutil.h


INCLUDEPATH += ../external/md5 ../external/pystring

LIBS += -L../external/md5/build/mingw482_32 -lmd5
LIBS += -L../external/pystring/build/mingw482_32 -lpystring
