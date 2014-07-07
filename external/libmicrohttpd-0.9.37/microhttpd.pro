QT -= core gui

TARGET = microhttpd
TEMPLATE = lib

SOURCES += \
    src/microhttpd/base64.c \
    src/microhttpd/basicauth.c \
    src/microhttpd/connection.c \
    src/microhttpd/daemon.c \
    src/microhttpd/digestauth.c \
    src/microhttpd/internal.c \
    src/microhttpd/md5.c \
    src/microhttpd/memorypool.c \
    src/microhttpd/postprocessor.c \
    src/microhttpd/reason_phrase.c \
    src/microhttpd/response.c \

INCLUDEPATH += src/include

win32 {
SOURCES += src/platform/w32functions.c
RC_FILE = src/microhttpd/microhttpd_dll_res.rc
INCLUDEPATH += configs/mingw
LIBS += -lwsock32
}
