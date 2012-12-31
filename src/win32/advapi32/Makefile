#
# Makefile for advapi32 library
#

all: advapi32.dll

advapi32.dll: advapi32.c
    $(CC) -shared -o $@ $^

install: advapi32.dll
    cp advapi32.dll /bin/advapi32.dll

clean:
    rm advapi32.dll
