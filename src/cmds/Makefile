#
# Makefile for shell commands
#

CMDS=chgrp.exe chmod.exe chown.exe cp.exe du.exe grep.exe ls.exe mkdir.exe mv.exe ping.exe rm.exe test.exe wc.exe

all: $(CMDS)

chgrp.exe: chgrp.c
    $(CC) -o $@ $^

chmod.exe: chmod.c
    $(CC) -o $@ $^

chown.exe: chown.c
    $(CC) -o $@ $^

cp.exe: cp.c
    $(CC) -o $@ $^

du.exe: du.c
    $(CC) -o $@ $^

grep.exe: grep.c
    $(CC) -o $@ $^

ls.exe: ls.c
    $(CC) -o $@ $^

mkdir.exe: mkdir.c
    $(CC) -o $@ $^

mv.exe: mv.c
    $(CC) -o $@ $^

ping.exe: ping.c
    $(CC) -o $@ $^

rm.exe: rm.c
    $(CC) -o $@ $^

test.exe: test.c
    $(CC) -o $@ $^

wc.exe: wc.c
    $(CC) -o $@ $^

install: $(CMDS)
    cp $(CMDS) /bin

clean:
    rm $(CMDS)

