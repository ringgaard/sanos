#
# Make file for ftpd
#

all: ftpd.exe

ftpd.exe: ftpd.c
    $(CC) ftpd.c

install: ftpd.exe
    cp ftpd.exe $(ROOT)/bin/ftpd.exe

cross: install

clean:
    rm ftpd.exe
