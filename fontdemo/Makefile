CC:=gcc
CFLAGS:=-std=c11 -Wall -Wextra -pedantic
ifdef DEBUG
CFLAGS+=-g3 -O0
else
CFLAGS+=-O2
endif

LIBS:=-lgdi32 -lcomctl32

all: fontdemo.exe

fontdemo.exe: fontdemo.c
	$(CC) -o$@ $(CFLAGS) $< $(LIBS)

clean:
	rm -f fontdemo.exe

.PHONY: all clean

