QT -= core gui

TARGET = playerdaemon
TEMPLATE = lib

DEFINES += PLAYERDAEMON_LIBRARY

HEADERS += \
    gmd5.h \
    gplatformutil.h \
    gplayerdaemon.h \
    gutil.h

SOURCES += \
    gmd5.cpp \
    gplatformutil.cpp \
    gplayerdaemon.cpp \
    gutil.cpp

INCLUDEPATH += ../external/md5 ../external/pystring ../external/libmicrohttpd-0.9.37/src/include

LIBS += -L../external/md5/build/mingw482_32 -lmd5
LIBS += -L../external/pystring/build/mingw482_32 -lpystring
LIBS += -L../external/libmicrohttpd-0.9.37/build/mingw482_32 -lmicrohttpd
