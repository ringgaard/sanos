#
# Make file for telnetd
#

all: telnetd.exe

telnetd.exe: telnetd.c
    $(CC) telnetd.c

install: telnetd.exe
    cp telnetd.exe $(ROOT)/bin/telnetd.exe

cross: install

clean:
    rm telnetd.exe
