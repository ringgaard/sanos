//
// test.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Test program
//

#include <os/krnl.h>

extern struct fs *mountlist;
struct bufpool *bufpools;

unsigned char buffer[4096];

void dump_pool_stat(struct bufpool *pool);

static void pause()
{
  char ch;

  dev_read(consdev, &ch, 1, 0);
}

static char *gets(char *buf)
{
  char *p = buf;
  char ch;

  while (1)
  {
    dev_read(consdev, &ch, 1, 0);
    if (ch == 8)
    {
      if (p > buf)
      {
	dev_write(consdev, &ch, 1, 0);
	p--;
      }

    }
    else
    {
      dev_write(consdev, &ch, 1, 0);
      if (ch == 10) break;
      *p++ = ch;
    }
  }

  *p = 0;
  return buf;
}

static void alloc_test()
{
  int i;
  char *buffer[20];

  kprintf("alloc\n");
  for (i = 1; i < 20; i++)
  {
    buffer[i] = (char *) kmalloc(i * 125);
  }

  kprintf("free\n");
  for (i = 1; i < 20; i++) 
  {
    kfree(buffer[i]);
  }

  dump_malloc();

  //rmap_dump(osvmap);
  kprintf("Memory %dMB total, %dKB used, %dKB free, %dKB reserved\n", 
	  maxmem * PAGESIZE / M, 
	  (totalmem - freemem) * PAGESIZE / K, 
	  freemem * PAGESIZE / K, (maxmem - totalmem) * PAGESIZE / K);
}

void dump_memory(unsigned long addr, unsigned char *p, int len)
{
  int i;
  char *end;
  int line;
  char dummy;

  end = p + len;
  line = 0;
  while (p < end)
  {
    kprintf("%08X ", addr);
    for (i = 0; i < 16; i++) kprintf("%02X ", p[i]);
    for (i = 0; i < 16; i++) kprintf("%c", p[i] < 0x20 ? '.' : p[i]);
    kprintf("\n");
    p += 16;
    addr += 16;
    if (++line == 24)
    {
      dev_read(consdev, &dummy, 1, 0);
      line = 0;
    }
  }
}

static void dump_disk(char *devname, blkno_t blkno)
{
  devno_t dev;
  int rc;

  dev = dev_open(devname);
  if (dev == NODEV)
  {
    kprintf("%s: unable to open device\n", devname);
    return;
  }

  if (blkno < 0 || blkno >= (unsigned int) dev_ioctl(dev, IOCTL_GETDEVSIZE, NULL, 0))
  {
    kprintf("%d: invalid block number\n", blkno);
    dev_close(dev);
    return;
  }

  kprintf("block %d:\n", blkno);
  rc = dev_read(dev, buffer, 512, blkno);
  if (rc < 0)
    kprintf("%s: read error %d\n", devname, rc);
  else
    dump_memory(0, buffer, 512);

  dev_close(dev);
}

static void mem_usage()
{
  dump_malloc();

  kprintf("Memory %dMB total, %dKB used, %dKB free, %dKB reserved\n", 
	  maxmem * PAGESIZE / M, 
	  (totalmem - freemem) * PAGESIZE / K, 
	  freemem * PAGESIZE / K, (maxmem - totalmem) * PAGESIZE / K);
}

static void list_file(char *filename)
{
  int count;
  struct file *file;

  if (open(filename, 0, &file) < 0)
  {
    kprintf("%s: file not found\n", filename);
    return;
  }

  while ((count = read(file, buffer, 4096)) > 0)
  {
    dev_write(consdev, buffer, count, 0);
  }

  close(file);
}

static void copy_file(char *srcfn, char *dstfn)
{
  char *data;
  int count;
  struct file *f1;
  struct file *f2;

  if (open(srcfn, 0, &f1) < 0)
  {
    kprintf("%s: file not found\n", srcfn);
    return;
  }

  if (open(dstfn, O_CREAT, &f2) < 0)
  {
    close(f1);
    kprintf("%s: unable to create file\n", dstfn);
    return;
  }

  data = kmalloc(64 * K);
  while ((count = read(f1, data, 64 * K)) > 0)
  {
    if (write(f2, data, count) != count)
    {
      kprintf("%s: error writing data\n", dstfn);
      break;
    }
  }
  kfree(data);

  if (count < 0) kprintf("%s: error reading data\n", srcfn);
  
  close(f1);
  close(f2);
}

static void list_dir(char *dirname)
{
  struct file *dir;
  struct dirent dirp;
  struct stat buf;
  struct tm tm;
  char path[MAXPATH];

  if (opendir(dirname, &dir) < 0)
  {
    kprintf("%s: directory not found\n", dirname);
    return;
  }

  while (readdir(dir, &dirp, 1) > 0)
  {
    strcpy(path, dirname);
    strcat(path, "/");
    strcat(path, dirp.name);

    if (stat(path, &buf) < 0) memset(&buf, 0, sizeof(struct stat));

    gmtime(&buf.ctime, &tm);

    kprintf("%8d %8d %02d/%02d/%04d %02d:%02d:%02d ", buf.ino, buf.quad.size_low, tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
    if (buf.mode & FS_DIRECTORY) 
      kprintf("[%s]", dirp.name);
    else
      kprintf("%s", dirp.name);

    kprintf("\n");
  }

  close(dir);
}

static void remove_file(char *filename)
{
  if (unlink(filename) < 0)
  {
    kprintf("%s: file not found\n", filename);
    return;
  }
}

static void move_file(char *oldname, char *newname)
{
  if (rename(oldname, newname) < 0)
  {
    kprintf("%s: unable to rename file to %s\n", oldname, newname);
    return;
  }
}

static void make_dir(char *filename)
{
  if (mkdir(filename) < 0)
  {
    kprintf("%s: cannot make directory\n", filename);
    return;
  }
}

static void remove_dir(char *filename)
{
  if (rmdir(filename) < 0)
  {
    kprintf("%s: cannot delete directory\n", filename);
    return;
  }
}

static void disk_usage(char *filename)
{
  struct fs *fs;
  struct filsys *filsys;

  fs = fslookup(filename, NULL);
  if (!fs)
  {
    kprintf("%s: unknown file system\n", filename);
  }

  filsys = (struct filsys *) fs->data;
  kprintf("block size %d\n", filsys->blocksize);
  kprintf("blocks %d\n", filsys->super->block_count);
  kprintf("inodes %d\n", filsys->super->inode_count);
  kprintf("free blocks %d\n", filsys->super->free_block_count);
  kprintf("free inodes %d\n", filsys->super->free_inode_count);

  kprintf("%dKB used, %dKB free, %dKB total\n", 
    filsys->blocksize * (filsys->super->block_count - filsys->super->free_block_count) / 1024, 
    filsys->blocksize * filsys->super->free_block_count / 1024, 
    filsys->blocksize * filsys->super->block_count / 1024);
}

static void show_date()
{
  time_t time;
  struct tm tm;

  time = get_time();
  gmtime(&time, &tm);
  kprintf("Time is %04d/%02d/%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

static void dump_mbr(char *devname)
{
  devno_t dev;
  struct master_boot_record *mbr;
  int i;

  if (sizeof(struct master_boot_record) != 512) panic("MBR is not 512 bytes");

  dev = dev_open(devname);
  if (dev == NODEV)
  {
    kprintf("%s: unable to open device\n", devname);
    return;
  }

  dev_read(dev, buffer, 512, 0);
  mbr = (struct master_boot_record *) buffer;

  if (mbr->signature != MBR_SIGNATURE)
  {
    kprintf("%s: illegal boot sector signature\n", devname);
  }
  else
  {
    for (i = 0; i < 4; i++)
    {
      kprintf("%d: active=%02X begin=%d/%d/%d end=%d/%d/%d system=%02X start=%d len=%d\n",
	i, mbr->parttab[i].bootid,
	(int) mbr->parttab[i].begcyl + (((int) mbr->parttab[i].begsect >> 6) << 8), mbr->parttab[i].begsect & 0x3F, mbr->parttab[i].beghead,
	(int) mbr->parttab[i].endcyl + (((int) mbr->parttab[i].endsect >> 6) << 8), mbr->parttab[i].endsect & 0x3F, mbr->parttab[i].endhead,
	mbr->parttab[i].systid, mbr->parttab[i].relsect, mbr->parttab[i].numsect);
    }
  }

  dev_close(dev);
}

static void mount_device(char *devname, char *path)
{
  devno_t dev;

  dev = dev_open(devname);
  if (dev == NODEV)
  {
    kprintf("%s: unable to open device\n", devname);
    return;
  }

  if (mount("dfs", path, dev, NULL) < 0)
  {
    kprintf("%s: unable to mount device to %s\n", devname, path);
    return;
  }
}

static void reboot_system()
{
  kprintf("Unmounting filesystems...\n");
  unmount_all();
  kprintf("Rebooting...\n");
  reboot();
}

static void test_read_file(char *filename)
{
  int count;
  struct file *file;
  char *data;
  int start;
  int time;
  int bytes;

  if (open(filename, 0, &file) < 0)
  {
    kprintf("%s: file not found\n", filename);
    return;
  }

  data = kmalloc(64 * K);

  bytes = 0;
  start = get_tick_count();
  while ((count = read(file, data, 64 * K)) > 0)
  {
    bytes += count;
  }
  time = get_tick_count() - start;
  kprintf("%s: read %dKB in %d ms, %dKB/s\n", filename, bytes / K, time, bytes / time);
  
  kfree(data);

  if (count < 0) kprintf("%s: error reading file\n", filename);

  close(file);
}

static void test_write_file(char *filename, int size)
{
  int count;
  struct file *file;
  char *data;
  int start;
  int time;
  int bytes;

  if (open(filename, O_CREAT, &file) < 0)
  {
    kprintf("%s: error creating file\n", filename);
    return;
  }

  data = kmalloc(64 * K);

  bytes = 0;
  start = get_tick_count();
  while (bytes < size)
  {
    if ((count = write(file, data, 64 * K)) <= 0)
    {
      kprintf("%s: error writing file\n", filename);
      break;
    }

    bytes += count;
  }
  time = get_tick_count() - start;
  kprintf("%s: wrote %dKB in %d ms, %dKB/s\n", filename, bytes / K, time, bytes / time);
  
  kfree(data);

  close(file);
}

static void test_alloc(int size)
{
  void *addr;

  kprintf("allocating %d KB\n", size / K);
  addr = mmap(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  kprintf("allocated at %p\n", addr);
}

#define FRAQ 2310

static void handle_list()
{
  static char *objtype[] = {"TASK", "EVNT", "TIMR", "MUTX", "SEMA", "FILE"};

  int h;
  int i;
  struct object *o;
  int lines = 0;
  int objcount[6];

  for (i = 0; i < 6; i++) objcount[i] = 0;

  kprintf("handle addr     s type count\n");
  kprintf("------ -------- - ---- -----\n");
  for (h = 0; h < htabsize; h++)
  {
    o = htab[h];

    if (o < (struct object *) OSBASE) continue;
    if (o == (struct object *) NOHANDLE) continue;
    
    kprintf("%6d %8X %d %4s %5d\n", h, o, o->signaled, objtype[o->type], o->handle_count);
    objcount[o->type] += FRAQ / o->handle_count;
    if (++lines % 24 == 0) pause();
  }

  kprintf("\n");
  for (i = 0; i < 6; i++) kprintf("%s:%d ", objtype[i], objcount[i] / FRAQ);
  kprintf("\n");
}

static void list_memmap(struct rmap *rmap, unsigned int startpos)
{
  struct rmap *r;
  struct rmap *rlim;
  unsigned int pos = startpos;
  unsigned int total = 0;

  rlim = &rmap[rmap->offset];
  for (r = &rmap[1]; r <= rlim; r++) 
  {
    unsigned int size = r->offset - pos;

    if (size > 0)
    {
      kprintf("[%08X..%08X] %6d KB\n", pos * PAGESIZE, r->offset * PAGESIZE - 1, size * PAGESIZE / K);
      total += size;
    }
    pos = r->offset + r->size;
  }
  kprintf("Total: %d KB\n", total * PAGESIZE / K);
}

static void vmem_list()
{
  list_memmap(vmap, BTOP(64 * K));
}

static void kmem_list()
{
  list_memmap(osvmap, BTOP(KHEAPBASE));
}


static void mmem_list()
{
  list_memmap(kmodmap, BTOP(OSBASE));
}

static void pmem_list()
{
  unsigned int n;
  int lines = 0;

  kprintf(".=free *=user page X=locked user page, #=kernel page -=reserved page\n");
  for (n = 0; n < maxmem; n++)
  {
    if (n % 64 == 0)
    {
      if (n > 0) kprintf("\n");
      if (++lines % 24 == 0) pause();
      kprintf("%08X ", PTOB(n));
    }

    switch (pfdb[n].type)
    {
      case PFT_FREE:
	kprintf(".");
	break;

      case PFT_USED:
	kprintf(pfdb[n].locks ? "X" : "*");
	break;

      case PFT_SYS:
	if (pfdb[n].size == 0 || pfdb[n].size >= PAGESHIFT)
	  kprintf("#");
	else
	  kprintf("%c", '0' + (PAGESHIFT - pfdb[n].size));
	break;

      case PFT_BAD:
	kprintf("-");
	break;

      default:
	kprintf("?");
    }
  }
  kprintf("\n");
}

static void dump_pdir()
{
  char *vaddr;
  pte_t pte;
  int lines = 0;

  int ma = 0;
  int us = 0;
  int su = 0;
  int ro = 0;
  int rw = 0;
  int ac = 0;
  int dt = 0;

  kprintf("virtaddr physaddr flags\n");
  kprintf("-------- -------- ----- \n");

  vaddr = NULL;
  while (1)
  {
    if ((pdir[PDEIDX(vaddr)] & PT_PRESENT) == 0)
      vaddr += PTES_PER_PAGE * PAGESIZE;
    else
    {
      pte = ptab[PTABIDX(vaddr)];
      if (pte & PT_PRESENT) 
      {
	ma++;
	
	if (pte & PT_WRITABLE)
	  rw++;
	else
	  ro++;

	if (pte & PT_USER) 
	  us++;
	else
	  su++;

	if (pte & PT_ACCESSED) ac++;
	if (pte & PT_DIRTY) dt++;

        if (++lines % 24 == 0) pause();
	kprintf("%08x %08x %c%c%c%c\n", 
	        vaddr, PAGEADDR(pte), 
		(pte & PT_WRITABLE) ? 'w' : 'r',
		(pte & PT_USER) ? 'u' : 's',
		(pte & PT_ACCESSED) ? 'a' : ' ',
		(pte & PT_DIRTY) ? 'd' : ' ');

      }

      vaddr += PAGESIZE;
    }

    if (!vaddr) break;
  }

  kprintf("\ntotal: %d user: %d sys: %d r/w: %d r/o: %d accessed: %d dirty: %d\n", ma, us, su, rw, ro, ac, dt);
}

static void fs_list()
{
  struct fs *fs = mountlist;
  struct dev *dev;

  while (fs)
  {
    if (fs->devno != NODEV)
    {
      dev = devtab[fs->devno];
      kprintf("%s (%s) mounted on %s with %s\n", dev->name, dev->driver->name, fs->path, fs->fsys->name);
    }
    else
      kprintf("%s mounted with %s\n", fs->path, fs->fsys->name);

    fs = fs->next;
  }
}

static void dump_devices()
{
  int i, j;
  struct device *dv;
  int lines = 0;

  for (i = 0; i < (int) num_devices; i++)
  {
    dv = devicetab[i];

    kprintf("%08X %08X ", dv->classcode, dv->devicecode);

    switch (dv->type)
    {
      case DEVICE_TYPE_LEGACY:
	kprintf("%s:", dv->name);
	break;

      case DEVICE_TYPE_PCI:
	if (strcmp(dv->name, "Unknown") == 0)
	  kprintf("PCI %04X:%04X (%X):", dv->pci->vendorid, dv->pci->deviceid, dv->pci->classcode);
	else
	  kprintf("PCI %04X:%04X (%s):", dv->pci->vendorid, dv->pci->deviceid, dv->name);
	break;

      case DEVICE_TYPE_PNP:
	if (strcmp(dv->name, "Unknown") == 0)
	  kprintf("PnP %s (%02X %02X %02X):", dv->pnp->name, dv->pnp->type_code[0], dv->pnp->type_code[1], dv->pnp->type_code[2]);
	else
	  kprintf("PnP %s (%s):", dv->pnp->name, dv->name);
	break;
    }

    for (j = 0; j < dv->numres; j++)
    {
      switch (dv->res[j].type)
      {
	case RESOURCE_IO: 
	  if (dv->res[j].len == 1) 
	    kprintf(" io:%03x", dv->res[j].start);
	  else
	    kprintf(" io:%03x-%03x", dv->res[j].start, dv->res[j].start + dv->res[j].len - 1);
	  break;

	case RESOURCE_MEM:
	  if (dv->res[j].len == 1) 
	    kprintf(" mem:%08x", dv->res[j].start);
	  else
	    kprintf(" mem:%08x-%08x", dv->res[j].start, dv->res[j].start + dv->res[j].len - 1);
	  break;

	case RESOURCE_IRQ:
	  if (dv->res[j].len == 1) 
	    kprintf(" irq:%d", dv->res[j].start);
	  else
	    kprintf(" irq:%d-%d", dv->res[j].start, dv->res[j].start + dv->res[j].len - 1);
	  break;

	case RESOURCE_DMA:
	  if (dv->res[j].len == 1) 
	    kprintf(" dma:%d", dv->res[j].start);
	  else
	    kprintf(" dma:%d-%d", dv->res[j].start, dv->res[j].start + dv->res[j].len - 1);
	  break;
      }
    }

    kprintf("\n");
    if (++lines % 10 == 0) pause();
  }
}

static void thread_list()
{
  static char *threadstatename[] = {"init", "ready", "run", "wait", "term"};
  struct thread *t = threadlist;
  kprintf("tid tcb      self state prio tib      suspend entry    handles\n");
  kprintf("--- -------- ---- ----- ---- -------- ------- -------- -------\n");
  while (1)
  {
    kprintf("%3d %p %4d %-5s %3d  %p  %4d   %p   %2d\n",
            t->id, t, t->self, threadstatename[t->state], t->priority, t->tib, 
	    t->suspend_count, t->entrypoint, t->object.handle_count);

    t = t->next;
    if (t == threadlist) break;
  }
}

static void cmos_dump()
{
  int i;

  for (i = 0; i < 128; i++) buffer[i] = read_cmos_reg(i);
  dump_memory(0, buffer, 128);
}

static void dump_mods(struct moddb *moddb)
{
  struct module *mod = moddb->modules;

  kprintf("handle   module           refs entry      size   text   data    bss\n");
  kprintf("-------- ---------------- ---- --------  -----  -----  -----  -----\n");

  while (1)
  {
    struct image_header *imghdr = get_image_header(mod->hmod);

    kprintf("%08X %-16s %4d %08X %5dK %5dK %5dK %5dK\n", 
            mod->hmod, mod->name, mod->refcnt, 
	    get_entrypoint(mod->hmod),
	    imghdr->optional.size_of_image / K,
	    imghdr->optional.size_of_code / K,
	    imghdr->optional.size_of_initialized_data / K,
	    imghdr->optional.size_of_uninitialized_data / K
	    );

    mod = mod->next;
    if (mod == moddb->modules) break;
  }
}

static void dump_kmods()
{
  dump_mods(&kmods);
}

static void dump_umods()
{
  dump_mods(((struct peb *) PEB_ADDRESS)->usermods);
}

static void dump_bufpools()
{
  struct bufpool *pool;

  pool = bufpools;
  while (pool)
  {
    dump_pool_stat(pool);
    kprintf("\n");
    pool = pool->next;
  }
}

static void load_mod(char *fn)
{
  hmodule_t hmod;

  hmod = load(fn);
  kprintf("hmodule: %08X\n", hmod);
}

static void test(char *arg)
{
  devno_t dev;
  int i;
  int rc;

  dev = dev_open(arg);
  if (dev == NODEV)
  {
    kprintf("%s: unable to open device\n", arg);
    return;
  }

  for (i = 0; i < 512; i++) buffer[i] = i & 0xFF;

  rc = dev_write(dev, buffer, 512, 10);
  kprintf("rc=%d\n", rc);

  dev_close(dev);
}

static void disktest(char *devname, int blocks)
{
  devno_t dev;
  int rc;
  int n, m;
  int ok;

  dev = dev_open(devname);
  if (dev == NODEV)
  {
    kprintf("%s: unable to open device\n", devname);
    return;
  }

  for (n = 0; n < blocks; n++)
  {
    kprintf("write %5d\r", n);
    memset(buffer, ((unsigned char) n) & 0xFF, 512);
  
    rc = dev_write(dev, buffer, 512, n);
    if (rc < 0) 
    { 
      kprintf("%s: write error %d block %d\n", devname, rc, n);
      break;
    }

    rc = dev_write(dev, buffer, 512, n + 2048);
    if (rc < 0) 
    { 
      kprintf("%s: write error %d block %d\n", devname, rc, n);
      break;
    }
  }

  for (n = 0; n < blocks; n++)
  {
    kprintf("read  %5d\r", n);
    memset(buffer, 0, 512);
  
    rc = dev_read(dev, buffer, 512, n);
    if (rc < 0) 
    { 
      kprintf("%s: read error %d block %d\n", devname, rc, n);
      break;
    }

    ok = 1;
    for (m = 0; m < 512; m++)
    {
      if (buffer[m] != (((unsigned char) n) & 0xFF))
      {
	ok = 0;
	break;
      }
    }

    if (!ok)
    {
      kprintf("%s: verify error at %d block %d\n", devname, m, n);
      break;
    }
  }
  kprintf("\n");

  dev_close(dev);
}

void shell()
{
  char cmd[256];
  char *arg;
  char *arg2;

  while (1)
  {
    kprintf("# ");
    gets(cmd);

    arg = cmd;
    while (*arg && *arg != ' ') arg++;
    while (*arg == ' ') *arg++ = 0;

    arg2 = arg;
    while (*arg2 && *arg2 != ' ') arg2++;
    while (*arg2 == ' ') *arg2++ = 0;

    if (strcmp(cmd, "exit") == 0) 
      break;
    else if (strcmp(cmd, "ls") == 0)
      list_dir(arg);
    else if (strcmp(cmd, "cat") == 0)
      list_file(arg);
    else if (strcmp(cmd, "cp") == 0)
      copy_file(arg, arg2);
    else if (strcmp(cmd, "rm") == 0)
      remove_file(arg);
    else if (strcmp(cmd, "mv") == 0)
      move_file(arg, arg2);
    else if (strcmp(cmd, "mkdir") == 0)
      make_dir(arg);
    else if (strcmp(cmd, "rmdir") == 0)
      remove_dir(arg);
    else if (strcmp(cmd, "du") == 0)
      disk_usage(arg);
    else if (strcmp(cmd, "mem") == 0)
      mem_usage();
    else if (strcmp(cmd, "d") == 0)
      dump_memory(atoi(arg), (unsigned char *) atoi(arg), atoi(arg2));
    else if (strcmp(cmd, "dd") == 0)
      dump_disk(arg, atoi(arg2));
    else if (strcmp(cmd, "date") == 0)
      show_date();
    else if (strcmp(cmd, "pcidump") == 0)
      dump_pci_devices();
    else if (strcmp(cmd, "devices") == 0)
      dump_devices();
    else if (strcmp(cmd, "mbr") == 0)
      dump_mbr(arg);
    else if (strcmp(cmd, "mount") == 0)
      mount_device(arg, arg2);
    else if (strcmp(cmd, "reboot") == 0)
      reboot_system();
    else if (strcmp(cmd, "read") == 0)
      test_read_file(arg);
    else if (strcmp(cmd, "write") == 0)
      test_write_file(arg, atoi(arg2) * K);
    else if (strcmp(cmd, "cls") == 0)
      kprintf("\f");
    else if (strcmp(cmd, "alloc") == 0)
      test_alloc(atoi(arg) * K);
    else if (strcmp(cmd, "handles") == 0)
      handle_list();
    else if (strcmp(cmd, "vmem") == 0)
      vmem_list();
    else if (strcmp(cmd, "kmem") == 0)
      kmem_list();
    else if (strcmp(cmd, "mmem") == 0)
      mmem_list();
    else if (strcmp(cmd, "pmem") == 0)
      pmem_list();
    else if (strcmp(cmd, "pdir") == 0)
      dump_pdir();
    else if (strcmp(cmd, "bufpools") == 0)
      dump_bufpools();
    else if (strcmp(cmd, "fs") == 0)
      fs_list();
    else if (strcmp(cmd, "threads") == 0)
      thread_list();
    else if (strcmp(cmd, "cmos") == 0)
      cmos_dump();
    else if (strcmp(cmd, "kmods") == 0)
      dump_kmods();
    else if (strcmp(cmd, "umods") == 0)
      dump_umods();
    else if (strcmp(cmd, "test") == 0)
      test(arg);
    else if (strcmp(cmd, "load") == 0)
      load_mod(arg);
    else if (strcmp(cmd, "break") == 0)
      dbg_break();
    else if (strcmp(cmd, "disktest") == 0)
      disktest(arg, atoi(arg2));
    else if (*cmd)
      kprintf("%s: unknown command\n", cmd);
  }
}
