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

sys:
    make -C sys

utils:
    make -C utils/ctohtml
    make -C utils/edit
    make -C utils/fdisk
    make -C utils/ftpd
    make -C utils/login
    make -C utils/mkboot
    make -C utils/genvmdk
    make -C utils/pkg
    make -C utils/setup
    make -C utils/sh
    make -C utils/telnetd

cmds:
    make -C cmds

install: install-sdk install-sys install-utils install-cmds

install-sdk:
    make -C lib install
    make -C utils/as install
    make -C utils/cc install
    make -C utils/ar install
    make -C utils/make install
    make -C utils/impdef install

install-sys:
    make -C sys install

install-utils:
    make -C utils/ctohtml install
    make -C utils/edit install
    make -C utils/fdisk install
    make -C utils/ftpd install
    make -C utils/login install
    make -C utils/mkboot install
    make -C utils/genvmdk install
    make -C utils/pkg install
    make -C utils/setup install
    make -C utils/sh install
    make -C utils/telnetd install

install-cmds:
    make -C cmds install

cross:
    make -C sys cross
    make -C lib cross
    make -C utils/as cross
    make -C utils/cc cross
    make -C utils/ar cross
    make -C utils/make cross
    make -C utils/impdef cross
    make -C utils/ctohtml cross
    make -C utils/edit cross
    make -C utils/fdisk cross
    make -C utils/ftpd cross
    make -C utils/login cross
    make -C utils/mkboot cross
    make -C utils/genvmdk cross
    make -C utils/pkg cross
    make -C utils/setup cross
    make -C utils/sh cross
    make -C utils/telnetd cross
    make -C cmds cross

clean: clean-sdk clean-sys clean-utils clean-cmds

clean-sdk:
    make -C lib clean
    make -C utils/as clean
    make -C utils/cc clean
    make -C utils/ar clean
    make -C utils/make clean
    make -C utils/impdef clean

clean-sys:
    make -C sys clean

clean-utils:
    make -C utils/ctohtml clean
    make -C utils/edit clean
    make -C utils/fdisk clean
    make -C utils/ftpd clean
    make -C utils/login clean
    make -C utils/mkboot clean
    make -C utils/genvmdk clean
    make -C utils/pkg clean
    make -C utils/setup clean
    make -C utils/sh clean
    make -C utils/telnetd clean

clean-cmds:
    make -C cmds clean

.PHONY: utils sys cmds

