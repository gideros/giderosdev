QT -= core gui

TARGET = playerdeamon
TEMPLATE = lib

DEFINES += PLAYERDEAMON_LIBRARY

HEADERS += \
    gmd5.h \
    gplatformutil.h \
    gplayerdeamon.h \
    gutil.h

SOURCES += \
    gmd5.cpp \
    gplatformutil.cpp \
    gplayerdeamon.cpp \
    gutil.cpp

INCLUDEPATH += ../external/md5 ../external/pystring ../external/libmicrohttpd-0.9.37/src/include

LIBS += -L../external/md5/build/mingw482_32 -lmd5
LIBS += -L../external/pystring/build/mingw482_32 -lpystring
LIBS += -L../external/libmicrohttpd-0.9.37/build/mingw482_32 -lmicrohttpd
