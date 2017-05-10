CC:=gcc
CFLAGS:=-O2
LIBS:=-lgdi32 -lcomctl32

all: demo.exe

demo.exe: demo.c
	$(CC) -o$@ $(CFLAGS) $< $(LIBS)

clean:
	rm -f demo.exe

.PHONY: all clean

