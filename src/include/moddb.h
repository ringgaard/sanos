//
// moddb.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Module Loader
//

#ifndef MODDB_H
#define MODDB_H

#define RVA(hmod, rva) (((char *) (hmod)) + (rva))

struct moddb
{
  int flags;
  void *(*load_image)(char *name);
  int (*unload_image)(hmodule_t hmod, size_t size);
  int (*protect_region)(void *mem, size_t size, int protect);
  void (*notify_load)(hmodule_t hmod);
  void (*notify_unload)(hmodule_t hmod);

  struct module *modules;
  struct module *execmod;
  char **modpaths;
  int nmodpaths;
};

struct module
{
  hmodule_t hmod;
  struct moddb *db;
  char *name;
  char *path;
  struct module *next;
  struct module *prev;
  int refcnt;
};

struct image_header *get_image_header(hmodule_t hmod);
void *get_proc_address(hmodule_t hmod, char *procname);
hmodule_t get_module_handle(struct moddb *db, char *name);
int get_module_filename(struct moddb *db, hmodule_t hmod, char *buffer, int size);
void *get_entrypoint(hmodule_t hmod);
hmodule_t load_module(struct moddb *db, char *name);
int unload_module(struct moddb *db, hmodule_t hmod);

void init_module_database(struct moddb *db, char *name, hmodule_t hmod, char *libpath, int flags);

#endif
