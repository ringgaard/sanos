#
# Makefile for hello world sample program
#

all: hello.exe hellos.exe

# Hello world using C runtime library
hello.exe: hello.c
    $(CC) hello.c

# Hello world using direct Sanos API
hellos.exe: hellos.c
    $(CC) hellos.c -nostdlib -los

clean:
    sh rm hello.exe hellos.exe
