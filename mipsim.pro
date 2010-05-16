
TARGET = mipsim

CONFIG += debug
CONFIG -= qt

OBJECTS_DIR = .obj

QMAKE_CFLAGS += -Wno-unused -std=c99

HEADERS += version.h config.h io.h cli.h elffile.h mipself.h mips.h mips_p.h  decode.h monitor.h
SOURCES += main.c config.c io.c cli.c elffile.c mipself.c mips.c mips_p.c decode.c memory.c monitor.c
