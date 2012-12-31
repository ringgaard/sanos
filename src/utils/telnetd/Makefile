#
# Make file for telnetd
#

all: telnetd.exe

telnetd.exe: telnetd.c
    $(CC) telnetd.c

install: telnetd.exe
    cp telnetd.exe /bin/telnetd.exe

clean:
    rm telnetd.exe
