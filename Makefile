#############################################################################
# Makefile for building: simips
# Generated by qmake (2.01a) (Qt 4.6.2) on: Thu May 27 20:28:01 2010
# Project:  mipsim.pro
# Template: app
# Command: /usr/bin/qmake -unix -o Makefile mipsim.pro
#############################################################################

####### Compiler, tools and options

CC            = gcc
CXX           = g++
DEFINES       = -D_SHELL_USE_READLINE_
CFLAGS        = -pipe -std=c99 -g -Wall -W $(DEFINES)
CXXFLAGS      = -pipe -g -Wall -W $(DEFINES)
INCPATH       = -I/usr/share/qt/mkspecs/linux-g++ -I.
LINK          = g++
LFLAGS        = -Wl,--hash-style=gnu -Wl,--as-needed
LIBS          = $(SUBLIBS)   -lreadline 
AR            = ar cqs
RANLIB        = 
QMAKE         = /usr/bin/qmake
TAR           = tar -cf
COMPRESS      = gzip -9f
COPY          = cp -f
SED           = sed
COPY_FILE     = $(COPY)
COPY_DIR      = $(COPY) -r
STRIP         = strip
INSTALL_FILE  = install -m 644 -p
INSTALL_DIR   = $(COPY_DIR)
INSTALL_PROGRAM = install -m 755 -p
DEL_FILE      = rm -f
SYMLINK       = ln -f -s
DEL_DIR       = rmdir
MOVE          = mv -f
CHK_DIR_EXISTS= test -d
MKDIR         = mkdir -p

####### Output directory

OBJECTS_DIR   = .obj/

####### Files

SOURCES       = main.c \
		util.c \
		config.c \
		io.c \
		shell.c \
		elffile.c \
		mipself.c \
		mips.c \
		mips_p.c \
		decode.c \
		memory.c \
		monitor.c 
OBJECTS       = .obj/main.o \
		.obj/util.o \
		.obj/config.o \
		.obj/io.o \
		.obj/shell.o \
		.obj/elffile.o \
		.obj/mipself.o \
		.obj/mips.o \
		.obj/mips_p.o \
		.obj/decode.o \
		.obj/memory.o \
		.obj/monitor.o
DIST          = /usr/share/qt/mkspecs/common/g++.conf \
		/usr/share/qt/mkspecs/common/unix.conf \
		/usr/share/qt/mkspecs/common/linux.conf \
		/usr/share/qt/mkspecs/qconfig.pri \
		/usr/share/qt/mkspecs/features/qt_functions.prf \
		/usr/share/qt/mkspecs/features/qt_config.prf \
		/usr/share/qt/mkspecs/features/exclusive_builds.prf \
		/usr/share/qt/mkspecs/features/default_pre.prf \
		/usr/share/qt/mkspecs/features/debug.prf \
		/usr/share/qt/mkspecs/features/default_post.prf \
		/usr/share/qt/mkspecs/features/warn_on.prf \
		/usr/share/qt/mkspecs/features/resources.prf \
		/usr/share/qt/mkspecs/features/uic.prf \
		/usr/share/qt/mkspecs/features/yacc.prf \
		/usr/share/qt/mkspecs/features/lex.prf \
		/usr/share/qt/mkspecs/features/include_source_dir.prf \
		mipsim.pro
QMAKE_TARGET  = simips
DESTDIR       = 
TARGET        = simips

first: all
####### Implicit rules

.SUFFIXES: .o .c .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"

####### Build rules

all: Makefile $(TARGET)

$(TARGET):  $(OBJECTS)  
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJCOMP) $(LIBS)

Makefile: mipsim.pro  /usr/share/qt/mkspecs/linux-g++/qmake.conf /usr/share/qt/mkspecs/common/g++.conf \
		/usr/share/qt/mkspecs/common/unix.conf \
		/usr/share/qt/mkspecs/common/linux.conf \
		/usr/share/qt/mkspecs/qconfig.pri \
		/usr/share/qt/mkspecs/features/qt_functions.prf \
		/usr/share/qt/mkspecs/features/qt_config.prf \
		/usr/share/qt/mkspecs/features/exclusive_builds.prf \
		/usr/share/qt/mkspecs/features/default_pre.prf \
		/usr/share/qt/mkspecs/features/debug.prf \
		/usr/share/qt/mkspecs/features/default_post.prf \
		/usr/share/qt/mkspecs/features/warn_on.prf \
		/usr/share/qt/mkspecs/features/resources.prf \
		/usr/share/qt/mkspecs/features/uic.prf \
		/usr/share/qt/mkspecs/features/yacc.prf \
		/usr/share/qt/mkspecs/features/lex.prf \
		/usr/share/qt/mkspecs/features/include_source_dir.prf
	$(QMAKE) -unix -o Makefile mipsim.pro
/usr/share/qt/mkspecs/common/g++.conf:
/usr/share/qt/mkspecs/common/unix.conf:
/usr/share/qt/mkspecs/common/linux.conf:
/usr/share/qt/mkspecs/qconfig.pri:
/usr/share/qt/mkspecs/features/qt_functions.prf:
/usr/share/qt/mkspecs/features/qt_config.prf:
/usr/share/qt/mkspecs/features/exclusive_builds.prf:
/usr/share/qt/mkspecs/features/default_pre.prf:
/usr/share/qt/mkspecs/features/debug.prf:
/usr/share/qt/mkspecs/features/default_post.prf:
/usr/share/qt/mkspecs/features/warn_on.prf:
/usr/share/qt/mkspecs/features/resources.prf:
/usr/share/qt/mkspecs/features/uic.prf:
/usr/share/qt/mkspecs/features/yacc.prf:
/usr/share/qt/mkspecs/features/lex.prf:
/usr/share/qt/mkspecs/features/include_source_dir.prf:
qmake:  FORCE
	@$(QMAKE) -unix -o Makefile mipsim.pro

dist: 
	@$(CHK_DIR_EXISTS) .obj/simips1.0.0 || $(MKDIR) .obj/simips1.0.0 
	$(COPY_FILE) --parents $(SOURCES) $(DIST) .obj/simips1.0.0/ && (cd `dirname .obj/simips1.0.0` && $(TAR) simips1.0.0.tar simips1.0.0 && $(COMPRESS) simips1.0.0.tar) && $(MOVE) `dirname .obj/simips1.0.0`/simips1.0.0.tar.gz . && $(DEL_FILE) -r .obj/simips1.0.0


clean:compiler_clean 
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core


####### Sub-libraries

distclean: clean
	-$(DEL_FILE) $(TARGET) 
	-$(DEL_FILE) Makefile


compiler_rcc_make_all:
compiler_rcc_clean:
compiler_uic_make_all:
compiler_uic_clean:
compiler_image_collection_make_all: qmake_image_collection.cpp
compiler_image_collection_clean:
	-$(DEL_FILE) qmake_image_collection.cpp
compiler_yacc_decl_make_all:
compiler_yacc_decl_clean:
compiler_yacc_impl_make_all:
compiler_yacc_impl_clean:
compiler_lex_make_all:
compiler_lex_clean:
compiler_clean: 

####### Compile

.obj/main.o: main.c version.h \
		config.h \
		shell.h \
		mips.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o .obj/main.o main.c

.obj/util.o: util.c util.h \
		io.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o .obj/util.o util.c

.obj/config.o: config.c config.h \
		mips.h \
		io.h \
		util.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o .obj/config.o config.c

.obj/io.o: io.c io.h \
		config.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o .obj/io.o io.c

.obj/shell.o: shell.c shell.h \
		mips.h \
		io.h \
		util.h \
		config.h \
		mipself.h \
		elffile.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o .obj/shell.o shell.c

.obj/elffile.o: elffile.c elffile.h \
		io.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o .obj/elffile.o elffile.c

.obj/mipself.o: mipself.c mipself.h \
		mips.h \
		elffile.h \
		io.h \
		config.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o .obj/mipself.o mipself.c

.obj/mips.o: mips.c mips.h \
		io.h \
		util.h \
		decode.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o .obj/mips.o mips.c

.obj/mips_p.o: mips_p.c mips_p.h \
		mips.h \
		io.h \
		monitor.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o .obj/mips_p.o mips_p.c

.obj/decode.o: decode.c decode.h \
		mips.h \
		io.h \
		util.h \
		monitor.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o .obj/decode.o decode.c

.obj/memory.o: memory.c mips.h \
		io.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o .obj/memory.o memory.c

.obj/monitor.o: monitor.c monitor.h \
		mips.h \
		io.h \
		config.h
	$(CC) -c $(CFLAGS) $(INCPATH) -o .obj/monitor.o monitor.c

####### Install

install:   FORCE

uninstall:   FORCE

FORCE:

