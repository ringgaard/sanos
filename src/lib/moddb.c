//
// moddb.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Module loader
//

#include <os.h>
#include <os/pe.h>
#include <string.h>
#include <inifile.h>

#include <moddb.h>

#ifdef KERNEL
#include <os/krnl.h>
#endif

static char *get_basename(const char *filename, char *buffer, int size)
{
  char *basename = (char *) filename;
  char *p = (char *) filename;

  while (*p)
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

static struct module *get_module_for_handle(struct moddb *db, hmodule_t hmod)
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
  int len;

  for (i = 0; i < db->nmodpaths; i++)
  {
    // Build path name
    len = strlen(db->modpaths[i]);
    p = path;
    memcpy(p, db->modpaths[i], len);
    p += len;

    *p++ = PS1;

    basename = p;
    dot = NULL;
    s = name;
    while (*s)
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

    if (!dot)
    {
      memcpy(p, ".dll", 4);
      p += 4;
    }

    *p = 0;

    // Return path if file exists
    if (stat(path, NULL) > 0) return basename;
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

  if (!dot)
  {
    memcpy(p, ".dll", 4);
    p += 4;
  }

  *p = 0;

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

  // Get or load each dependent module
  while (imp->characteristics != 0)
  {
    char *name = RVA(mod->hmod, imp->name);
    struct module *newmod = get_module(mod->db, name);

    //syslog(LOG_MODULE | LOG_DEBUG, "imports from %s%s\n", name, newmod != NULL ? " (loaded)" : "");

    if (newmod == NULL)
    {
      char *imgbase;

      name = get_module_name(mod->db, name, path);
      if (!name)
      {
	//syslog(LOG_MODULE | LOG_ERROR, "DLL %s not found\n", name);
        return NULL;
      }

      imgbase = mod->db->load_image(path);
      if (imgbase == NULL) 
      {
	//syslog(LOG_MODULE | LOG_ERROR, "DLL %s not found\n", path);
        return NULL;
      }

      newmod = (struct module *) malloc(sizeof(struct module));
      newmod->hmod = imgbase;
      newmod->db = mod->db;
      newmod->name = strdup(name);
      newmod->path = strdup(path);
      newmod->refcnt = 1;
      insert_before(mod, newmod);

      newmod = resolve_imports(newmod);
      if (newmod == NULL) return NULL;

      if (modlist == mod) modlist = newmod;
    }
    else
      newmod->refcnt++;

    imp++;
  }

  return modlist;
}

static void bind_imports(struct moddb *db, hmodule_t hmod)
{
  struct image_import_descriptor *imp;

  // Find import directory in image
  imp = (struct image_import_descriptor *) get_image_directory(hmod, IMAGE_DIRECTORY_ENTRY_IMPORT);
  if (!imp) return;
  
  // Update Import Address Table (IAT)
  while (imp->characteristics != 0)
  {
    unsigned long *thunks;
    struct image_import_by_name *ibn;
    char *name;
    struct module *expmod;

    name = RVA(hmod, imp->name);
    expmod = get_module(db, name);
    if (!expmod) panic("module no longer loaded");
    if (imp->forwarder_chain != 0) panic("import forwarder chains not supported");

    //syslog(LOG_MODULE | LOG_TRACE, "import from %s (%p)\n", name, imp->characteristics);

    thunks = (unsigned long *) RVA(hmod, imp->first_thunk);
    while (*thunks)
    {
      if (*thunks & IMAGE_ORDINAL_FLAG)
      {
	// Import by ordinal
	//syslog(LOG_MODULE | LOG_TRACE, "  import ordinal %d from %s\n", *thunks & ~IMAGE_ORDINAL_FLAG, name);

	*thunks = (unsigned long) get_proc_by_ordinal(expmod->hmod, *thunks & ~IMAGE_ORDINAL_FLAG);
	if (*thunks == 0) panic("unable to resolve imports");
      }
      else
      {
	// Import by name (and hint)
	ibn = (struct image_import_by_name *) RVA(hmod, *thunks);

	//syslog(LOG_MODULE | LOG_TRACE, "  import %s from %s\n", ibn->name, name);

	*thunks = (unsigned long) get_proc_by_name(expmod->hmod, ibn->hint, ibn->name);
	if (*thunks == 0) panic("unable to resolve imports");
      }

      thunks++;
    }

    imp++;
  }
}

static void relocate_module(hmodule_t hmod)
{
  int offset;
  char *pagestart;
  unsigned short *fixup;
  int i;
  struct image_base_relocation *reloc;
  int nrelocs;

  offset = (int) hmod - get_image_header(hmod)->optional.image_base;
  if (offset == 0) return;

  reloc = (struct image_base_relocation *) get_image_directory(hmod, IMAGE_DIRECTORY_ENTRY_BASERELOC);
  if (!reloc) return;
  if (get_image_header(hmod)->header.characteristics & IMAGE_FILE_RELOCS_STRIPPED) panic("relocation info missing");

  while (reloc->virtual_address != 0)
  {
    pagestart = RVA(hmod, reloc->virtual_address);
    nrelocs = (reloc->size_of_block - sizeof(struct image_base_relocation)) / 2;
    fixup = (unsigned short *) (reloc + 1);
    for (i = 0; i < nrelocs; i++, fixup++)
    {
      int type = *fixup >> 12;
      int pos = *fixup & 0xfff;

      if (type == IMAGE_REL_BASED_HIGHLOW)
	*(unsigned long *) (pagestart + pos) += offset;
      else if (type != IMAGE_REL_BASED_ABSOLUTE)
	panic("unsupported relocation type");
    }

    reloc = (struct image_base_relocation *) fixup;
  }
}

static void protect_module(struct moddb *db, hmodule_t hmod)
{
  struct image_header *imghdr = get_image_header(hmod);
  int i;

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

    db->protect_region(RVA(hmod, imghdr->sections[i].virtual_address), imghdr->sections[i].size_of_raw_data, protect);
  }
}

void *get_proc_address(hmodule_t hmod, char *procname)
{
  return get_proc_by_name(hmod, -1, procname);
}

hmodule_t get_module_handle(struct moddb *db, char *name)
{
  struct module *mod;
  
  mod = name ? get_module(db, name) : db->execmod;
  if (mod == NULL) return NULL;
  return mod->hmod;
}

int get_module_filename(struct moddb *db, hmodule_t hmod, char *buffer, int size)
{
  struct module *mod;
  
  mod = hmod ? get_module_for_handle(db, hmod) : db->execmod;
  if (mod == NULL) return -EINVAL;
  strncpy(buffer, mod->path, size);
  return strlen(mod->path);
}

void *get_entrypoint(hmodule_t hmod)
{
  return RVA(hmod, get_image_header(hmod)->optional.address_of_entry_point);
}

hmodule_t load_module(struct moddb *db, char *name)
{
  char buffer[MAXPATH];
  char *basename;
  char *imgbase;
  struct module *mod;
  struct module *modlist;
  struct module *m;

  //syslog(LOG_MODULE | LOG_DEBUG, "loading module %s\n", name);

  // Return existing handle if module already loaded
  mod = get_module(db, name);
  if (mod != NULL)
  {
    mod->refcnt++;
    return mod->hmod;
  }

  // Get canonical name
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
  mod->refcnt = 1;
  insert_before(db->modules, mod);

  // Resolve module dependencies
  modlist = resolve_imports(mod);

  // Relocate, bind imports and initialize DLL's
  m = modlist;
  while (1)
  {
    relocate_module(m->hmod);
    bind_imports(db, m->hmod);
    if (db->protect_region) protect_module(db, m->hmod);

    if (get_image_header(m->hmod)->header.characteristics & IMAGE_FILE_DLL)
    {
      int ok;
      
      ok = ((int (__stdcall *)(hmodule_t, int, void *)) get_entrypoint(m->hmod))(m->hmod, DLL_PROCESS_ATTACH, NULL);
      if (!ok) return NULL;
    }
    else if (db->execmod == NULL)
      db->execmod = m;

    if (m == mod) break;
    m = m->next;
  }

  return mod->hmod;
}

int unload_module(struct moddb *db, hmodule_t hmod)
{
  struct module *mod;
  struct image_header *imghdr;
  struct image_import_descriptor *imp;

  // Find module
  mod = hmod ? get_module_for_handle(db, hmod) : db->execmod;
  if (mod == NULL) return -EINVAL;

  // Decrement reference count, return if not zero
  if (--mod->refcnt > 0) return 0;

  // Notify DLL
  imghdr = get_image_header(hmod);
  if (imghdr->header.characteristics & IMAGE_FILE_DLL)
  {
    ((int (__stdcall *)(hmodule_t, int, void *)) get_entrypoint(mod->hmod))(mod->hmod, DLL_PROCESS_DETACH, NULL);
  }
  if (mod->refcnt > 0) return 0;

  // Decrement reference count on all dependent modules 
  imp = (struct image_import_descriptor *) get_image_directory(mod->hmod, IMAGE_DIRECTORY_ENTRY_IMPORT);
  if (imp)
  {
    while (imp->characteristics != 0)
    {
      char *name = RVA(mod->hmod, imp->name);
      struct module *depmod = get_module(db, name);

      if (depmod)
      {
	int rc = unload_module(db, depmod->hmod);
	if (rc < 0) return rc;
      }

      imp++;
    }
  }

  // Release memory for module
  db->unload_image(mod->hmod, imghdr->optional.size_of_image);

  // Remove module from module list
  remove(mod);
  free(mod->name);
  free(mod);

  return 0;
}

void init_module_database(struct moddb *db, char *name, hmodule_t hmod, char *libpath, int flags)
{
  char buffer[MAXPATH];

  db->flags = flags;

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

  if (!find_in_modpaths(db, name, buffer)) panic("initial module missing");
  db->modules = (struct module *) malloc(sizeof(struct module));
  db->modules->hmod = hmod;
  db->modules->db = db;
  db->modules->name = strdup(name);
  db->modules->path = strdup(buffer);
  db->modules->next = db->modules;
  db->modules->prev = db->modules;
  db->modules->refcnt = 1;

  if (db->protect_region) protect_module(db, db->modules->hmod);
}
