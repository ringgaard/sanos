#
# Make file for ftpd
#

all: ftpd.exe

ftpd.exe: ftpd.c
    $(CC) ftpd.c

install: ftpd.exe
    sh cp ftpd.exe /bin/ftpd.exe

clean:
    sh rm ftpd.exe
