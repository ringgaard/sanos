//
// moddb.h
//
// Module Loader
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

#ifndef MODDB_H
#define MODDB_H

#define RVA(hmod, rva) (((char *) (hmod)) + (rva))

#define MODLOAD_NOINIT          1
#define MODLOAD_NOSHARE         2
#define MODLOAD_EXE             4
#define MODLOAD_SCRIPT          8

#define MODTYPE_DLL             1
#define MODTYPE_EXE             2
#define MODTYPE_SCRIPT          4

#define MODULE_LOADED           0x0001
#define MODULE_IMPORTS_REFED    0x0002
#define MODULE_RESOLVED         0x0004
#define MODULE_RELOCATED        0x0008
#define MODULE_BOUND            0x0010
#define MODULE_PROTECTED        0x0020
#define MODULE_INITIALIZED      0x0040

struct modalias {
  char *name;
  char *alias;
  struct modalias *next;
};

struct moddb {
  int flags;
  int (*read_magic)(char *name, char *buffer, int size);
  void *(*load_image)(char *name);
  int (*unload_image)(hmodule_t hmod, size_t size);
  int (*protect_region)(void *mem, size_t size, int protect);
  void (*notify_load)(hmodule_t hmod, char **name);
  void (*notify_unload)(hmodule_t hmod);
  void (*log)(char *msg);

  struct module *modules;
  char **modpaths;
  int nmodpaths;
  struct modalias *aliases;
};

struct module {
  hmodule_t hmod;
  struct moddb *db;
  char *name;
  char *path;
  struct module *next;
  struct module *prev;
  int refcnt;
  int flags;
};

struct stackframe {
  void *eip;
  void *ebp;
  hmodule_t hmod;
  char *modname;
  char *file;
  char *func;
  int offset;
  int line;
};

#ifdef  __cplusplus
extern "C" {
#endif

struct module *get_module_for_handle(struct moddb *db, hmodule_t hmod);
struct image_header *get_image_header(hmodule_t hmod);
void *get_proc_address(hmodule_t hmod, char *procname);
hmodule_t get_module_handle(struct moddb *db, char *name);
int get_module_filename(struct moddb *db, hmodule_t hmod, char *buffer, int size);
void *get_entrypoint(hmodule_t hmod);
hmodule_t load_module(struct moddb *db, char *name, int flags);
int unload_module(struct moddb *db, hmodule_t hmod);
int get_resource_data(hmodule_t hmod, char *id1, char *id2, char *id3, void **data);
int get_stack_trace(struct moddb *db, struct context *ctxt, void *stktop, void *stklimit, struct stackframe *frames, int depth);
int init_module_database(struct moddb *db, char *name, hmodule_t hmod, char *libpath, struct section *aliassect, int flags);

#ifdef  __cplusplus
}
#endif

#endif
