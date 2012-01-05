//
// krnl.h
//
// Main kernel include file
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.  
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.  
// 3. Neither the name of the project nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
// SUCH DAMAGE.
// 

#ifndef KRNL_H
#define KRNL_H

#include <os/config.h>
#include <os.h>

#include <sys/types.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include <bitops.h>
#include <rmap.h>
#include <inifile.h>
#include <moddb.h>
#include <verinfo.h>

#include <os/tss.h>
#include <os/seg.h>
#include <os/fpu.h>
#include <os/cpu.h>

#include <os/pdir.h>
#include <os/pframe.h>

#include <os/mach.h>

#include <os/kmem.h>
#include <os/kmalloc.h>
#include <os/vmm.h>

#include <os/syspage.h>

#include <os/pe.h>

#include <os/buf.h>

#include <os/timer.h>
#include <os/user.h>
#include <os/object.h>
#include <os/queue.h>
#include <os/sched.h>
#include <os/trap.h>
#include <os/dbg.h>
#include <os/klog.h>

#include <os/pic.h>
#include <os/pit.h>

#include <os/dev.h>
#include <os/pci.h>
#include <os/pnpbios.h>
#include <os/virtio.h>

#include <os/video.h>
#include <os/kbd.h>
#include <os/rnd.h>

#include <os/iovec.h>
#include <os/vfs.h>
#include <os/dfs.h>
#include <os/devfs.h>
#include <os/procfs.h>

#include <os/mbr.h>

#include <os/pe.h>
#include <os/ldr.h>

#include <os/syscall.h>

#include <net/net.h>

#if _MSC_VER < 1300
#pragma warning(disable : 4761)
#endif

// start.c

krnlapi extern dev_t bootdev;
krnlapi extern char krnlopts[KRNLOPTS_LEN];
krnlapi extern struct section *krnlcfg;
krnlapi extern struct peb *peb;

krnlapi void panic(char *msg);
krnlapi int license();

krnlapi void stop(int mode);

// syscall.c

void init_syscall();

// cpu.c

int cpu_proc(struct proc_file *pf, void *arg);

// smbfs.c

void init_smbfs();

// pipefs.c

void init_pipefs();
int pipe(struct file **readpipe, struct file **writepipe);

// cdfs.c

void init_cdfs();

// cons.c

extern int serial_console;
void init_console();
void console_print(char *buffer, int size);

// serial.c

void init_serial();

// ramdisk.c

int create_initrd();

// hd.c

void init_hd();

// fd.c

void init_fd();

// virtioblk.c

void init_vblk();

// apm.c

void apm_power_off();
extern int apm_enabled;

// opts.c

char *get_option(char *opts, char *name, char *buffer, int size, char *defval);
int get_num_option(char *opts, char *name, int defval);

// strtol.c

unsigned long strtoul(const char *nptr, char **endptr, int ibase);

// vsprintf.c

int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);

#endif
