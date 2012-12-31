#
# Makefile for winmm library
#

all: winmm.dll

winmm.dll: winmm.c
    $(CC) -shared -o $@ $^

install: winmm.dll
    cp winmm.dll /bin/winmm.dll

clean:
    rm winmm.dll
