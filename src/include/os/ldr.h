//
// ldr.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Kernel module loader
//

#ifndef LDR_H
#define LDR_H

extern struct moddb kmods;

void *load_image_file(char *filename, int userspace);

krnlapi void *resolve(hmodule_t hmod, char *procname);
krnlapi hmodule_t getmodule(char *name);
krnlapi int getmodpath(hmodule_t hmod, char *buffer, int size);
krnlapi hmodule_t load(char *name);
krnlapi int unload(hmodule_t hmod);

void init_kernel_modules();

#endif
