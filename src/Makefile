#
# Main makefile for Sanos applications
#

all: sdk apps

sdk:
    make -C utils/as
    make -C utils/cc
    make -C utils/ar
    make -C utils/make
    make -C utils/impdef
    make -C utils/makedepend
    make -C lib

apps:
    make -C utils/ctohtml
    make -C utils/edit
    make -C utils/fdisk
    make -C utils/ftpd
    make -C utils/login
    make -C utils/setup
    make -C utils/sh
    make -C utils/telnetd

install: sdk apps 
    make -C lib install
    make -C utils/as install
    make -C utils/cc install
    make -C utils/ar install
    make -C utils/make install
    make -C utils/impdef install
    make -C utils/makedepend install
    make -C utils/edit install
    make -C utils/fdisk install
    make -C utils/ftpd install
    make -C utils/login install
    make -C utils/setup install
    make -C utils/sh install
    make -C utils/telnetd install

clean:
    make -C lib clean
    make -C utils/as clean
    make -C utils/cc clean
    make -C utils/ar clean
    make -C utils/make clean
    make -C utils/impdef clean
    make -C utils/makedepend clean
    make -C utils/edit clean
    make -C utils/fdisk clean
    make -C utils/ftpd clean
    make -C utils/login clean
    make -C utils/setup clean
    make -C utils/sh clean
    make -C utils/telnetd clean
