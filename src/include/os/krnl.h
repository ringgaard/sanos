//
// krnl.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//

#ifndef KRNL_H
#define KRNL_H

#include <os.h>

#include <types.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <bitops.h>
#include <rmap.h>
#include <inifile.h>
#include <moddb.h>

#include <os/tss.h>
#include <os/seg.h>
#include <os/fpu.h>
#include <os/pdir.h>
#include <os/pframe.h>
#include <os/kmem.h>
#include <os/kmalloc.h>
#include <os/vmm.h>

#include <os/syspage.h>

#include <os/pe.h>

#include <os/buf.h>

#include <os/object.h>
#include <os/queue.h>
#include <os/sched.h>
#include <os/intr.h>
#include <os/dbg.h>

#include <os/pic.h>
#include <os/pit.h>

#include <os/pci.h>
#include <os/pnpbios.h>
#include <os/dev.h>

#include <os/video.h>
#include <os/kbd.h>

#include <os/vfs.h>
#include <os/dfs.h>

#include <os/mbr.h>

#include <os/pe.h>
#include <os/ldr.h>

#include <os/syscall.h>

#include <net/net.h>

// start.c

extern devno_t bootdev;
extern struct section *krnlcfg;

void panic(char *msg);
void exit();

// cons.c

extern devno_t consdev;

krnlapi void kprintf(const char *fmt, ...);

// hd.c

void init_hd();

// fd.c

void init_fd();

// devfs.c

void init_devfs();

// Intrinsic i/o functions

int __cdecl _inp(port_t);
unsigned short __cdecl _inpw(port_t);
unsigned long __cdecl _inpd(port_t);

int __cdecl _outp(port_t, int);
unsigned short __cdecl _outpw(port_t, unsigned short);
unsigned long __cdecl _outpd(port_t, unsigned long);

#endif
