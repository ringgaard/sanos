//
// os.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Operating system API
//

#include <os.h>
#include <string.h>
#include <inifile.h>
#include <moddb.h>

#include <os/seg.h>
#include <os/tss.h>
#include <os/syspage.h>
#include <os/pe.h>

#include "heap.h"
#include "resolv.h"

struct critsect heap_lock;
struct critsect mod_lock;
struct section *config;
struct moddb usermods;
struct peb *peb;

unsigned long loglevel = LOG_DEBUG | LOG_APITRACE | LOG_AUX | LOG_MODULE;
int logfile = -1;

void init_sntpd();

void panic(const char *msg)
{
  write(2, msg, strlen(msg));
  write(2, "\n", 1);
  if (logfile > 0) close(logfile);
  exit(3);
}

void syslog(int priority, const char *fmt,...)
{
  va_list args;
  char buffer[1024];

  if (!(priority & LOG_SUBSYS_MASK)) priority |= LOG_AUX;

  if (((unsigned) priority & LOG_LEVEL_MASK) <= (loglevel & LOG_LEVEL_MASK) && (priority & loglevel & LOG_SUBSYS_MASK) != 0)
  {
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);
    write(1, buffer, strlen(buffer));
    if (logfile > 0) write(logfile, buffer, strlen(buffer));
  }
}

void *malloc(size_t size)
{
  void *p;

  //syslog(LOG_MODULE | LOG_DEBUG, "malloc %d bytes\n", size);

  enter(&heap_lock);
  p = heap_alloc(size);
  leave(&heap_lock);

  //syslog(LOG_MODULE | LOG_DEBUG, "malloced %d bytes at %p\n", size, p);

  return p;
}

void *realloc(void *mem, size_t size)
{
  void *p;

  enter(&heap_lock);
  p = heap_realloc(mem, size);
  leave(&heap_lock);
  return p;
}

void *calloc(size_t num, size_t size)
{
  void *p;

  enter(&heap_lock);
  p = heap_calloc(num, size);
  leave(&heap_lock);
  return p;
}

void free(void *p)
{
  enter(&heap_lock);
  heap_free(p);
  leave(&heap_lock);
}

char *canonicalize(const char *filename, char *buffer, int size)
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

static void *load_image(char *filename)
{
  handle_t f;
  char *buffer;
  char *imgbase;
  struct dos_header *doshdr;
  struct image_header *imghdr;
  int i;
  unsigned int bytes;

  // Allocate header buffer
  buffer = malloc(PAGESIZE);
  if (!buffer) return NULL;
  memset(buffer, 0, PAGESIZE);

  // Open file
  f = open(filename, O_RDONLY);
  if (f < 0) 
  {
    free(buffer);
    return NULL;
  }

  // Read headers
  if ((bytes = read(f, buffer, PAGESIZE)) < 0)
  {
    close(f);
    free(buffer);
    return NULL;
  }
  
  doshdr = (struct dos_header *) buffer;
  imghdr = (struct image_header *) (buffer + doshdr->e_lfanew);

  // Check PE file signature
  if (doshdr->e_lfanew > bytes || imghdr->signature != IMAGE_PE_SIGNATURE) 
  {
    close(f);
    free(buffer);
    return NULL;
  }

  // Check alignment
  //if (imghdr->optional.file_alignment != PAGESIZE || imghdr->optional.section_alignment != PAGESIZE) panic("image not page aligned");

  // Allocate memory for module
  imgbase = (char *) mmap(NULL, imghdr->optional.size_of_image, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
  if (imgbase == NULL)
  {
    close(f);
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
      if (lseek(f, imghdr->sections[i].pointer_to_raw_data, SEEK_SET) != imghdr->sections[i].pointer_to_raw_data)
      {
	munmap(imgbase, imghdr->optional.size_of_image, MEM_RELEASE);
	close(f);
        free(buffer);
	return NULL;
      }

      if (read(f, RVA(imgbase, imghdr->sections[i].virtual_address), imghdr->sections[i].size_of_raw_data) != (int) imghdr->sections[i].size_of_raw_data)
      {
	munmap(imgbase, imghdr->optional.size_of_image, MEM_RELEASE);
	close(f);
        free(buffer);
	return NULL;
      }
    }
  }

  //syslog(LOG_MODULE | LOG_DEBUG, "image %s loaded at %p (%d KB)\n", filename, imgbase, imghdr->optional.size_of_image / 1024);

  // Close file
  close(f);
  free(buffer);

  return imgbase;
}

static int unload_image(hmodule_t hmod, size_t size)
{
  return munmap(hmod, size, MEM_RELEASE);
}

static int protect_region(void *mem, size_t size, int protect)
{
  return mprotect(mem, size, protect);
}

static void logldr(char *msg)
{
  syslog(LOG_MODULE | LOG_DEBUG, "mod: %s\n", msg);
}

void *resolve(hmodule_t hmod, const char *procname)
{
  void *addr;

  enter(&mod_lock);
  addr = get_proc_address(hmod, (char *) procname);
  leave(&mod_lock);
  return addr;
}

hmodule_t getmodule(char *name)
{
  hmodule_t hmod;

  enter(&mod_lock);
  hmod = get_module_handle(&usermods, name);
  leave(&mod_lock);
  return hmod;
}

int getmodpath(hmodule_t hmod, char *buffer, int size)
{
  int rc;

  enter(&mod_lock);
  rc = get_module_filename(&usermods, hmod, buffer, size);
  leave(&mod_lock);
  return rc;
}

hmodule_t load(const char *name)
{
  hmodule_t hmod;

  enter(&mod_lock);
  hmod = load_module(&usermods, (char *) name);
  leave(&mod_lock);
  return hmod;
}

int unload(hmodule_t hmod)
{
  int rc;

  enter(&mod_lock);
  rc = unload_module(&usermods, hmod);
  leave(&mod_lock);
  return rc;
}

int exec(hmodule_t hmod, char *args)
{
  return ((int (__stdcall *)(hmodule_t, char *, int)) get_entrypoint(hmod))(hmod, args, 0);
}

void dbgbreak()
{
  __asm { int 3 };
}

void init_mount()
{
  struct section *sect;
  struct property *prop;
  char devname[256];
  char *type;
  char *opts;
  int rc;

  sect = find_section(config, "mount");
  if (!sect) return;

  prop = sect->properties;
  while (prop)
  {
    strcpy(devname, prop->value ? prop->value : "");
    type = strchr(devname, ',');
    if (type)
    {
      *type++ = 0;
      while (*type == ' ') type++;
      opts = strchr(type, ',');
      if (opts)
      {
	*opts++ = 0;
	while (*opts == ' ') opts++;
      }
      else
	opts = NULL;
    }
    else
    {
      type = "dfs";
      opts = NULL;
    }

    //syslog(LOG_DEBUG, "mount %s on %s type %s opts %s\n", devname, prop->name, type, opts);

    rc = mount(type, prop->name, devname, opts);
    if (rc < 0) syslog(LOG_ERR, "%s: error %d mounting %s %s\n", prop->name, rc, type, devname);

    prop = prop->next;
  }
}

int __stdcall start(hmodule_t hmod, void *reserved, void *reserved2)
{
  char *initpgm;
  char *initargs;
  hmodule_t hexecmod;
  int rc;
  char *logfn;

  // Set usermode segment selectors
  __asm
  {
    mov	ax, SEL_UDATA + SEL_RPL3
    mov	ds, ax
    mov	es, ax
  }

  // Setup pointer to process environment block (PEB)
  peb = (struct peb *) PEB_ADDRESS;

  // Initialize heap and module locks
  mkcs(&heap_lock);
  mkcs(&mod_lock);

  // Thread specific stdin, stdout and stderr
  gettib()->in = 0;
  gettib()->out = 1;
  gettib()->err = 2;

  // Load configuration file
  config = read_properties("/etc/os.ini");
  if (!config) panic("error reading /etc/os.ini");
  loglevel = get_numeric_property(config, "os", "loglevel", loglevel);
  logfn = get_property(config, "os", "logfile", NULL);
  if (logfn != NULL) 
  {
    logfile = open(logfn, O_CREAT);
    if (logfile > 0) lseek(logfile, 0, SEEK_END);
  }

  // Initialize resolver
  res_init();

  // Initialize NTP daemon
  init_sntpd();

  // Initialize user module database
  peb->usermods = &usermods;
  usermods.load_image = load_image;
  usermods.unload_image = unload_image;
  usermods.protect_region = protect_region;
  usermods.log = logldr;

  init_module_database(&usermods, "os.dll", hmod, get_property(config, "os", "libpath", "/os"), 0);

  // Mount devices
  init_mount();

  // Load and execute init program
  initpgm = get_property(config, "os", "initpgm", "/os/init.exe");
  initargs = get_property(config, "os", "initargs", "");

  //syslog(LOG_DEBUG, "exec %s(%s)\n", initpgm, initargs);
  hexecmod = load(initpgm);
  if (hexecmod == NULL) panic("unable to load executable");

  rc = exec(hexecmod, initargs);

  if (rc != 0) syslog(LOG_DEBUG, "Exitcode: %d\n", rc);
  if (logfile > 0) close(logfile);
  exit(rc);
}
