#
# Make file for telnetd
#

all: telnetd.exe

telnetd.exe: telnetd.c
    $(CC) telnetd.c

install: telnetd.exe
    sh cp telnetd.exe /bin/telnetd.exe

clean:
    sh rm telnetd.exe
