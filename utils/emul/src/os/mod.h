#ifndef MOD_H
#define MOD_H

struct module
{
  hmodule_t hmod;
  char *name;
  char *path;
  struct module *next;
  struct module *prev;
  int refcnt;
};

extern hmodule_t hexecmod;

void init_modules(hmodule_t hmod, char *libpath);
void *get_entrypoint(hmodule_t hmod);

#endif
