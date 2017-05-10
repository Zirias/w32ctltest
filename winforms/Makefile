NETFW:=C:\\Windows\\Microsoft.NET\\Framework\\v4.0.30319
CSC:=$(NETFW)\\csc.exe
LDFLAGS:=/lib:$(NETFW)
LIBS:=System.Windows.Forms.dll,System.Drawing.dll

all: demo.exe

demo.exe: demo.cs
	$(CSC) /out:$@ $(LDFLAGS) /reference:$(LIBS) $<

clean:
	rm -f demo.exe

.PHONY: all clean

