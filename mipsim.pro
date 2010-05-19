
TARGET = simips

CONFIG += debug
CONFIG -= qt

OBJECTS_DIR = .obj

QMAKE_CFLAGS += -std=c99

LIBS += -lreadline

HEADERS += version.h config.h io.h shell.h elffile.h mipself.h mips.h mips_p.h  decode.h monitor.h
SOURCES += main.c config.c io.c shell.c elffile.c mipself.c mips.c mips_p.c decode.c memory.c monitor.c
