//
// moddb.c
//
// Module loader
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

#include <os.h>
#include <os/pe.h>
#include <string.h>
#include <stdarg.h>
#include <inifile.h>

#include <moddb.h>

#ifdef KERNEL
#include <os/krnl.h>
#endif

int vsprintf(char *buf, const char *fmt, va_list args);

static void logmsg(struct moddb *db, const char *fmt, ...)
{
  va_list args;
  char buffer[1024];

  if (db->log)
  {
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);
    db->log(buffer);
  }
}

static char *get_basename(char *filename, char *buffer, int size)
{
  char *basename = filename;
  char *bufend = buffer + size - 1;
  char *p = filename;

  while (*p && buffer < bufend)
  {
    *buffer = *p;
    if (*buffer == PS1 || *buffer == PS2) basename = buffer + 1;
    buffer++;
    p++;
  }
  *buffer = 0;

  return basename;
}

static void insert_before(struct module *m, struct module *n)
{
  n->next = m;
  n->prev = m->prev;
  m->prev->next = n;
  m->prev = n;
}

static void insert_after(struct module *m, struct module *n)
{
  n->next = m->next;
  n->prev = m;
  m->next->prev = n;
  m->next = n;
}

static void remove(struct module *m)
{
  m->next->prev = m->prev;
  m->prev->next = m->next;
}

struct image_header *get_image_header(hmodule_t hmod)
{
  struct dos_header *doshdr = (struct dos_header *) hmod;

  if (doshdr == NULL) return NULL;
  return (struct image_header *) RVA(hmod, doshdr->e_lfanew);
}

static void *get_image_directory(hmodule_t hmod, int dir)
{
  unsigned long addr = get_image_header(hmod)->optional.data_directory[dir].virtual_address;
  return (void *) addr ? RVA(hmod, addr) : 0;
}

struct module *get_module_for_handle(struct moddb *db, hmodule_t hmod)
{
  struct module *m;

  // Try to find module in module list
  m = db->modules;
  while (1)
  {
    if (hmod == m->hmod) return m;
    m = m->next;
    if (m == db->modules) break;
  }

  return NULL;
}

static char *find_in_modpaths(struct moddb *db, char *name, char *path)
{
  int i;
  char *p;
  char *s;
  char *basename;
  char *dot;
  char *pathend = path + MAXPATH - 1;
  int len;
  struct modalias *ma;

  for (i = 0; i < db->nmodpaths; i++)
  {
    // Build path name
    len = strlen(db->modpaths[i]);
    if (len > MAXPATH - 2) continue;
    p = path;
    memcpy(p, db->modpaths[i], len);
    p += len;

    *p++ = PS1;

    basename = p;
    dot = NULL;
    s = name;
    while (*s && p < pathend)
    {
      if (*s == '.')
      {
        dot = s;
        *p++ = '.';
      }
      else if (*s >= 'A' && *s <= 'Z')
        *p++ = *s + ('a' - 'A');
      else
        *p++ = *s;

      s++;
    }

    if (!dot && p + 4 < pathend)
    {
      memcpy(p, ".dll", 4);
      p += 4;
    }

    *p = 0;

    // Check for alias
    ma = db->aliases;
    while (ma)
    {
      if (strcmp(basename, ma->name) == 0 && basename + strlen(ma->name) < pathend)
      {
        strcpy(basename, ma->alias);
        break;
      }

      ma = ma->next;
    }

    // Return path if file exists
    if (stat(path, NULL) >= 0) return basename;
  }

  return NULL;
}

static char *get_module_name(struct moddb *db, char *name, char *path)
{
  char *dot;
  char *basename;
  char *s;

  // Check for file path in module name
  s = name;
  while (*s != 0 && *s != PS1 && *s != PS2) s++;

  if (*s)
  {
    // Get full path name for module
    basename = get_basename(name, path, MAXPATH);

    // Convert base name to lower case
    dot = NULL;
    s = basename;
    while (*s)
    {
      if (*s == '.')
        dot = s;
      else if (*s >= 'A' && *s <= 'Z')
        *s = *s + ('a' - 'A');

      s++;
    }

    // Add .dll to name if no extension
    if (!dot)
    {
      memcpy(s, ".dll", 4);
      s += 4;
    }

    *s = 0;
  }
  else
  {
    // Search for file in library paths
    basename = find_in_modpaths(db, name, path);
  }

  return basename;
}

static struct module *get_module(struct moddb *db, char *name)
{
  char buffer[MAXPATH];
  char *basename;
  char *dot;
  char *p;
  char *s;
  struct module *m;
  struct modalias *ma;

  // Get canonical module name
  basename = name;
  while (*name)
  {
    if (*name == PS1 || *name == PS2) basename = name + 1;
    name++;
  }

  p = buffer;
  s = basename;
  dot = NULL;
  while (*s)
  {
    if (p - buffer == MAXPATH - 1) break;

    if (*s == '.')
    {
      dot = s;
      *p++ = '.';
    }
    else if (*s >= 'A' && *s <= 'Z')
      *p++ = *s + ('a' - 'A');
    else
      *p++ = *s;

    s++;
  }

  if (!dot && p - buffer < MAXPATH - 5)
  {
    memcpy(p, ".dll", 4);
    p += 4;
  }

  *p = 0;

  // Check for alias
  ma = db->aliases;
  while (ma)
  {
    if (strcmp(buffer, ma->name) == 0) return get_module(db, ma->alias);
    ma = ma->next;
  }

  // Try to find module in module list
  m = db->modules;
  while (1)
  {
    if (strcmp(buffer, m->name) == 0) return m;
    m = m->next;
    if (m == db->modules) break;
  }

  return NULL;
}

static void *get_proc_by_name(hmodule_t hmod, int hint, char *procname)
{
  struct image_export_directory *exp;
  unsigned int *names;
  unsigned int i;

  exp = (struct image_export_directory *) get_image_directory(hmod, IMAGE_DIRECTORY_ENTRY_EXPORT);
  if (!exp) return NULL;

  names = (unsigned int *) RVA(hmod, exp->address_of_names);

  if (hint >= 0 && hint < (int) exp->number_of_names && strcmp(procname, RVA(hmod, names[hint])) == 0)
  {
    unsigned short idx;

    idx = *((unsigned short *) RVA(hmod, exp->address_of_name_ordinals) + hint);
    return RVA(hmod, *((unsigned long *) RVA(hmod, exp->address_of_functions) + idx));
  }

  for (i = 0; i < exp->number_of_names; i++)
  {
    char *name = RVA(hmod,  *names);
    if (strcmp(name, procname) == 0)
    {
      unsigned short idx;
      
      idx = *((unsigned short *) RVA(hmod, exp->address_of_name_ordinals) + i);
      return RVA(hmod, *((unsigned long *) RVA(hmod, exp->address_of_functions) + idx));
    }

    names++;
  }

  return NULL;
}

static void *get_proc_by_ordinal(hmodule_t hmod, unsigned int ordinal)
{
  struct image_export_directory *exp;

  exp = (struct image_export_directory *) get_image_directory(hmod, IMAGE_DIRECTORY_ENTRY_EXPORT);
  if (!exp) return NULL;

  if (ordinal < exp->base || ordinal >= exp->number_of_functions + exp->base) panic("invalid ordinal");
  return RVA(hmod, *((unsigned long *) RVA(hmod, exp->address_of_functions) + (ordinal - exp->base)));
}

static struct module *resolve_imports(struct module *mod)
{
  struct image_import_descriptor *imp;
  struct module *modlist = mod;
  char path[MAXPATH];

  // Find import directory in image
  imp = (struct image_import_descriptor *) get_image_directory(mod->hmod, IMAGE_DIRECTORY_ENTRY_IMPORT);
  if (!imp) return mod;

  // Load each dependent module
  while (imp->characteristics != 0)
  {
    char *name = RVA(mod->hmod, imp->name);
    struct module *newmod = get_module(mod->db, name);

    if (newmod == NULL)
    {
      char *imgbase;

      name = get_module_name(mod->db, name, path);
      if (!name)
      {
        logmsg(mod->db, "module %s not found", RVA(mod->hmod, imp->name));
        return NULL;
      }

      imgbase = mod->db->load_image(path);
      if (imgbase == NULL) 
      {
        logmsg(mod->db, "unable to load module %s", path);
        return NULL;
      }

      newmod = (struct module *) malloc(sizeof(struct module));
      newmod->hmod = imgbase;
      newmod->db = mod->db;
      newmod->name = strdup(name);
      newmod->path = strdup(path);
      newmod->refcnt = 0;
      newmod->flags = MODULE_LOADED;
      insert_before(mod, newmod);

      newmod = resolve_imports(newmod);
      if (newmod == NULL) return NULL;

      if (modlist == mod) modlist = newmod;
    }
    
    imp++;
  }

  return modlist;
}

static int bind_imports(struct module *mod)
{
  struct image_import_descriptor *imp;
  int errs = 0;

  // Find import directory in image
  imp = (struct image_import_descriptor *) get_image_directory(mod->hmod, IMAGE_DIRECTORY_ENTRY_IMPORT);
  if (!imp) return 0;
  
  if (imp->forwarder_chain != 0 && imp->forwarder_chain != 0xFFFFFFFF)
  {
    logmsg(mod->db, "import forwarder chains not supported (%s)", mod->name);
    return -ENOSYS;
  }

  // Update Import Address Table (IAT)
  while (imp->characteristics != 0)
  {
    unsigned long *thunks;
    unsigned long *origthunks;
    struct image_import_by_name *ibn;
    char *name;
    struct module *expmod;

    name = RVA(mod->hmod, imp->name);
    expmod = get_module(mod->db, name);
    
    if (!expmod) 
    {
      logmsg(mod->db, "module %s no longer loaded", name);
      return -ENOEXEC;
    }
    
    thunks = (unsigned long *) RVA(mod->hmod, imp->first_thunk);
    origthunks = (unsigned long *) RVA(mod->hmod, imp->original_first_thunk);
    while (*thunks)
    {
      if (*origthunks & IMAGE_ORDINAL_FLAG)
      {
        // Import by ordinal
        unsigned long ordinal = *origthunks & ~IMAGE_ORDINAL_FLAG;
        *thunks = (unsigned long) get_proc_by_ordinal(expmod->hmod, ordinal);
        if (*thunks == 0) 
        {
          logmsg(mod->db, "unable to resolve %s:#%d in %s", expmod->name, ordinal, mod->name);
          errs++;
        }
      }
      else
      {
        // Import by name (and hint)
        ibn = (struct image_import_by_name *) RVA(mod->hmod, *origthunks);
        *thunks = (unsigned long) get_proc_by_name(expmod->hmod, ibn->hint, ibn->name);
        if (*thunks == 0)
        {
          logmsg(mod->db, "unable to resolve %s:%s in %s", expmod->name, ibn->name, mod->name);
          errs++;
        }
      }

      thunks++;
      origthunks++;
    }

    imp++;
  }

  if (errs) return -ENOEXEC;

  mod->flags |= MODULE_BOUND;
  return 0;
}

static int relocate_module(struct module *mod)
{
  unsigned long offset;
  char *pagestart;
  unsigned short *fixup;
  int i;
  struct image_base_relocation *reloc;
  int nrelocs;

  offset = (unsigned long) mod->hmod - get_image_header(mod->hmod)->optional.image_base;
  if (offset == 0) return 0;

  if (get_image_header(mod->hmod)->header.characteristics & IMAGE_FILE_RELOCS_STRIPPED) 
  {
    logmsg(mod->db, "relocation info missing for %s", mod->name);
    return -ENOEXEC;
  }

  reloc = (struct image_base_relocation *) get_image_directory(mod->hmod, IMAGE_DIRECTORY_ENTRY_BASERELOC);
  if (!reloc) return 0;

  while (reloc->virtual_address != 0 || reloc->size_of_block != 0)
  {
    pagestart = RVA(mod->hmod, reloc->virtual_address);
    nrelocs = (reloc->size_of_block - sizeof(struct image_base_relocation)) / 2;
    fixup = (unsigned short *) (reloc + 1);
    for (i = 0; i < nrelocs; i++, fixup++)
    {
      unsigned short type = *fixup >> 12;
      unsigned short pos = *fixup & 0xfff;

      if (type == IMAGE_REL_BASED_HIGHLOW)
        *(unsigned long *) (pagestart + pos) += offset;
      else if (type != IMAGE_REL_BASED_ABSOLUTE)
      {
        logmsg(mod->db, "unsupported relocation type %d in %s", type, mod->name);
        return -ENOEXEC;
      }
    }

    reloc = (struct image_base_relocation *) fixup;
  }

  mod->flags |= MODULE_RELOCATED;
  return 0;
}

static int protect_module(struct module *mod)
{
  struct image_header *imghdr = get_image_header(mod->hmod);
  int i;

  if (!mod->db->protect_region) return 0;

  // Set page protect for each section
  for (i = 0; i < imghdr->header.number_of_sections; i++)
  {
    int protect;
    unsigned long scn = imghdr->sections[i].characteristics & (IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE);

    if (scn == (IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ)) 
      protect = PAGE_EXECUTE_READ;
    else if (scn == (IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE)) 
      protect = PAGE_READWRITE;
    else if (scn == IMAGE_SCN_MEM_READ)
      protect = PAGE_READONLY;
    else
      protect = PAGE_EXECUTE_READWRITE;

    mod->db->protect_region(RVA(mod->hmod, imghdr->sections[i].virtual_address), imghdr->sections[i].size_of_raw_data, protect);
  }

  mod->flags |= MODULE_PROTECTED;
  return 0;
}

static void update_refcount(struct module *mod)
{
  struct image_import_descriptor *imp;

  // If imports already ref counted just skip
  if (mod->flags & MODULE_IMPORTS_REFED) return;

  // Find import directory in image
  imp = (struct image_import_descriptor *) get_image_directory(mod->hmod, IMAGE_DIRECTORY_ENTRY_IMPORT);
  if (!imp) return;

  // Get or load each dependent module
  while (imp->characteristics != 0)
  {
    char *name = RVA(mod->hmod, imp->name);
    struct module *depmod = get_module(mod->db, name);

    if (depmod != NULL) depmod->refcnt++;
    
    imp++;
  }

  mod->flags |= MODULE_IMPORTS_REFED;
}

static int remove_module(struct module *mod)
{
  struct image_header *imghdr;
  struct image_import_descriptor *imp;

  imghdr = get_image_header(mod->hmod);

  if (mod->flags & MODULE_INITIALIZED)
  {
    // Notify DLL
    if (imghdr->header.characteristics & IMAGE_FILE_DLL)
    {
      ((int (__stdcall *)(hmodule_t, int, void *)) get_entrypoint(mod->hmod))(mod->hmod, DLL_PROCESS_DETACH, NULL);
    }
    mod->flags &= ~MODULE_INITIALIZED;
  }

  if (mod->flags & MODULE_IMPORTS_REFED)
  {
    // Decrement reference count on all dependent modules 
    imp = (struct image_import_descriptor *) get_image_directory(mod->hmod, IMAGE_DIRECTORY_ENTRY_IMPORT);
    if (imp)
    {
      while (imp->characteristics != 0)
      {
        char *name = RVA(mod->hmod, imp->name);
        struct module *depmod = get_module(mod->db, name);

        if (depmod && --depmod->refcnt == 0) remove_module(depmod); 
        imp++;
      }
    }

    mod->flags &= ~MODULE_IMPORTS_REFED;
  }

  // Release memory for module
  if (mod->flags & MODULE_LOADED)
  {
    mod->db->unload_image(mod->hmod, imghdr->optional.size_of_image);
    mod->flags &= ~MODULE_LOADED;
  }

  // Notify
  if (mod->db->notify_unload) mod->db->notify_unload(mod->hmod);

  // Remove module from module list
  remove(mod);
  free(mod->name);
  free(mod->path);
  free(mod);

  return 0;
}

static void free_unused_modules(struct moddb *db)
{
  struct module *mod;
  
  mod = db->modules;
  while (1)
  {
    if (mod->refcnt == 0)
    {
      remove_module(mod);
      mod = db->modules;
    }
    else
    {
      mod = mod->next;
      if (mod == db->modules) break;
    }
  }
}

void *get_proc_address(hmodule_t hmod, char *procname)
{
  return get_proc_by_name(hmod, -1, procname);
}

hmodule_t get_module_handle(struct moddb *db, char *name)
{
  struct module *mod;
  
  if (!name || strlen(name) > MAXPATH - 1) return NULL;
  mod = get_module(db, name);
  if (mod == NULL) return NULL;
  return mod->hmod;
}

int get_module_filename(struct moddb *db, hmodule_t hmod, char *buffer, int size)
{
  struct module *mod;
  
  mod = get_module_for_handle(db, hmod);
  if (mod == NULL) return -EINVAL;
  strncpy(buffer, mod->path, size);
  return strlen(mod->path);
}

void *get_entrypoint(hmodule_t hmod)
{
  return RVA(hmod, get_image_header(hmod)->optional.address_of_entry_point);
}

hmodule_t load_module(struct moddb *db, char *name, int flags)
{
  char buffer[MAXPATH];
  char *basename;
  char *imgbase;
  struct module *mod;
  struct module *modlist;
  struct module *m;
  int rc;

  if ((flags & MODLOAD_NOSHARE) == 0)
  {
    // Return existing handle if module already loaded
    mod = get_module(db, name);
    if (mod != NULL)
    {
      mod->refcnt++;
      return mod->hmod;
    }
  }

  // Get canonical name
  if (!name || strlen(name) > MAXPATH - 1) return NULL;
  basename = get_module_name(db, name, buffer);
  if (!basename) return NULL;

  // Load image into memory
  imgbase = db->load_image(buffer);
  if (imgbase == NULL) return NULL;

  // Create new module
  mod = (struct module *) malloc(sizeof(struct module));
  mod->hmod = imgbase;
  mod->db = db;
  mod->name = strdup(basename);
  mod->path = strdup(buffer);
  mod->refcnt = 0;
  mod->flags = MODULE_LOADED;
  insert_before(db->modules, mod);

  // Resolve module dependencies
  modlist = resolve_imports(mod);
  if (modlist == NULL)
  {
    free_unused_modules(db);
    return NULL;
  }

  // Relocate, bind imports and protect new modules
  m = modlist;
  while (1)
  {
    rc = relocate_module(m);
    if (rc < 0)
    {
      free_unused_modules(db);
      return NULL;
    }

    rc = bind_imports(m);
    if (rc < 0)
    {
      free_unused_modules(db);
      return NULL;
    }

    rc = protect_module(m);
    if (rc < 0)
    {
      free_unused_modules(db);
      return NULL;
    }

    if (m == mod) break;
    m = m->next;
  }

  // Initialize and notify new modules
  m = modlist;
  while (1)
  {
    if (get_image_header(m->hmod)->header.characteristics & IMAGE_FILE_DLL)
    {
      if ((flags & MODLOAD_NOINIT) == 0 || m != mod)
      {
        int ok;

        //logmsg(db, "initializing module %s", m->name);
        ok = ((int (__stdcall *)(hmodule_t, int, void *)) get_entrypoint(m->hmod))(m->hmod, DLL_PROCESS_ATTACH, NULL);
        //logmsg(db, "module %s initialized%s", m->name, ok ? "" : ", init failed");
        if (!ok)
        {
          free_unused_modules(db);
          return NULL;
        }

        m->flags |= MODULE_INITIALIZED;
      }
    }

    // Notify
    if (db->notify_load) db->notify_load(m->hmod, &m->name);

    if (m == mod) break;
    m = m->next;
  }

  // Update ref counts on depend module
  mod->refcnt++;
  m = modlist;
  while (1)
  {
    update_refcount(m);
    if (m == mod) break;
    m = m->next;
  }

  return mod->hmod;
}

int unload_module(struct moddb *db, hmodule_t hmod)
{
  struct module *mod;

  // Find module
  mod = get_module_for_handle(db, hmod);
  if (mod == NULL) return -EINVAL;

  // Decrement reference count, return if not zero
  if (--mod->refcnt > 0) return 0;

  // Remove module
  return remove_module(mod);
}

static struct image_resource_directory_entry *find_resource(char *resbase, struct image_resource_directory *dir, char *id)
{
  struct image_resource_directory_entry *direntry;
  struct image_resource_directory_string *entname;
  int i;

  direntry = (struct image_resource_directory_entry *) (dir + 1);
  if ((unsigned long) id < 0x10000)
  {
    // Lookup by ID, first skip named entries
    direntry += dir->number_of_named_entries;

    for (i = 0; i < dir->number_of_id_entries; i++)
    {
      if (direntry->id == (unsigned long) id) return direntry;
      direntry++;
    }
  }
  else
  {
    // Lookup by name
    for (i = 0; i < dir->number_of_named_entries; i++)
    {
      unsigned short *p1;
      unsigned char *p2;
      int left;
      unsigned short ch1;
      unsigned short ch2;

      entname = (struct image_resource_directory_string *) RVA(resbase, direntry->name_offset);
      p1 = (unsigned short *) entname->name_string;
      p2 = (unsigned char *) id;
      left = entname->length;
      while (left > 0 && *p2 != 0)
      {
        if (((ch1 = *p1++) >= 'a') && (ch1 <= 'z')) ch1 += 'A' - 'a';
        if (((ch2 = *p2++) >= 'a') && (ch2 <= 'z')) ch2 += 'A' - 'a';

        if (ch1 != ch2) break;

        left--;
      }
      
      if (left == 0 && *p2 == 0) return direntry;
      direntry++;
    }
  }

  return NULL;
}

int get_resource_data(hmodule_t hmod, char *id1, char *id2, char *id3, void **data)
{
  char *resbase;
  struct image_resource_directory *dir;
  struct image_resource_directory_entry *direntry;
  struct image_resource_data_entry *dataentry;

  // Find resource root directory
  resbase = get_image_directory(hmod, IMAGE_DIRECTORY_ENTRY_RESOURCE);
  if (!resbase) return -ENOENT;
  dir = (struct image_resource_directory *) resbase;

  // Find first level entry
  direntry = find_resource(resbase, dir, id1);
  if (!direntry) return -ENOENT;
  if (!direntry->data_is_directory) return -EINVAL;
  dir = (struct image_resource_directory *) RVA(resbase, direntry->offset_to_directory);

  // Find second level entry
  direntry = find_resource(resbase, dir, id2);
  if (!direntry) return -ENOENT;
  if (!direntry->data_is_directory) return -EINVAL;
  dir = (struct image_resource_directory *) RVA(resbase, direntry->offset_to_directory);

  // Find third level entry
  direntry = find_resource(resbase, dir, id3);
  if (!direntry) return -ENOENT;
  if (direntry->data_is_directory) return -EINVAL;

  dataentry = (struct image_resource_data_entry *) RVA(resbase, direntry->offset_to_data);
  *data = RVA(hmod, dataentry->offset_to_data);
  return dataentry->size;
}

int init_module_database(struct moddb *db, char *name, hmodule_t hmod, char *libpath, struct section *aliassect, int flags)
{
  char *basename;
  char *p;
  struct property *prop;

  // Set flags
  db->flags = flags;

  // Set library paths
  if (libpath)
  {
    int n;
    char *p;
    char *q;
    char *path;

    p = libpath;
    n = 1;
    while (*p)
    {
      if (*p == ';') n++;
      p++;
    }

    db->nmodpaths = n;
    db->modpaths = (char **) malloc(n * sizeof(char *));

    p = libpath;
    n = 0;
    while (*p)
    {
      q = p;
      while (*q && *q != ';') q++;

      db->modpaths[n++] = path = (char *) malloc(q - p + 1);
      memcpy(path, p, q - p);
      path[q - p] = 0;
      
      if (*q)
        p = q + 1;
      else
        p = q;
    }
  }
  else
  {
    db->modpaths = NULL;
    db->nmodpaths = 0;
  }

  // Setup module aliases
  db->aliases = NULL;
  if (aliassect)
  {
    prop = aliassect->properties;
    while (prop)
    {
      struct modalias *ma;

      ma = (struct modalias *) malloc(sizeof(struct modalias));
      if (!ma) return -ENOMEM;

      ma->name = strdup(prop->name);
      ma->alias = strdup(prop->value);
      ma->next = db->aliases;

      db->aliases = ma;

      prop = prop->next;
    }
  }

  // Setup module database with initial module
  basename = name;
  p = name;
  while (*p)
  {
    if (*p == PS1 || *p == PS2) basename = p + 1;
    p++;
  }

  db->modules = (struct module *) malloc(sizeof(struct module));
  if (!db->modules) return -ENOMEM;

  db->modules->hmod = hmod;
  db->modules->db = db;
  db->modules->name = strdup(basename);
  db->modules->path = strdup(name);
  db->modules->next = db->modules;
  db->modules->prev = db->modules;
  db->modules->refcnt = 1;
  db->modules->flags =  MODULE_LOADED | MODULE_IMPORTS_REFED | MODULE_RESOLVED | MODULE_RELOCATED | MODULE_BOUND | MODULE_PROTECTED | MODULE_INITIALIZED;

  // Protect initial module
  if (db->protect_region) protect_module(db->modules);

  return 0;
}
