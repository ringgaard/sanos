#
# Makefile for device drivers
#

LIB=/usr/src/lib
DRIVERS=3c905c.sys eepro100.sys ne2000.sys pcnet32.sys rtl8139.sys sis900.sys tulip.sys virtionet.sys

all: $(DRIVERS)

3c905c.sys: 3c905c.c $(LIB)/string.c
    $(CC) -shared -entry _start@12 -nostdlib -lkrnl -o $@ -D KERNEL $**

eepro100.sys: eepro100.c $(LIB)/opts.c $(LIB)/string.c $(LIB)/strtol.c $(LIB)/ctype.c
    $(CC) -shared -entry _start@12 -nostdlib -lkrnl -o $@ -D KERNEL $**

ne2000.sys: ne2000.c $(LIB)/opts.c $(LIB)/string.c $(LIB)/strtol.c $(LIB)/ctype.c
    $(CC) -shared -entry _start@12 -nostdlib -lkrnl -o $@ -D KERNEL $**

pcnet32.sys: pcnet32.c $(LIB)/string.c $(LIB)/ctype.c
    $(CC) -shared -entry _start@12 -nostdlib -lkrnl -o $@ -D KERNEL $**

rtl8139.sys: rtl8139.c $(LIB)/opts.c $(LIB)/string.c $(LIB)/strtol.c $(LIB)/ctype.c
    $(CC) -shared -entry _start@12 -nostdlib -lkrnl -o $@ -D KERNEL $**

sis900.sys: sis900.c $(LIB)/opts.c $(LIB)/string.c $(LIB)/strtol.c $(LIB)/ctype.c
    $(CC) -shared -entry _start@12 -nostdlib -lkrnl -o $@ -D KERNEL $**

tulip.sys: tulip.c $(LIB)/opts.c $(LIB)/string.c $(LIB)/strtol.c $(LIB)/ctype.c
    $(CC) -shared -entry _start@12 -nostdlib -lkrnl -o $@ -D KERNEL $**

virtionet.sys: virtionet.c /usr/src/lib/string.c
    $(CC) -shared -entry _start@12 -nostdlib -lkrnl -o $@ -D KERNEL $**

install: $(DRIVERS)
    cp 3c905c.sys /boot/3c905c.sys
    cp eepro100.sys /boot/eepro100.sys
    cp ne2000.sys /boot/ne2000.sys
    cp pcnet32.sys /boot/pcnet32.sys 
    cp rtl8139.sys /boot/rtl8139.sys 
    cp sis900.sys /boot/sis900.sys
    cp tulip.sys /boot/tulip.sys
    cp virtionet.sys /boot/virtionet.sys

clean:
    rm $(DRIVERS)

