//
// start.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Kernel initialization
//

#include <os/krnl.h>
#include <os/version.h>

#define KERNEL_CONFIG  "/etc/krnl.ini"

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

  shell();

  kprintf("syncing filesystems...\n");
  unmount_all();
  kprintf("system stopped\n");
  sleep(100);
  cli();
  halt();
}

void stop(int restart)
{
  kprintf("syncing filesystems...\n");
  unmount_all();

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
  
  // Initialize timers, scheduler, and object manager
  init_timers();
  init_sched();
  init_objects();

  // Enable interrupts and calibrate delay
  __asm { sti };
  calibrate_delay();

  // Start main task and dispatch to idle task
  mainthread = create_kernel_thread(main, 0, PRIORITY_NORMAL, "init");
  
  idle_task();
}

void init_mount()
{
  struct section *sect;
  struct property *prop;
  char devname[256];
  char *type;
  char *opts;
  int rc;

  sect = find_section(krnlcfg, "mount");
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

    //kprintf("mount %s on %s type %s opts %s\n", devname, prop->name, type, opts);

    rc = mount(type, prop->name, devname, opts);
    if (rc < 0) kprintf("%s: error %d mounting device %s\n", prop->name, rc, devname);

    prop = prop->next;
  }
}

void init_net()
{
  struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;

  stats_init();
  ether_init();
  pbuf_init();
  arp_init();
  ip_init();
  udp_init();
  dhcp_init();
  tcp_init();
  socket_init();

  IP4_ADDR(&ipaddr, 0, 0, 0, 0);
  IP4_ADDR(&netmask, 255, 255, 255, 255);
  IP4_ADDR(&gw, 0, 0, 0, 0);

  nic = ether_netif_add("eth0", "nic0", &ipaddr, &netmask, &gw);
  if (nic == NULL) return;

  netif_set_default(nic);
  peb->ipaddr.s_addr = nic->ipaddr.addr;
  if (peb->primary_dns.s_addr != INADDR_ANY)
  {
    kprintf("dns: primary %a", &peb->primary_dns);
    if (peb->secondary_dns.s_addr != INADDR_ANY) kprintf(" secondary %a", &peb->secondary_dns);
    if (*peb->default_domain) kprintf(" domain %s", peb->default_domain);
    kprintf("\n");
  }
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

  // Allocate and initialize PEB
  peb = mmap((void *) PEB_ADDRESS, PAGESIZE, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
  if (!peb) panic("unable to allocate PEB");
  memset(peb, 0, PAGESIZE);
  peb->fast_syscalls_supported = (cpu.features & CPU_FEATURE_SEP) != 0;

  // Enumerate root host buses and units
  enum_host_bus();

  // Initialize boot device drivers
  init_hd();
  init_fd();

  // Initialize built-in file systems
  init_dfs();
  init_devfs();
  init_procfs();
 
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

  // Mount devices
  init_mount();

  // Allocate handles for stdin, stdout and stderr
  open("/dev/console", O_RDONLY, &stdin);
  open("/dev/console", O_WRONLY, &stdout);
  open("/dev/console", O_WRONLY, &stderr);
  if (halloc(&stdin->object) != 0) panic("unexpected stdin handle");
  if (halloc(&stdout->object) != 1) panic("unexpected stdout handle");
  if (halloc(&stderr->object) != 2) panic("unexpected stderr handle");

  // Load os.dll in user address space
  imgbase = load_image_file("/os/os.dll", 1);
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
