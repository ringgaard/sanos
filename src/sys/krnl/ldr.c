//
// ldr.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Kernel module loader
//

#include <os/krnl.h>

struct moddb kmods;
struct mutex ldr_lock;

static void *load_image(char *filename)
{
  return load_image_file(filename, 0);
}

static int unload_image(hmodule_t hmod, size_t size)
{
  free_module_mem(hmod, size);
  return 0;
}

void *load_image_file(char *filename, int userspace)
{
  struct file *f;
  char *buffer;
  char *imgbase;
  struct dos_header *doshdr;
  struct image_header *imghdr;
  int i;

  // Allocate header buffer
  buffer = kmalloc(PAGESIZE);
  if (!buffer) return NULL;

  // Open file
  if (open(filename, O_RDONLY, &f) < 0) 
  {
    kfree(buffer);
    return NULL;
  }

  // Read headers
  if (read(f, buffer, PAGESIZE) < 0)
  {
    close(f);
    kfree(buffer);
    return NULL;
  }
  doshdr = (struct dos_header *) buffer;
  imghdr = (struct image_header *) (buffer + doshdr->e_lfanew);

  // Check PE file signature
  if (imghdr->signature != IMAGE_PE_SIGNATURE)  panic("invalid PE signature");

  // Check alignment
  if (imghdr->optional.file_alignment != PAGESIZE || imghdr->optional.section_alignment != PAGESIZE) panic("image not page aligned");

  // Allocate memory for module
  if (userspace)
  {
    // User module
    imgbase = (char *) mmap((void *) (imghdr->optional.image_base), imghdr->optional.size_of_image, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (imgbase == NULL)
    {
      // Try to load image at any available address 
      imgbase = (char *) mmap(NULL, imghdr->optional.size_of_image, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    }
  }
  else
  {
    // Kernel module
    imgbase = (char *) alloc_module_mem(BTOP(imghdr->optional.size_of_image));
    if (imgbase) memset(imgbase, 0, imghdr->optional.size_of_image);
  }

  if (imgbase == NULL)
  {
    close(f);
    kfree(buffer);
    return NULL;
  }

  // Copy header to image
  memcpy(imgbase, buffer, PAGESIZE);

  // Read sections
  for (i = 0; i < imghdr->header.number_of_sections; i++)
  {
    if (imghdr->sections[i].pointer_to_raw_data != 0)
    {
      lseek(f, imghdr->sections[i].pointer_to_raw_data, SEEK_SET);
      if (read(f, RVA(imgbase, imghdr->sections[i].virtual_address), imghdr->sections[i].size_of_raw_data) < 0)
      {
	if (userspace)
	  munmap(imgbase, imghdr->optional.size_of_image, MEM_RELEASE);
	else
	  free_module_mem(imgbase, imghdr->optional.size_of_image);

	close(f);
        kfree(buffer);
	return NULL;
      }
    }
  }

  //kprintf("image %s loaded at %p (%d KB)\n", filename, imgbase, imghdr->optional.size_of_image / 1024);

  // Close file
  close(f);
  kfree(buffer);

  return imgbase;
}

void *resolve(hmodule_t hmod, char *procname)
{
  void *addr;

  wait_for_object(&ldr_lock, INFINITE);
  addr = get_proc_address(hmod, procname);
  release_mutex(&ldr_lock);
  return addr;
}

hmodule_t getmodule(char *name)
{
  hmodule_t hmod;

  wait_for_object(&ldr_lock, INFINITE);
  hmod = get_module_handle(&kmods, name);
  release_mutex(&ldr_lock);
  return hmod;
}

int getmodpath(hmodule_t hmod, char *buffer, int size)
{
  int rc;

  wait_for_object(&ldr_lock, INFINITE);
  rc = get_module_filename(&kmods, hmod, buffer, size);
  release_mutex(&ldr_lock);
  return rc;
}

hmodule_t load(char *name)
{
  hmodule_t hmod;

  wait_for_object(&ldr_lock, INFINITE);
  hmod = load_module(&kmods, name);
  release_mutex(&ldr_lock);
  return hmod;
}

int unload(hmodule_t hmod)
{
  int rc;

  wait_for_object(&ldr_lock, INFINITE);
  rc = unload_module(&kmods, hmod);
  release_mutex(&ldr_lock);
  return rc;
}

void init_kernel_modules()
{
  init_mutex(&ldr_lock, 0);

  kmods.load_image = load_image;
  kmods.unload_image = unload_image;
  kmods.protect_region = NULL;
  kmods.notify_load = dbg_notify_load_module;
  kmods.notify_unload = dbg_notify_unload_module;

  init_module_database(&kmods, "krnl.dll", (hmodule_t) OSBASE, get_property(krnlcfg, "kernel", "libpath", "/os"), 0);
  kmods.execmod = kmods.modules;
}
