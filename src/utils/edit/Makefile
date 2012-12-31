#
# Makefile for edit
#

all: edit.exe less.exe

edit.exe: edit.c
    $(CC) edit.c

less.exe: edit.c
    $(CC) -o less.exe edit.c -DLESS

install: edit.exe less.exe
    cp edit.exe /bin/edit.exe
    cp less.exe /bin/less.exe

clean:
    rm edit.exe less.exe
