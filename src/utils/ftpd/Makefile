#
# Make file for ftpd
#

all: ftpd.exe

ftpd.exe: ftpd.c
    $(CC) ftpd.c

install: ftpd.exe
    cp ftpd.exe /bin/ftpd.exe

clean:
    rm ftpd.exe
