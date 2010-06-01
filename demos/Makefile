
CC       = mips-elf-gcc
CFLAGS   = -pipe -std=c99 -g -Wall -Wextra -W $(DEFINES)
LFLAGS   = -Tidt.ld
AS       = mips-elf-as
ASFLAGS  = 
INCPATH  =
DEL_FILE = rm -f

SOURCES       = hello.s \
		x-integer.s \
		hello.c \
		arith.c \
		hangman.c \
		calculator.c
OBJECTS       = hellos.o \
		x-integer.o \
		helloc.o \
		arith.o \
		hangman.o \
		calculator.o
TARGETS       = hellos \
		x-integer \
		helloc \
		arith \
		hangman \
		calculator

first: all
####### Implicit rules

.SUFFIXES: .o .c .s

.s.o:
	$(AS) -c $(ASLAGS) $(INCPATH) -o "$@" "$<"

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"

####### Build rules

all: $(OBJECTS) $(TARGETS)

clean: FORCE 
	-$(DEL_FILE) $(OBJECTS) $(TARGETS)

hellos.o: hello.s
	$(CC) -c $(CFLAGS) $(INCPATH) -o hellos.o hello.s

hellos: hello.s
	$(CC) $(CFLAGS) $(INCPATH) -o hellos hello.s

x-integer.o: x-integer.s
	$(CC) -c $(CFLAGS) $(INCPATH) -o x-integer.o x-integer.s

x-integer: x-integer.s
	$(CC) $(CFLAGS) $(INCPATH) -o x-integer x-integer.s

helloc.o: hello.c
	$(CC) -c $(CFLAGS) $(INCPATH) -o helloc.o hello.c

arith: arith.c
	$(CC) $(CFLAGS) $(LFLAGS) $(INCPATH) -o arith arith.c

arith.o: arith.c
	$(CC) -c $(CFLAGS) $(INCPATH) -o arith.o arith.c

hangman: hangman.c
	$(CC) $(CFLAGS) $(LFLAGS) $(INCPATH) -o hangman hangman.c

hangman.o: hangman.c
	$(CC) -c $(CFLAGS) $(INCPATH) -o hangman.o hangman.c

calculator: ../util.h ../util.c \
		calculator.c
	$(CC) $(CFLAGS) $(LFLAGS) $(INCPATH) -o calculator calculator.c

calculator.o: ../util.h ../util.c \
		calculator.c
	$(CC) -c $(CFLAGS) $(INCPATH) -o calculator.o calculator.c

FORCE:
