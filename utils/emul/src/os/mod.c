#include <os.h>
#include <pe.h>

#include "mod.h"

struct module *modules;
struct module *execmod;

char **modpaths = 0;
int nmodpaths = 0;

#define RVA(hmod, rva) (((char *) (hmod)) + (rva))

int stricmp(const char *s1, const char *s2);
char *strdup(char *s);
char *strncpy(char *dest, const char *source, size_t count);

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
  n->prev = n;
  m->next->prev = n;
  m->next = n;
}

static void remove(struct module *m)
{
  m->next->prev = m->prev;
  m->prev->next = m->next;
}

static struct image_header *get_image_header(hmodule_t hmod)
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

static struct module *get_module_for_handle(hmodule_t hmod)
{
  struct module *m;

  // Try to find module in module list
  m = modules;
  while (1)
  {
    if (hmod == m->hmod) return m;
    m = m->next;
    if (m == modules) break;
  }

  return NULL;
}

static char *find_in_modpaths(char *name, char *path)
{
  int i;
  char *p;
  char *s;
  char *basename;
  char *dot;
  int len;

  for (i = 0; i < nmodpaths; i++)
  {
    // Build path name
    len = strlen(modpaths[i]);
    p = path;
    memcpy(p, modpaths[i], len);
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
    if (get_file_stat(path, NULL) > 0) return basename;
  }

  return NULL;
}

static char *get_module_name(char *name, char *path)
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
    basename = get_file_path(name, path, MAXPATH);

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
    basename = find_in_modpaths(name, path);
  }

  syslog(LOG_MODULE | LOG_DEBUG, "get module name %s = %s (%s)\n", name, path, basename ? basename : "null");
  return basename;
}

static struct module *get_module(char *name)
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
  m = modules;
  while (1)
  {
    if (strcmp(buffer, m->name) == 0) return m;
    m = m->next;
    if (m == modules) break;
  }

  return NULL;
}

static void *load_image(char *filename)
{
  handle_t f;
  int size;
  char *buffer;
  char *imgbase;
  struct dos_header *doshdr;
  struct image_header *imghdr;
  struct stat statbuf;
  int i;

  // Allocate header buffer
  buffer = malloc(PAGESIZE);
  if (!buffer) return NULL;

  // Open file
  f = open_file(filename, O_RDONLY);
  if (f < 0) 
  {
    free(buffer);
    return NULL;
  }

  // Get file size
  get_handle_stat(f, &statbuf);
  size = statbuf.quad.size_low;

  // Read headers
  if (read_file(f, buffer, PAGESIZE) < 0)
  {
    close_handle(f);
    free(buffer);
    return NULL;
  }
  doshdr = (struct dos_header *) buffer;
  imghdr = (struct image_header *) (buffer + doshdr->e_lfanew);

  // Check PE file signature
  if (imghdr->signature != IMAGE_PE_SIGNATURE) panic("invalid PE signature");

  // Check alignment
  if (imghdr->optional.file_alignment != PAGESIZE || imghdr->optional.section_alignment != PAGESIZE) panic("image not page aligned");

  // Allocate memory for module
  imgbase = (char *) mmap(NULL, imghdr->optional.size_of_image, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
  if (imgbase == NULL)
  {
    close_handle(f);
    free(buffer);
    return NULL;
  }

  // copy header to image
  memcpy(imgbase, buffer, PAGESIZE);

  // Read sections
  for (i = 0; i < imghdr->header.number_of_sections; i++)
  {
    if (imghdr->sections[i].pointer_to_raw_data != 0)
    {
      set_file_pointer(f, imghdr->sections[i].pointer_to_raw_data, SEEK_SET);
      if (read_file(f, RVA(imgbase, imghdr->sections[i].virtual_address), imghdr->sections[i].size_of_raw_data) < 0)
      {
	munmap(imgbase, imghdr->optional.size_of_image, MEM_RELEASE);
	close_handle(f);
        free(buffer);
	return NULL;
      }
    }
  }

  // Close file
  close_handle(f);
  free(buffer);

  syslog(LOG_MODULE | LOG_DEBUG, "image %s loaded at %p (%d KB)\n", filename, imgbase, imghdr->optional.size_of_image / 1024);

  return imgbase;
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
    struct module *newmod = get_module(name);

    syslog(LOG_MODULE | LOG_DEBUG, "imports from %s%s\n", name, newmod != NULL ? " (loaded)" : "");

    if (newmod == NULL)
    {
      char *imgbase;

      name = get_module_name(name, path);
      if (!name)
      {
	syslog(LOG_MODULE | LOG_ERROR, "DLL %s not found\n", name);
        return NULL;
      }

      imgbase = load_image(path);
      if (imgbase == NULL) 
      {
	syslog(LOG_MODULE | LOG_ERROR, "DLL %s not found\n", path);
        return NULL;
      }

      newmod = (struct module *) malloc(sizeof(struct module));
      newmod->hmod = imgbase;
      newmod->name = strdup(name);
      newmod->path = strdup(path);
      newmod->refcnt = 1;
      insert_before(mod, newmod);

      newmod = resolve_imports(newmod);
      if (modlist == mod) modlist = newmod;
    }
    else
      newmod->refcnt++;

    imp++;
  }

  return modlist;
}

static void bind_imports(hmodule_t hmod)
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
    hmodule_t hexpmod;

    name = RVA(hmod, imp->name);
    hexpmod = get_module_handle(name);
    if (!hexpmod) panic("module no longer loaded");
    if (imp->forwarder_chain != 0) panic("import forwarder chains not supported");
    syslog(LOG_MODULE | LOG_TRACE, "import from %s (%p)\n", name, imp->characteristics);

    thunks = (unsigned long *) RVA(hmod, imp->first_thunk);
    while (*thunks)
    {
      if (*thunks & IMAGE_ORDINAL_FLAG)
      {
	// Import by ordinal
	syslog(LOG_MODULE | LOG_TRACE, "  import ordinal %d from %s\n", *thunks & ~IMAGE_ORDINAL_FLAG, name);

	*thunks = (unsigned long) get_proc_by_ordinal(hexpmod, *thunks & ~IMAGE_ORDINAL_FLAG);
	if (*thunks == 0) panic("unable to resolve imports");
      }
      else
      {
	// Import by name (and hint)
	ibn = (struct image_import_by_name *) RVA(hmod, *thunks);

	syslog(LOG_MODULE | LOG_TRACE, "  import %s from %s\n", ibn->name, name);

	*thunks = (unsigned long) get_proc_by_name(hexpmod, ibn->hint, ibn->name);
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

void *get_proc_address(hmodule_t hmod, char *procname)
{
  syslog(LOG_MODULE | LOG_DEBUG, "get_proc_address(%p,%s)\n", hmod, procname);
  return get_proc_by_name(hmod, -1, procname);
}

hmodule_t get_module_handle(char *name)
{
  struct module *mod;
  
  mod = name ? get_module(name) : execmod;
  if (mod == NULL) return NULL;
  return mod->hmod;
}

int get_module_filename(hmodule_t hmod, char *buffer, int size)
{
  struct module *mod;
  
  mod = hmod ? get_module_for_handle(hmod) : execmod;
  if (mod == NULL) return 0;
  strncpy(buffer, mod->path, size);
  return strlen(mod->path);
}

hmodule_t load_module(char *name)
{
  char buffer[MAXPATH];
  char *basename;
  char *imgbase;
  struct module *mod;
  struct module *modlist;
  struct module *m;

  syslog(LOG_MODULE | LOG_DEBUG, "loading module %s\n", name);

  // Return existing handle if module already loaded
  mod = get_module(name);
  if (mod != NULL)
  {
    mod->refcnt++;
    return mod->hmod;
  }

  // Get canonical name
  basename = get_module_name(name, buffer);
  if (!basename) return NULL;

  // Load image into memory
  imgbase = load_image(buffer);
  if (imgbase == NULL) return NULL;

  // Create new module
  mod = (struct module *) malloc(sizeof(struct module));
  mod->hmod = imgbase;
  mod->name = strdup(basename);
  mod->path = strdup(buffer);
  mod->refcnt = 1;
  insert_before(modules, mod);

  // Resolve module dependencies
  modlist = resolve_imports(mod);

#if 0
  syslog(LOG_MODULE | LOG_DEBUG, "Module list start\n");
  m = modules;
  while (1)
  {
    syslog(LOG_MODULE | LOG_DEBUG, "  Module %s (%s)\n", m->name, m == modlist ? "*" : "-");
    m = m->next;
    if (m == modules) break;
  }
  syslog(LOG_MODULE | LOG_DEBUG, "Module list end\n");
#endif

  // Relocate, bind imports and initialize DLL's
  m = modlist;
  while (1)
  {
    syslog(LOG_MODULE | LOG_DEBUG, "Relocate %s\n", m->name);
    relocate_module(m->hmod);

    syslog(LOG_MODULE | LOG_DEBUG, "Bind imports in %s\n", m->name);
    bind_imports(m->hmod);

    if (get_image_header(m->hmod)->header.characteristics & IMAGE_FILE_DLL)
    {
      int ok;
      
      syslog(LOG_MODULE | LOG_DEBUG, "Initialize %s\n", m->name);
      ok = ((int (__stdcall *)(hmodule_t, int, void *)) get_entrypoint(m->hmod))(m->hmod, DLL_PROCESS_ATTACH, NULL);
      syslog(LOG_MODULE | LOG_DEBUG, "Initialize %s returned %d\n", m->name, ok);
      if (!ok) return NULL;
    }
    else if (execmod == NULL)
      execmod = m;

    if (m == mod) break;
    m = m->next;
  }

  return mod->hmod;
}

void *get_entrypoint(hmodule_t hmod)
{
  return RVA(hmod, get_image_header(hmod)->optional.address_of_entry_point);
}

void init_modules(hmodule_t hmod, char *libpath)
{
  char buffer[MAXPATH];

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

    nmodpaths = n;
    modpaths = (char **) malloc(n * sizeof(char *));

    p = libpath;
    n = 0;
    while (*p)
    {
      q = p;
      while (*q && *q != ';') q++;

      modpaths[n++] = path = (char *) malloc(q - p + 1);
      memcpy(path, p, q - p);
      path[q - p] = 0;
      
      if (*q)
        p = q + 1;
      else
	p = q;
    }
  }

  if (!find_in_modpaths("os.dll", buffer)) panic("os.dll missing");
  modules = (struct module *) malloc(sizeof(struct module));
  modules->hmod = hmod;
  modules->name = strdup("os.dll");
  modules->path = strdup(buffer);
  modules->next = modules;
  modules->prev = modules;
  modules->refcnt = 1;
}
