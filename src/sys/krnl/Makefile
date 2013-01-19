#
# Makefile for Sanos kernel
#

KRNL_SRCS=\
  apm.c \
  buf.c \
  cpu.c \
  dbg.c \
  dev.c \
  fpu.c \
  hndl.c \
  iomux.c \
  iop.c \
  iovec.c \
  kmalloc.c \
  kmem.c \
  ldr.c \
  mach.c \
  object.c \
  pci.c \
  pdir.c \
  pframe.c \
  pic.c \
  pit.c \
  pnpbios.c \
  queue.c \
  sched.c \
  start.c \
  syscall.c \
  timer.c \
  trap.c \
  user.c \
  vfs.c \
  virtio.c \
  vmi.c \
  vmm.c

DEV_SRCS=\
  ../dev/cons.c \
  ../dev/fd.c \
  ../dev/hd.c \
  ../dev/kbd.c \
  ../dev/klog.c \
  ../dev/null.c \
  ../dev/nvram.c \
  ../dev/ramdisk.c \
  ../dev/rnd.c \
  ../dev/serial.c \
  ../dev/smbios.c \
  ../dev/video.c \
  ../dev/virtioblk.c \
  ../dev/virtiocon.c

NET_SRCS=\
  ../net/arp.c \
  ../net/dhcp.c \
  ../net/ether.c \
  ../net/icmp.c \
  ../net/inet.c \
  ../net/ipaddr.c \
  ../net/ip.c \
  ../net/loopif.c \
  ../net/netif.c \
  ../net/pbuf.c \
  ../net/raw.c \
  ../net/rawsock.c \
  ../net/socket.c \
  ../net/stats.c \
  ../net/tcp.c \
  ../net/tcp_input.c \
  ../net/tcp_output.c \
  ../net/tcpsock.c \
  ../net/udp.c \
  ../net/udpsock.c \

FS_SRCS=\
  ../fs/cdfs/cdfs.c \
  ../fs/devfs/devfs.c \
  ../fs/dfs/dfs.c \
  ../fs/dfs/dir.c \
  ../fs/dfs/file.c \
  ../fs/dfs/group.c \
  ../fs/dfs/inode.c \
  ../fs/dfs/super.c \
  ../fs/pipefs/pipefs.c \
  ../fs/procfs/procfs.c \
  ../fs/smbfs/smbcache.c \
  ../fs/smbfs/smbfs.c \
  ../fs/smbfs/smbproto.c \
  ../fs/smbfs/smbutil.c

LIB_SRCS=\
  /usr/src/lib/bitops.c \
  /usr/src/lib/ctype.c \
  /usr/src/lib/inifile.c \
  /usr/src/lib/moddb.c \
  /usr/src/lib/opts.c \
  /usr/src/lib/rmap.c \
  /usr/src/lib/string.c \
  /usr/src/lib/strtol.c \
  /usr/src/lib/tcccrt.c \
  /usr/src/lib/time.c \
  /usr/src/lib/verinfo.c \
  /usr/src/lib/vsprintf.c

SRCS=$(KRNL_SRCS) $(DEV_SRCS) $(NET_SRCS) $(FS_SRCS) $(LIB_SRCS) 

all: krnl.dll
    cp krnl.def /usr/lib/krnl.def

krnl.dll: $(SRCS)
    $(CC) -shared -entry _start@12 -fixed 0x80000000 -filealign 4096 -nostdlib -o krnl.dll -def krnl.def -D KERNEL -D KRNL_LIB $(SRCS)

install: krnl.dll
    cp krnl.def /usr/lib/krnl.def
    mkboot -k krnl.dll

clean:
    rm krnl.dll krnl.def

