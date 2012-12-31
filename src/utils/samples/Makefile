#
# Makefile for sanos sample programs
#

all: hello.exe hellos.exe calc.exe webserver.exe

# Hello world using C runtime library
hello.exe: hello.c
    $(CC) hello.c

# Hello world using direct Sanos API
hellos.exe: hellos.c
    $(CC) hellos.c -nostdlib -los

# Simple webserver
webserver.exe: webserver.c
    $(CC) webserver.c

# Simple calculator
calc.exe: calc.c
    $(CC) calc.c

clean:
    rm hello.exe hellos.exe calc.exe webserver.exe
