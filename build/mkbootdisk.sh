#!/bin/sh

# Initialize file system
mkfs rd0

# Mount file system
mkdir /rd
mount rd0 /rd

# Create directories
mkdir /rd/boot
mkdir /rd/bin
mkdir /rd/dev
mkdir /rd/proc

# Install system
mkboot -d /rd -b /boot/boot -l /boot/osldr.dll -k /boot/krnl.dll
cp /boot/os.dll /rd/boot/
cp /bin/sh.exe /rd/bin/

if [ "$PKG" -eq "1" ] ; then
  # Install package manager and networking
  cp /boot/rtl8139.sys /rd/boot/
  cp /bin/pkg.exe /rd/bin/

  mkdir /rd/etc
  echo '[os]' > /rd/etc/os.ini
  echo 'libpath=/bin;/usr/bin' >> /rd/etc/os.ini
  echo '[netif]' >> /rd/etc/os.ini
  echo 'eth0' >> /rd/etc/os.ini

  echo '[bindings]' > /rd/boot/krnl.ini
  echo 'pci unit 10EC8139=rtl8139.sys' >> /rd/boot/krnl.ini
fi

# Unmount file system
umount /rd

# Generate VMDK image
genvmdk /dev/rd0 /var/rd.vmdk

