#
# Makefile for shell commands
#

CMDS=find.exe grep.exe ping.exe
ALLCMDS=chgrp.exe chmod.exe chown.exe cp.exe du.exe ls.exe mkdir.exe mv.exe rm.exe test.exe touch.exe wc.exe $(CMDS)

cmds: $(CMDS) 
all: $(ALLCMDS)

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

find.exe: find.c
    $(CC) -o $@ $^ -DUSE_LOCAL_HEAP -noshare

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

touch.exe: touch.c
    $(CC) -o $@ $^

wc.exe: wc.c
    $(CC) -o $@ $^

install: $(CMDS)
    cp $(CMDS) /bin

install-all: $(ALLCMDS)
    cp $(ALLCMDS) /bin

clean:
    rm *.exe

