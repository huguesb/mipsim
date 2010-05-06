
TARGET = mipsim

CONFIG += debug
CONFIG -= qt

OBJECTS_DIR = .obj

QMAKE_CFLAGS += -std=c99

HEADERS += mipsim.h elffile.h mips.h
SOURCES += main.c elffile.c mips.c
