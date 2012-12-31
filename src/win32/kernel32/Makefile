#
# Makefile for kernel32 library
#

all: kernel32.dll

kernel32.dll: kernel32.c
    $(CC) -shared -o $@ $^

install: kernel32.dll
    cp kernel32.dll /bin/kernel32.dll

clean:
    rm kernel32.dll
