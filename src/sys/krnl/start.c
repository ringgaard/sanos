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
"ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\n"
"IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE\n"
"ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE\n"
"LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR\n"
"CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE\n"
"GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)\n"
"HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT\n"
"LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY\n"
"OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF\n"
"SUCH DAMAGE.\n";

struct thread *mainthread;
struct section *krnlcfg;

struct netif *nic;

struct file *stdin;
struct file *stdout;
struct file *stderr;

struct peb *peb;

void shell();

void main(void *arg);

void panic(char *msg)
{
  kprintf("panic: %s\n", msg);
  if (debugging) dbg_output(msg);
  dbg_break();
}

void exit(int status)
{
  if (status != 0) kprintf("exit code = %d\n", status);

  //shell();

  kprintf("syncing filesystems...\n");
  umount_all();
  kprintf("system stopped\n");
  sleep(1000);
  cli();
  halt();
}

void stop(int restart)
{
  kprintf("syncing filesystems...\n");
  umount_all();

  if (restart)
  {
    kprintf("rebooting\n");
    reboot();
  }
  else
  {
    kprintf("system stopped\n");
    sleep(100);
    cli();
    halt();
  }
}

static int load_kernel_config()
{
  struct file *f;
  int size;
  int rc;
  struct stat buffer;
  char *props;

  rc = open(KERNEL_CONFIG, 0, &f);
  if (rc < 0) return rc;

  fstat(f, &buffer);
  size = buffer.quad.size_low;

  props = (char *) kmalloc(size + 1);
  if (!props) 
  {
    close(f);
    return -ENOMEM;
  }

  rc = read(f, props, size);
  if (rc < 0)
  {
    free(props);
    close(f);
    return rc;
  }

  close(f);

  props[size] = 0;

  krnlcfg = parse_properties(props);
  free(props);

  return 0;
}

static int version_proc(struct proc_file *pf, void *arg)
{
#ifdef DEBUG
  pprintf(pf, OSNAME " version " OSVERSION " (Debug Build " __DATE__ " " __TIME__ ")\n");
#else
  pprintf(pf, OSNAME " version " OSVERSION " (Build " __DATE__ " " __TIME__ ")\n");
#endif
  pprintf(pf, COPYRIGHT "\n");

  return 0;
}

static int copyright_proc(struct proc_file *pf, void *arg)
{
  proc_write(pf, copyright, strlen(copyright));
  return 0;
}

void __stdcall start(void *hmod, int reserved1, int reserved2)
{
  // Initialize screen
  init_video();
  clear_screen();

  // Display banner
#ifdef DEBUG
  print_string(OSNAME " version " OSVERSION " (Debug Build " __DATE__ " " __TIME__ ")\n");
#else
  print_string(OSNAME " version " OSVERSION " (Build " __DATE__ " " __TIME__ ")\n");
#endif
  print_string(COPYRIGHT "\n\n");

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

  kprintf("start: loop\n");
  {
    unsigned int tmo = ticks + 1*HZ;
    while (time_before(ticks, tmo)) yield();
  }
  kprintf("start: sleep\n");
  sleep(1000);
  kprintf("start: sleep done \n");

  // Allocate and initialize PEB
  peb = mmap((void *) PEB_ADDRESS, PAGESIZE, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE, 'PEB');
  if (!peb) panic("unable to allocate PEB");
  memset(peb, 0, PAGESIZE);
  peb->fast_syscalls_supported = (cpu.features & CPU_FEATURE_SEP) != 0;

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
  init_smbfs();
 
  // Open boot device
  if (syspage->bootparams.bootdrv & 0x80)
  {
    if (syspage->bootparams.bootpart == -1)
      sprintf(bootdev, "hd%c", '0' + (syspage->bootparams.bootdrv & 0x7F));
    else
      sprintf(bootdev, "hd%c%c", '0' + (syspage->bootparams.bootdrv & 0x7F), 'a' + syspage->bootparams.bootpart);
  }
  else
    sprintf(bootdev, "fd%c", '0' + (syspage->bootparams.bootdrv & 0x7F));

  kprintf("mount: root on device %s\n", bootdev);

  // Mount root and device file systems
  rc = mount("dfs", "/", bootdev, "");
  if (rc < 0) panic("error mounting root filesystem");

  rc = mount("devfs", "/dev", NULL, NULL);
  if (rc < 0) panic("error mounting dev filesystem");

  rc = mount("procfs", "/proc", NULL, NULL);
  if (rc < 0) panic("error mounting proc filesystem");

  // Load kernel configuration
  rc = load_kernel_config();
  if (rc < 0) kprintf("%s: error %d loading kernel configuration\n", KERNEL_CONFIG, rc);

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
  open("/dev/console", O_RDONLY, &stdin);
  open("/dev/console", O_WRONLY, &stdout);
  open("/dev/console", O_WRONLY, &stderr);
  if (halloc(&stdin->object) != 0) panic("unexpected stdin handle");
  if (halloc(&stdout->object) != 1) panic("unexpected stdout handle");
  if (halloc(&stderr->object) != 2) panic("unexpected stderr handle");

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
