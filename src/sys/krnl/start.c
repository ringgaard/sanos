//
// start.c
//
// Kernel initialization
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

#include <os/krnl.h>
#include <os/version.h>

#define KERNEL_CONFIG  "/etc/krnl.ini"

char *copyright = 
OSNAME " version " OSVERSION "\n"
COPYRIGHT "\n"
"\n"
"Redistribution and use in source and binary forms, with or without\n"
"modification, are permitted provided that the following conditions\n"
"are met:\n"
"\n"
"1. Redistributions of source code must retain the above copyright\n"
"   notice, this list of conditions and the following disclaimer.\n" 
"2. Redistributions in binary form must reproduce the above copyright\n"
"   notice, this list of conditions and the following disclaimer in the\n"
"   documentation and/or other materials provided with the distribution.\n"
"3. Neither the name of the project nor the names of its contributors\n"
"   may be used to endorse or promote products derived from this software\n"
"   without specific prior written permission.\n"
"\n"
"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND\n"
"ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\n"
"WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE\n"
"DISCLAIMED IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR\n"
"ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES\n"
"(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;\n"
"LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON\n"
"ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n"
"(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\n"
"SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n";

#define ONPANIC_HALT   0
#define ONPANIC_REBOOT 1
#define ONPANIC_DEBUG  2

struct thread *mainthread;
struct section *krnlcfg;
int onpanic = ONPANIC_HALT;
struct netif *nic;
struct peb *peb;
char krnlopts[KRNLOPTS_LEN];

void main(void *arg);

void stop(int restart)
{
  kprintf("kernel: suspending all user threads...\n");
  suspend_all_user_threads();

  kprintf("kernel: syncing filesystems...\n");
  umount_all();

  kprintf("kernel: clearing connections...\n");
  tcp_shutdown();
  
  sleep(200);

  if (restart)
  {
    kprintf("kernel: rebooting...\n");
    reboot();
  }
  else
  {
    kprintf("kernel: system stopped\n");
    cli();
    halt();
  }
}

void exit(int status)
{
  if (status != 0) kprintf("kernel: exit code = %d\n", status);
  stop(0);
}

void panic(char *msg)
{
  static int inpanic = 0;

  inpanic = 1;
  kprintf("panic: %s\n", msg);
  switch (onpanic)
  {
    case ONPANIC_HALT:
      if (inpanic)
      {
	cli();
	halt();
      }
      else
	stop(0);
      break;

    case ONPANIC_REBOOT:
      if (inpanic)
	reboot();
      else
	stop(1);
      break;

    case ONPANIC_DEBUG:
      if (debugging) dbg_output(msg);
      dbg_break();
      break;
  }
}

// Date string Mmm dd YYYY
//             01234567890

static int daynumber(char *datestr)
{
  struct tm tm;

  memset(&tm, 0, sizeof(struct tm));

  switch (datestr[0])
  {
    case 'J': tm.tm_mon = (datestr[1] == 'a') ? 0 : ((datestr[2] == 'n') ? 5 : 6); break;
    case 'F': tm.tm_mon = 1; break;
    case 'M': tm.tm_mon = (datestr[2] == 'r') ? 2 : 4; break;
    case 'A': tm.tm_mon = (datestr[1] == 'p') ? 3 : 7; break;
    case 'S': tm.tm_mon = 8; break;
    case 'O': tm.tm_mon = 9; break;
    case 'N': tm.tm_mon = 10; break;
    case 'D': tm.tm_mon = 11; break;
  }

  tm.tm_mday = datestr[5] - '0';
  if (datestr[4] != ' ') tm.tm_mday += (datestr[4] - '0') * 10;

  tm.tm_year += (datestr[7] - '0') * 1000;
  tm.tm_year += (datestr[8] - '0') * 100;
  tm.tm_year += (datestr[9] - '0') * 10;
  tm.tm_year += (datestr[10] - '0') - 1900;

  return mktime(&tm) / (24 * 60 * 60);
}

int buildno()
{
  return daynumber(__DATE__) - daynumber(RELEASE_DATE) + 1;
}

static int load_kernel_config()
{
  struct file *f;
  int size;
  int rc;
  struct stat64 buffer;
  char *props;

  rc = open(KERNEL_CONFIG, O_RDONLY | O_BINARY, 0, &f);
  if (rc < 0) return rc;

  fstat(f, &buffer);
  size = (int) buffer.st_size;

  props = (char *) kmalloc(size + 1);
  if (!props) 
  {
    close(f);
    destroy(f);
    return -ENOMEM;
  }

  rc = read(f, props, size);
  if (rc < 0)
  {
    free(props);
    close(f);
    destroy(f);
    return rc;
  }

  close(f);
  destroy(f);

  props[size] = 0;

  krnlcfg = parse_properties(props);
  free(props);

  return 0;
}

static int version_proc(struct proc_file *pf, void *arg)
{
#ifdef DEBUG
  pprintf(pf, "%s version %s Debug Build %d (%s %s)\n", OSNAME, OSVERSION, buildno(), __DATE__, __TIME__);
#else
  pprintf(pf, "%s version %s Build %d (%s %s)\n", OSNAME, OSVERSION, buildno(), __DATE__, __TIME__);
#endif
  pprintf(pf, "%s\n", COPYRIGHT);

  return 0;
}

static int copyright_proc(struct proc_file *pf, void *arg)
{
  proc_write(pf, copyright, strlen(copyright));
  return 0;
}

void __stdcall start(void *hmod, char *opts, int reserved2)
{
  // Copy kernel options
  strcpy(krnlopts, opts);
  if (get_option(opts, "silent", NULL, 0, NULL) != NULL) 
  {
    extern int kprint_enabled; // from cons.c
    kprint_enabled = 0;
  }

  // Initialize screen
  init_video();
  //clear_screen();

  // Display banner
#ifdef DEBUG
  kprintf("%s version %s Debug Build %d (%s %s)\n", OSNAME, OSVERSION, buildno(), __DATE__, __TIME__);
#else
  kprintf("%s version %s Build %d (%s %s)\n", OSNAME, OSVERSION, buildno(), __DATE__, __TIME__);
#endif
  kprintf("%s\n", COPYRIGHT);
  if (*krnlopts) kprintf("options: %s\n", krnlopts);

  // Initialize CPU
  init_cpu();

  // Initialize page frame database
  init_pfdb();

  // Initialize page directory
  init_pdir();

  // Initialize kernel heap
  init_kmem();

  // Initialize kernel allocator
  init_malloc();

  // Initialize virtual memory manager
  init_vmm();

  // Flush tlb
  flushtlb();

  // Register memory management procs
  register_proc_inode("memmap", memmap_proc, NULL);
  register_proc_inode("memusage", memusage_proc, NULL);
  register_proc_inode("memstat", memstat_proc, NULL);
  register_proc_inode("physmem", physmem_proc, NULL);
  register_proc_inode("pdir", pdir_proc, NULL);
  register_proc_inode("kmem", kmem_proc, NULL);
  register_proc_inode("kmodmem", kmodmem_proc, NULL);
  register_proc_inode("kheap", kheapstat_proc, NULL);
  register_proc_inode("vmem", vmem_proc, NULL);

  // Initialize interrupts, floating-point support, and real-time clock
  init_pic();
  init_trap();
  init_fpu();
  init_pit();
  
  // Initialize timers, scheduler, and handle manager
  init_timers();
  init_sched();
  init_handles();
  init_syscall();

  //if (syspage->bootparams.memmap.count != 0)
  //{
  //  struct memmap *memmap = &syspage->bootparams.memmap;
  //  int n;

  //  for (n = 0; n < memmap->count; n++)
  //  {
  //    unsigned long addr = (unsigned long) memmap->entry[n].addr;
  //    unsigned long size = (unsigned long) memmap->entry[n].size;
  //    unsigned long type = memmap->entry[n].type;
  //    kprintf("%p %p %d\n", addr, size, type);
  //  }
  //}

  // Enable interrupts and calibrate delay
  __asm { sti };
  calibrate_delay();

  // Start main task and dispatch to idle task
  mainthread = create_kernel_thread(main, 0, PRIORITY_NORMAL, "init");
  
  idle_task();
}

void init_net()
{
  stats_init();
  netif_init();
  ether_init();
  pbuf_init();
  arp_init();
  ip_init();
  udp_init();
  dhcp_init();
  tcp_init();
  socket_init();
  loopif_init();

  register_ether_netifs();
}

void main(void *arg)
{
  unsigned long *stacktop;
  struct thread *t;

  void *imgbase;
  void *entrypoint;
  unsigned long stack_reserve;
  unsigned long stack_commit;
  struct image_header *imghdr;

  char bootdev[8];
  int rc;
  char *str;
  struct file *cons;

  // Allocate and initialize PEB
  peb = mmap((void *) PEB_ADDRESS, PAGESIZE, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE, 'PEB');
  if (!peb) panic("unable to allocate PEB");
  memset(peb, 0, PAGESIZE);
  peb->fast_syscalls_supported = (cpu.features & CPU_FEATURE_SEP) != 0;

  // Initialize APM BIOS
  //init_apm();

  // Enumerate root host buses and units
  enum_host_bus();

  // Initialize boot device drivers
  init_hd();
  init_fd();

  // Initialize built-in file systems
  init_vfs();
  init_dfs();
  init_devfs();
  init_procfs();
  init_pipefs();
  init_smbfs();
  init_cdfs();
 
  // Open boot device
  if ((syspage->ldrparams.bootdrv & 0xF0) == 0xF0)
  {
    create_initrd();
    strcpy(bootdev, "initrd");
  }
  else if (syspage->ldrparams.bootdrv & 0x80)
  {
    if (syspage->ldrparams.bootpart == -1)
      sprintf(bootdev, "hd%c", '0' + (syspage->ldrparams.bootdrv & 0x7F));
    else
      sprintf(bootdev, "hd%c%c", '0' + (syspage->ldrparams.bootdrv & 0x7F), 'a' + syspage->ldrparams.bootpart);
  }
  else
    sprintf(bootdev, "fd%c", '0' + (syspage->ldrparams.bootdrv & 0x7F));

  kprintf("mount: root on device %s\n", bootdev);

  // Mount file systems
  rc = mount("dfs", "/", bootdev, "", NULL);
  if (rc < 0) panic("error mounting root filesystem");

  rc = mount("devfs", "/dev", NULL, NULL, NULL);
  if (rc < 0) panic("error mounting dev filesystem");

  rc = mount("procfs", "/proc", NULL, NULL, NULL);
  if (rc < 0) panic("error mounting proc filesystem");

  // Load kernel configuration
  rc = load_kernel_config();
  if (rc < 0) kprintf("%s: error %d loading kernel configuration\n", KERNEL_CONFIG, rc);
  str = get_property(krnlcfg, "krnl", "onpanic", "halt");
  if (strcmp(str, "halt") == 0)
    onpanic = ONPANIC_HALT;
  else if (strcmp(str, "reboot") == 0)
    onpanic = ONPANIC_REBOOT;
  else if (strcmp(str, "debug") == 0)
    onpanic = ONPANIC_DEBUG;

  // Initialize module loader
  init_kernel_modules();

  // Install device drivers
  install_drivers();

  // Initialize network
  init_net();

  // Install /proc/version and /proc/copyright handler
  register_proc_inode("version", version_proc, NULL);
  register_proc_inode("copyright", copyright_proc, NULL);

  // Allocate handles for stdin, stdout and stderr
  open("/dev/console", O_RDWR, S_IREAD | S_IWRITE, &cons);
  if (halloc(&cons->iob.object) != 0) panic("unexpected stdin handle");
  if (halloc(&cons->iob.object) != 1) panic("unexpected stdout handle");
  if (halloc(&cons->iob.object) != 2) panic("unexpected stderr handle");

  // Load os.dll in user address space
  imgbase = load_image_file("/bin/os.dll", 1);
  if (!imgbase) panic("unable to load os.dll");
  imghdr = get_image_header(imgbase);
  stack_reserve = imghdr->optional.size_of_stack_reserve;
  stack_commit = imghdr->optional.size_of_stack_commit;
  entrypoint = get_entrypoint(imgbase);

  // Initialize initial user thread
  t = self();
  if (init_user_thread(t, entrypoint) < 0) panic("unable to initialize initial user thread");
  if (allocate_user_stack(t, stack_reserve, stack_commit) < 0) panic("unable to allocate stack for initial user thread");
  t->hndl = halloc(&t->object);
  mark_thread_running();

  kprintf("mem: %dMB total, %dKB used, %dKB free, %dKB reserved\n", 
	  maxmem * PAGESIZE / M, 
	  (totalmem - freemem) * PAGESIZE / K, 
	  freemem * PAGESIZE / K, (maxmem - totalmem) * PAGESIZE / K);

  // Place arguments to start routine on stack
  stacktop = (unsigned long *) t->tib->stacktop;
  *(--stacktop) = 0;
  *(--stacktop) = 0;
  *(--stacktop) = (unsigned long) imgbase;
  *(--stacktop) = 0;

  // Jump into user mode
  __asm
  {
    mov eax, stacktop
    mov ebx, entrypoint

    push SEL_UDATA + SEL_RPL3
    push eax
    pushfd
    push SEL_UTEXT + SEL_RPL3
    push ebx
    iretd

    cli
    hlt
  }
}
