//
// os.c
//
// Operating system API
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
#include <string.h>
#include <inifile.h>
#include <moddb.h>
#include <stdlib.h>

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
  dbgbreak();
  //exit(3);
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
    if (logfile >= 0) write(logfile, buffer, strlen(buffer));
  }
}

void *malloc(size_t size)
{
  void *p;

  //syslog(LOG_MODULE | LOG_DEBUG, "malloc %d bytes\n", size);

  enter(&heap_lock);
  p = heap_alloc(size);
  leave(&heap_lock);

  if (size && !p) panic("malloc: out of memory");
  //syslog(LOG_MODULE | LOG_DEBUG, "malloced %d bytes at %p\n", size, p);

  return p;
}

void *realloc(void *mem, size_t size)
{
  void *p;

  enter(&heap_lock);
  p = heap_realloc(mem, size);
  leave(&heap_lock);

  if (size && !p) panic("realloc: out of memory");

  return p;
}

void *calloc(size_t num, size_t size)
{
  void *p;

  enter(&heap_lock);
  p = heap_calloc(num, size);
  leave(&heap_lock);

  if (size && !p) panic("realloc: out of memory");

  return p;
}

void free(void *p)
{
  enter(&heap_lock);
  heap_free(p);
  leave(&heap_lock);
}

struct mallinfo mallinfo()
{
  struct mallinfo m;

  enter(&heap_lock);
  m = heap_mallinfo();
  leave(&heap_lock);

  return m;
}

int canonicalize(const char *filename, char *buffer, int size)
{
  char *p;
  char *end;
  int len;

  // Check for maximum filename length
  if (!filename) return -EINVAL;

  // Remove drive letter from filename (e.g. c:)
  if (filename[0] != 0 && filename[1] == ':') filename += 2;

  // Initialize buffer
  p = buffer;
  end = buffer + size;

  // Add current directory to filename if relative path
  if (*filename != PS1 && *filename != PS2)
  {
    // Do not add current directory if it is root directory
    len = strlen(peb->curdir);
    if (len > 1)
    {
      memcpy(p, peb->curdir, len);
      p += len;
    }
  }

  while (*filename)
  {
    // Parse path separator
    if (*filename == PS1 || *filename == PS2) filename++;
    if (p == end) return -ENAMETOOLONG;
    *p++ = PS1;

    // Parse next name part in path
    len = 0;
    while (*filename && *filename != PS1 && *filename != PS2)
    {
      // We do not allow control characters in filenames
      if (*filename > 0 && *filename < ' ') return -EINVAL;
      if (p == end) return -ENAMETOOLONG;
      *p++ = *filename++;
      len++;
    }

    // Handle empty name parts and '.' and '..'
    if (len == 0)
      p--;
    if (len == 1 && filename[-1] == '.')
      p -= 2;
    else if (len == 2 && filename[-1] == '.' && filename[-2] == '.')
    {
      p -= 4;
      if (p < buffer) return -EINVAL;
      while (*p != PS1) p--;
    }
  }

  // Convert empty filename to /
  if (p == buffer) *p++ = PS1;

  // Terminate string
  if (p == end) return -ENAMETOOLONG;
  *p = 0;

  return p - buffer;
}

char *getcwd(char *buf, size_t size)
{
  size_t len;

  len = strlen(peb->curdir);

  if (buf)
  {
    if (len >= size)
    {
      errno = -ERANGE;
      return NULL;
    }
  }
  else
  {
    if (size == 0)
      buf = malloc(len + 1);
    else
    {
      if (len >= size)
      {
	errno = -ERANGE;
	return NULL;
      }

      buf = malloc(size);
    }

    if (!buf) 
    {
      errno = -ENOMEM;
      return NULL;
    }
  }

  memcpy(buf, peb->curdir, len + 1);
  return buf;
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
  imgbase = (char *) mmap(NULL, imghdr->optional.size_of_image, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE, 'UMOD');
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

hmodule_t getmodule(const char *name)
{
  hmodule_t hmod;

  enter(&mod_lock);
  hmod = get_module_handle(&usermods, (char *) name);
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
  hmod = load_module(&usermods, (char *) name, 0);
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
  struct module *prev_execmod;
  char *prev_args;
  int rc;

  prev_execmod = usermods.execmod;
  prev_args = gettib()->args;

  usermods.execmod = get_module_for_handle(&usermods, hmod);
  gettib()->args = args;

  rc = ((int (*)(hmodule_t, char *, int)) get_entrypoint(hmod))(hmod, args, 0);
  
  usermods.execmod = prev_execmod;
  gettib()->args = prev_args;

  return rc;
}

void dbgbreak()
{
  __asm { int 3 };
}

void init_net()
{
  struct section *sect;
  struct property *prop;
  struct ifcfg ifcfg;
  struct sockaddr_in *sin;
  char str[256];
  int first;
  int rc;
  int sock;

  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) return;

  sect = find_section(config, "netif");
  if (!sect) return;

  first = 1;
  prop = sect->properties;
  while (prop)
  {
    memset(&ifcfg, 0, sizeof(ifcfg));

    strcpy(ifcfg.name, prop->name);

    if (get_option(prop->value, "ip", str, sizeof str, NULL))
    {
      sin = (struct sockaddr_in *) &ifcfg.addr;
      sin->sin_len = sizeof(struct sockaddr_in);
      sin->sin_family = AF_INET;
      sin->sin_addr.s_addr = inet_addr(str);
    }
    else
      ifcfg.flags |= IFCFG_DHCP;

    if (get_option(prop->value, "gw", str, sizeof str, NULL))
    {
      sin = (struct sockaddr_in *) &ifcfg.gw;
      sin->sin_len = sizeof(struct sockaddr_in);
      sin->sin_family = AF_INET;
      sin->sin_addr.s_addr = inet_addr(str);
    }

    if (get_option(prop->value, "mask", str, sizeof str, NULL))
    {
      sin = (struct sockaddr_in *) &ifcfg.netmask;
      sin->sin_len = sizeof(struct sockaddr_in);
      sin->sin_family = AF_INET;
      sin->sin_addr.s_addr = inet_addr(str);
    }

    if (get_option(prop->value, "broadcast", str, sizeof str, NULL))
    {
      sin = (struct sockaddr_in *) &ifcfg.broadcast;
      sin->sin_len = sizeof(struct sockaddr_in);
      sin->sin_family = AF_INET;
      sin->sin_addr.s_addr = inet_addr(str);
    }

    ifcfg.flags |= IFCFG_UP;
    if (first) ifcfg.flags |= IFCFG_DEFAULT;

    rc = ioctl(sock, SIOIFCFG, &ifcfg, sizeof(struct ifcfg));
    if (rc < 0)
      syslog(LOG_ERR, "%s: unable to configure net interface, %s\n", ifcfg.name, strerror(rc));
    else
    {
      unsigned long addr = ((struct sockaddr_in *) &ifcfg.addr)->sin_addr.s_addr;
      unsigned long gw = ((struct sockaddr_in *) &ifcfg.gw)->sin_addr.s_addr;
      unsigned long mask = ((struct sockaddr_in *) &ifcfg.netmask)->sin_addr.s_addr;
      unsigned long bcast = ((struct sockaddr_in *) &ifcfg.broadcast)->sin_addr.s_addr;

      syslog(LOG_INFO, "%s: addr %a mask %a gw %a bcast %a\n", ifcfg.name, &addr, &mask, &gw, &bcast);

      if (first) peb->ipaddr.s_addr = addr;
    }

    prop = prop->next;
    first = 0;
  }

  close(sock);
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

  // Initialize network interfaces
  init_net();

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

  init_module_database(&usermods, "os.dll", hmod, get_property(config, "os", "libpath", "/bin"), find_section(config, "modaliases"), 0);

  // Mount devices
  init_mount();

  // Initialize log
  loglevel = get_numeric_property(config, "os", "loglevel", loglevel);
  logfn = get_property(config, "os", "logfile", NULL);
  if (logfn != NULL) 
  {
    logfile = open(logfn, O_CREAT);
    if (logfile >= 0) lseek(logfile, 0, SEEK_END);
  }

  // Load and execute init program
  initpgm = get_property(config, "os", "initpgm", "/bin/init.exe");
  initargs = get_property(config, "os", "initargs", "");

  //syslog(LOG_DEBUG, "exec %s(%s)\n", initpgm, initargs);
  hexecmod = load(initpgm);
  if (hexecmod == NULL) panic("unable to load executable");

  rc = exec(hexecmod, initargs);

  if (rc != 0) syslog(LOG_DEBUG, "Exitcode: %d\n", rc);
  if (logfile > 0) close(logfile);
  exit(rc);
}
