#include <windows.h>
#include <dbghelp.h>
#include <stdio.h>
#include <time.h>
#include <io.h>

#undef IMAGE_ORDINAL_FLAG

#include "getopt.h"
#include "rdp.h"

#include <pdir.h>
#include <tss.h>
#include <syspage.h>
#include <pe.h>

#define MAX_THREADS 1024
#define MAX_MODULES 1024

#define DIR_SYSINFO     0
#define DIR_EXCEPTION   1
#define DIR_THREADS     2
#define DIR_MODULES     3
#define DIR_MEMORY      4

#define DIR_ENTRIES     5

struct memblock
{
  RVA rva;
  unsigned long addr;
  unsigned long size;
  unsigned char *mem;
  struct memblock *next;
};

struct module
{
  hmodule_t hmod;
  char *name;
  RVA namerva;
  unsigned long imagesize;
  time_t timestamp;
};

struct thread
{
  tid_t tid;
  void *tib;
  void *tcb;
  CONTEXT context;
  RVA ctxtrva;
};

char *dumpfn = "sanos.dmp";
char *port = "\\\\.\\pipe\\com_1";
char *searchpath = "c:\\sanos\\dbg\\bin";

struct dbg_session *sess;
struct memblock *memory = NULL;
struct module modules[MAX_MODULES];
int nummodules = 0;
struct thread threads[MAX_THREADS];
int numthreads = 0;
FILE *mdfile;
RVA mdpos = 0;
MINIDUMP_HEADER header;
MINIDUMP_DIRECTORY dir[DIR_ENTRIES];

RVA writemd(void *buffer, int len)
{
  RVA rva = mdpos;
  fwrite(buffer, 1, len, mdfile);
  mdpos += len;
  return rva;
}

RVA writemdstr(char *str)
{
  static char NUL = 0;
  int len = strlen(str);
  RVA rva = mdpos;
  writemd(&len, 4);
  while (*str)
  {
    writemd(str++, 1);
    writemd(&NUL, 1);
  }
  writemd(&NUL, 1);
  writemd(&NUL, 1);

  return rva;
}

char *read_string(char *addr)
{
  char buffer[2 * PAGESIZE];
  char *ptr = buffer;
  int left = 0;

  while (1)
  {
    if (left == 0)
    {
      left = PAGESIZE - ((unsigned long) addr & (PAGESIZE - 1));
      dbg_read_memory(sess, addr, left, ptr);
      addr += left;
    }

    if (*ptr == 0) break;
    ptr++;
    left--;
  }

  return strdup(buffer);
}

RVA get_memory_rva(void *addr)
{
  unsigned long loc = (unsigned long) addr;
  struct memblock *mb = memory;
  while (mb)
  {
    if (mb->addr < loc && mb->addr + mb->size > loc) return mb->rva + (loc - mb->addr);
    mb = mb->next;
  }

  printf("warning: memory location %p not in memory image\n", addr);
  return 0;
}

void convert_context(struct context *ctxt, CONTEXT *ctx)
{
  memset(ctx, 0, sizeof(CONTEXT));
  ctx->ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_SEGMENTS;

  ctx->SegSs = ctxt->ess;
  ctx->Esp = ctxt->esp;
  ctx->SegCs = ctxt->ecs;
  ctx->Eip = ctxt->eip;
  ctx->EFlags = ctxt->eflags;
  ctx->Ebp = ctxt->ebp;

  ctx->Eax = ctxt->eax;
  ctx->Ebx = ctxt->ebx;
  ctx->Ecx = ctxt->ecx;
  ctx->Edx = ctxt->edx;
  ctx->Esi = ctxt->esi;
  ctx->Edi = ctxt->edi;

  ctx->SegDs = ctxt->ds;
  ctx->SegEs = ctxt->es;
  ctx->SegFs = 0;
  ctx->SegGs = 0;
}

unsigned long convert_trap_number(unsigned long traptype)
{
  switch (traptype)
  {
    case INTR_DIV:     return EXCEPTION_INT_DIVIDE_BY_ZERO;
    case INTR_DEBUG:   return EXCEPTION_SINGLE_STEP;
    case INTR_NMI:     return 0x80001002L;
    case INTR_BPT:     return EXCEPTION_BREAKPOINT;
    case INTR_OVFL:    return EXCEPTION_INT_OVERFLOW;
    case INTR_BOUND:   return EXCEPTION_ARRAY_BOUNDS_EXCEEDED;
    case INTR_INSTR:   return EXCEPTION_ILLEGAL_INSTRUCTION;
    case INTR_FPU:     return 0x80001007L;
    case INTR_DFAULT:  return 0x80001008L;
    case INTR_CPSOVER: return 0x80001009L;
    case INTR_INVTSS:  return 0x8000100AL;
    case INTR_SEG:     return 0x8000100BL;
    case INTR_STACK:   return EXCEPTION_STACK_OVERFLOW;
    case INTR_GENPRO:  return EXCEPTION_PRIV_INSTRUCTION;
    case INTR_PGFLT:   return EXCEPTION_ACCESS_VIOLATION;
    case INTR_RESVD1:  return 0x8000100FL;
    case INTR_NPX:     return 0x80001010L;
  }

  return 0x800010FFL;
}

char *locate_executable(char *name, char *buf)
{
  char path[256];
  char *p = searchpath;
  while (*p)
  {
    char *q = p;
    while (*q && *q != ';') q++;
    memcpy(path, p, q - p);
    path[q - p] = 0;
    strcat(path, "\\");
    strcat(path, name);

    if (_access(path, 0) == 0)
    {
      strcpy(buf, path);
      return buf;
    }

    p = q;
    if (*p == ';') p++;
  }

  return NULL;
}

void add_module(hmodule_t hmod, void **namepp)
{
  char *nameptr;
  char *name;
  struct dos_header doshdr;
  struct image_header imghdr;

  // Read name address location
  dbg_read_memory(sess, namepp, sizeof(nameptr), &nameptr);

  // Read module name
  name = read_string(nameptr);

  // Read header for PE image
  dbg_read_memory(sess, hmod, sizeof(struct dos_header), &doshdr);
  dbg_read_memory(sess, (char *) hmod + doshdr.e_lfanew, sizeof(struct image_header), &imghdr);

  printf("module %s hmod=%08X size=%dK\n", name, hmod, imghdr.optional.size_of_image / 1024);

  modules[nummodules].hmod = hmod;
  modules[nummodules].name = name;
  modules[nummodules].imagesize = imghdr.optional.size_of_image;
  modules[nummodules].timestamp = imghdr.header.timestamp;
  nummodules++;
}

void add_thread(tid_t tid, void *tib, void *startaddr, void *tcb)
{
  struct context ctxt;

  printf("thread %d tib=%p tcb=%p entry=%p\n", tid, tib, tcb, startaddr);

  dbg_get_context(sess, tid, &ctxt);
  threads[numthreads].tid = tid;
  threads[numthreads].tib = tib;
  threads[numthreads].tcb = tcb;
  convert_context(&ctxt, &threads[numthreads].context);
  numthreads++;
}

void write_memory()
{
  unsigned int i, j, k;
  struct memblock *last = NULL;
  struct memblock *mb;
  pte_t ptab[PTES_PER_PAGE];
  pte_t pdir[PTES_PER_PAGE];
  int total = 0;
  int numblocks = 0;
  MINIDUMP_MEMORY_LIST memlist;
  MINIDUMP_MEMORY_DESCRIPTOR memdesc;
  RVA rva;

  // Read page table directory
  dbg_read_memory(sess, (void *) PAGEDIR_ADDRESS, PAGESIZE, ptab);

  for (i = 0; i < PTES_PER_PAGE; i++)
  {
    if (ptab[i] & PT_PRESENT)
    {
      //printf("PDIR %d present\n", i);
      dbg_read_memory(sess, (void *) (PTBASE + i * PAGESIZE), PAGESIZE, pdir);
      j = 0;
      while (j < PTES_PER_PAGE)
      {
        int s;
        char *addr;
        char *mem;

        while (j < PTES_PER_PAGE && (pdir[j] & PT_PRESENT) == 0) j++;
        if (j == PTES_PER_PAGE) break;

        s = j;
        while (j < PTES_PER_PAGE && (pdir[j] & PT_PRESENT) != 0) j++;

        mb = (struct memblock *) malloc(sizeof(struct memblock));
        if (memory == NULL) memory = mb;
        if (last != NULL) last->next = mb;

        mb->addr = ((i * PTES_PER_PAGE) + s) * PAGESIZE;
        mb->size = (j - s) * PAGESIZE;
        mb->mem = malloc(mb->size);
        mb->next = NULL;
        numblocks++;
        last = mb;
        total += (j - s);

        printf("memory %08X (%dK)", mb->addr, mb->size / 1024);

        addr = (char *) mb->addr;
        mem = mb->mem;
        for (k = s; k < j; k++)
        {
          dbg_read_memory(sess, addr, PAGESIZE, mem);
          addr += PAGESIZE;
          mem += PAGESIZE;
          printf(".");
        }
        printf("\n");
      }
    }
  }
  printf("total %d blocks (%dK)\n", numblocks, (total * PAGESIZE) / 1024);

  // Write memory blocks to dump file
  mb = memory;
  while (mb)
  {
    mb->rva = writemd(mb->mem, mb->size);
    mb = mb->next;
  }

  memlist.NumberOfMemoryRanges = numblocks;
  rva = writemd(&memlist, sizeof(memlist));
  mb = memory;
  while (mb)
  {
    memdesc.StartOfMemoryRange = mb->addr;
    memdesc.Memory.DataSize = mb->size;
    memdesc.Memory.Rva = mb->rva;
    writemd(&memdesc, sizeof(memdesc));
    mb = mb->next;
  }

  dir[DIR_MEMORY].StreamType = MemoryListStream;
  dir[DIR_MEMORY].Location.DataSize = mdpos - rva;
  dir[DIR_MEMORY].Location.Rva = rva;
}

void write_sysinfo()
{
  RVA rva;
  MINIDUMP_SYSTEM_INFO sysinfo;

  memset(&sysinfo, 0, sizeof(sysinfo));
  sysinfo.ProcessorArchitecture = PROCESSOR_ARCHITECTURE_INTEL;
  sysinfo.ProcessorLevel = sess->conn.cpu.family;
  sysinfo.ProcessorRevision = (sess->conn.cpu.model << 16) | sess->conn.cpu.stepping;
  sysinfo.MajorVersion = 5;
  sysinfo.MinorVersion = 0;
  sysinfo.PlatformId = VER_PLATFORM_WIN32_NT;
  sysinfo.CSDVersionRva = writemdstr("Sanos");

  rva = writemd(&sysinfo, sizeof(sysinfo));
  dir[DIR_SYSINFO].StreamType = SystemInfoStream;
  dir[DIR_SYSINFO].Location.DataSize = mdpos - rva;
  dir[DIR_SYSINFO].Location.Rva = rva;
}

void write_exception()
{
  RVA rva;
  MINIDUMP_EXCEPTION_STREAM excpt;

  memset(&excpt, 0, sizeof(excpt));
  excpt.ThreadId = sess->conn.trap.tid;
  excpt.ExceptionRecord.ExceptionCode = convert_trap_number(sess->conn.trap.traptype);
  excpt.ExceptionRecord.ExceptionAddress = (ULONG64) sess->conn.trap.addr;
  excpt.ThreadContext.DataSize = sizeof(CONTEXT);
  excpt.ThreadContext.Rva = threads[0].ctxtrva;

  rva = writemd(&excpt, sizeof(excpt));
  dir[DIR_EXCEPTION].StreamType = ExceptionStream;
  dir[DIR_EXCEPTION].Location.DataSize = mdpos - rva;
  dir[DIR_EXCEPTION].Location.Rva = rva;
}

void write_threads()
{
  MINIDUMP_THREAD_LIST thrlist;
  MINIDUMP_THREAD thr;
  RVA rva;
  int i;

  for (i = 0; i < numthreads; i++)
  {
    threads[i].ctxtrva = writemd(&threads[i].context, sizeof(CONTEXT));
  }

  thrlist.NumberOfThreads = numthreads;
  rva = writemd(&thrlist, sizeof(thrlist));

  for (i = 0; i < numthreads; i++)
  {
    memset(&thr, 0, sizeof(thr));
    thr.ThreadId = threads[i].tid;
    thr.Teb = (ULONG64) threads[i].tib;
    thr.ThreadContext.DataSize = sizeof(CONTEXT);
    thr.ThreadContext.Rva = threads[i].ctxtrva;
    writemd(&thr, sizeof(thr));
  }

  dir[DIR_THREADS].StreamType = ThreadListStream;
  dir[DIR_THREADS].Location.DataSize = mdpos - rva;
  dir[DIR_THREADS].Location.Rva = rva;
}

void write_modules()
{
  MINIDUMP_MODULE_LIST modlist;
  MINIDUMP_MODULE mod;
  RVA rva;
  int i;
  char path[256];

  for (i = 0; i < nummodules; i++) 
  {
    if (locate_executable(modules[i].name, path))
      modules[i].namerva = writemdstr(path);
    else
      modules[i].namerva = writemdstr(modules[i].name);
  }

  modlist.NumberOfModules = nummodules;
  rva = writemd(&modlist, sizeof(modlist));

  for (i = 0; i < nummodules; i++)
  {
    memset(&mod, 0, sizeof(mod));
    mod.BaseOfImage = (ULONG64) modules[i].hmod;
    mod.ModuleNameRva = modules[i].namerva;
    mod.SizeOfImage = modules[i].imagesize;
    mod.TimeDateStamp = modules[i].timestamp;
    writemd(&mod, sizeof(mod));
  }

  dir[DIR_MODULES].StreamType = ModuleListStream;
  dir[DIR_MODULES].Location.DataSize = mdpos - rva;
  dir[DIR_MODULES].Location.Rva = rva;
}

int main(int argc, char *argv[])
{
  int help = 0;
  struct dbg_event *e;
  int c;

  // Print banner
  printf("Sanos Post Mortem Dump Utility\n");
  printf("Copyright (C) 2005 Michael Ringgaard. All rights reserved.\n\n");

  // Parse command line options
  while ((c = getopt(argc, argv, "o:p:s:?")) != EOF)
  {
    switch (c)
    {
      case 'o':
        dumpfn = optarg;
        break;

      case 'p':
        port = optarg;
        break;

      case 's':
        searchpath = optarg;
        break;

      case '?':
      default:
        help = 1;
    }
  }

  if (help)
  {
    printf("usage: pmdump [options]\n\n");
    printf("  -o <dumpfile> (default sanos.dmp)\n");
    printf("  -p <port>     (port for debugee, e.g. com1 or \\\\.\\pipe\\com_1)\n");
    printf("  -s <pathlist> (semicolon separated list of paths for locating executable files)\n");
    return 0;
  }

  // Connect to debugger
  sess = dbg_create_session(port);
  if (!sess)
  {
    printf("Unable to connect to debugger\n");
    return 0;
  }

  add_module(sess->conn.mod.hmod, sess->conn.mod.name);
  add_thread(sess->conn.thr.tid, sess->conn.thr.tib, sess->conn.thr.startaddr, sess->conn.thr.tcb);

  // Process event from connect
  e = dbg_next_event(sess);
  while (e)
  {
    if (e->type == DBGEVT_LOAD_MODULE)
      add_module(e->evt.load.hmod, e->evt.load.name);
    else if (e->type == DBGEVT_CREATE_THREAD)
      add_thread(e->evt.create.tid, e->evt.create.tib, e->evt.create.startaddr, e->evt.create.tcb);

    dbg_release_event(e);
    e = dbg_next_event(sess);
  }

  // Open minidump file
  mdfile = fopen(dumpfn, "wb");
  memset(&header, 0, sizeof(header));
  header.Signature = MINIDUMP_SIGNATURE;
  header.Version = MINIDUMP_VERSION;
  header.NumberOfStreams = DIR_ENTRIES;
  header.TimeDateStamp = time(NULL);

  writemd(&header, sizeof(header));

  // Write streams
  write_memory();
  write_sysinfo();
  write_threads();
  write_modules();
  write_exception();

  // Write directory
  header.StreamDirectoryRva = writemd(dir, sizeof(dir));

  // Fixup header and close minidump file
  fseek(mdfile, 0, SEEK_SET);
  writemd(&header, sizeof(header));
  fclose(mdfile);

  // Close debugger session and resume
  dbg_close_session(sess, 1);

  return 0;
}
