#
# Main makefile for Sanos
#

all: sdk sys utils cmds

sdk:
    make -C lib
    make -C utils/as
    make -C utils/cc
    make -C utils/ar
    make -C utils/make
    make -C utils/impdef
    make -C utils/yacc

sys:
    make -C sys

utils:
    make -C utils/ctohtml
    make -C utils/edit
    make -C utils/fdisk
    make -C utils/ftpd
    make -C utils/login
    make -C utils/mkboot
    make -C utils/setup
    make -C utils/sh
    make -C utils/telnetd

cmds:
    make -C cmds

extra:
    make -C utils/zlib
    make -C utils/tar
    make -C utils/lua
    make -C utils/awk
    make -C utils/makedepend

install: install-sdk install-sys install-utils install-cmds

install-sdk:
    make -C lib install
    make -C utils/as install
    make -C utils/cc install
    make -C utils/ar install
    make -C utils/make install
    make -C utils/impdef install
    make -C utils/yacc install

install-sys:
    make -C sys install

install-utils:
    make -C utils/ctohtml install
    make -C utils/edit install
    make -C utils/fdisk install
    make -C utils/ftpd install
    make -C utils/login install
    make -C utils/mkboot install
    make -C utils/setup install
    make -C utils/sh install
    make -C utils/telnetd install

install-cmds:
    make -C cmds install

install-extra:
    make -C utils/zlib install
    make -C utils/tar install
    make -C utils/lua install
    make -C utils/awk install
    make -C utils/makedepend install

clean: clean-sdk clean-sys clean-utils clean-cmds

clean-sdk:
    make -C lib clean
    make -C utils/as clean
    make -C utils/cc clean
    make -C utils/ar clean
    make -C utils/make clean
    make -C utils/impdef clean
    make -C utils/yacc clean

clean-sys:
    make -C sys clean

clean-utils:
    make -C utils/edit clean
    make -C utils/fdisk clean
    make -C utils/ftpd clean
    make -C utils/login clean
    make -C utils/setup clean
    make -C utils/sh clean
    make -C utils/telnetd clean

clean-cmds:
    make -C cmds clean

clean-extra:
    make -C utils/zlib clean
    make -C utils/tar clean
    make -C utils/lua clean
    make -C utils/awk clean
    make -C utils/makedepend clean

.PHONY: utils sys cmds

