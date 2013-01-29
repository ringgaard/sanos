//
// ldr.c
//
// Kernel module loader
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

struct moddb kmods;
struct mutex ldr_lock;

static void *load_image(char *filename) {
  return load_image_file(filename, 0);
}

static int unload_image(hmodule_t hmod, size_t size) {
  free_module_mem(hmod, PAGES(size));
  return 0;
}

void *load_image_file(char *filename, int userspace) {
  struct file *f;
  char *buffer;
  char *imgbase;
  struct dos_header *doshdr;
  struct image_header *imghdr;
  int i;
  unsigned int bytes;

  //kprintf("ldr: loading module %s\n", filename);

  // Allocate header buffer
  buffer = kmalloc(PAGESIZE);
  if (!buffer) return NULL;

  // Open file
  if (open(filename, O_RDONLY | O_BINARY, 0, &f) < 0) {
    kfree(buffer);
    return NULL;
  }

  // Read headers
  if ((bytes = read(f, buffer, PAGESIZE)) < 0) {
    close(f);
    destroy(f);
    kfree(buffer);
    return NULL;
  }
  doshdr = (struct dos_header *) buffer;
  imghdr = (struct image_header *) (buffer + doshdr->e_lfanew);

  // Check PE file signature
  if (doshdr->e_lfanew > bytes || imghdr->signature != IMAGE_PE_SIGNATURE) panic("invalid PE signature");

  // Check alignment
  //if (imghdr->optional.file_alignment != PAGESIZE || imghdr->optional.section_alignment != PAGESIZE) panic("image not page aligned");

  // Allocate memory for module
  if (userspace) {
    // User module
    imgbase = (char *) vmalloc((void *) (imghdr->optional.image_base), imghdr->optional.size_of_image, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE, 'UMOD', NULL);
    if (imgbase == NULL) {
      // Try to load image at any available address 
      imgbase = (char *) vmalloc(NULL, imghdr->optional.size_of_image, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE, 'UMOD', NULL);
    }
  } else {
    // Kernel module
    imgbase = (char *) alloc_module_mem(PAGES(imghdr->optional.size_of_image));
    if (imgbase) memset(imgbase, 0, PAGES(imghdr->optional.size_of_image) * PAGESIZE);
  }

  if (imgbase == NULL) {
    close(f);
    destroy(f);
    kfree(buffer);
    return NULL;
  }

  // Copy header to image
  memcpy(imgbase, buffer, PAGESIZE);

  // Read sections
  for (i = 0; i < imghdr->header.number_of_sections; i++) {
    if (imghdr->sections[i].pointer_to_raw_data != 0) {
      lseek(f, imghdr->sections[i].pointer_to_raw_data, SEEK_SET);
      if (read(f, RVA(imgbase, imghdr->sections[i].virtual_address), imghdr->sections[i].size_of_raw_data) < 0) {
        if (userspace) {
          vmfree(imgbase, imghdr->optional.size_of_image, MEM_RELEASE);
        } else {
          free_module_mem(imgbase, imghdr->optional.size_of_image);
        }

        close(f);
        destroy(f);
        kfree(buffer);
        return NULL;
      }
    }
  }

  //kprintf("image %s loaded at %p (%d KB)\n", filename, imgbase, imghdr->optional.size_of_image / 1024);

  // Close file
  close(f);
  destroy(f);
  kfree(buffer);

  return imgbase;
}

static void logldr(char *msg) {
  kprintf(KERN_WARNING "ldr: %s\n", msg);
}

void *resolve(hmodule_t hmod, char *procname) {
  void *addr;

  wait_for_object(&ldr_lock, INFINITE);
  addr = get_proc_address(hmod, procname);
  release_mutex(&ldr_lock);
  return addr;
}

hmodule_t getmodule(char *name) {
  hmodule_t hmod;

  wait_for_object(&ldr_lock, INFINITE);
  hmod = get_module_handle(&kmods, name);
  release_mutex(&ldr_lock);
  return hmod;
}

int getmodpath(hmodule_t hmod, char *buffer, int size) {
  int rc;

  wait_for_object(&ldr_lock, INFINITE);
  rc = get_module_filename(&kmods, hmod, buffer, size);
  release_mutex(&ldr_lock);
  return rc;
}

hmodule_t load(char *name, int flags) {
  hmodule_t hmod;

  wait_for_object(&ldr_lock, INFINITE);
  hmod = load_module(&kmods, name, flags);
  release_mutex(&ldr_lock);
  return hmod;
}

int unload(hmodule_t hmod) {
  int rc;

  wait_for_object(&ldr_lock, INFINITE);
  rc = unload_module(&kmods, hmod);
  release_mutex(&ldr_lock);
  return rc;
}

void *getentrypoint(hmodule_t hmod) {
  return get_entrypoint(hmod);
}

static int dump_mods(struct proc_file *pf, struct moddb *moddb) {
  struct module *mod = moddb->modules;

  pprintf(pf, "handle   module           refs entry      size   text   data    bss\n");
  pprintf(pf, "-------- ---------------- ---- --------  -----  -----  -----  -----\n");

  while (1) {
    struct image_header *imghdr = get_image_header(mod->hmod);

    pprintf(pf, "%08X %-16s %4d %08X %5dK %5dK %5dK %5dK\n", 
            mod->hmod, mod->name, mod->refcnt, 
            get_entrypoint(mod->hmod),
            imghdr->optional.size_of_image / 1024,
            imghdr->optional.size_of_code / 1024,
            imghdr->optional.size_of_initialized_data / 1024,
            imghdr->optional.size_of_uninitialized_data / 1024
            );

    mod = mod->next;
    if (mod == moddb->modules) break;
  }

  return 0;
}

static int kmods_proc(struct proc_file *pf, void *arg) {
  return dump_mods(pf, &kmods);
}

static int umods_proc(struct proc_file *pf, void *arg) {
  if (!page_mapped((void *) PEB_ADDRESS)) return -EFAULT;
  if (!((struct peb *) PEB_ADDRESS)->usermods) return -EFAULT;

  return dump_mods(pf, ((struct peb *) PEB_ADDRESS)->usermods);
}

void init_kernel_modules() {
  struct module *krnlmod;

  init_mutex(&ldr_lock, 0);

  kmods.read_magic = NULL;
  kmods.load_image = load_image;
  kmods.unload_image = unload_image;
  kmods.protect_region = NULL;
  kmods.log = logldr;
  kmods.notify_load = dbg_notify_load_module;
  kmods.notify_unload = dbg_notify_unload_module;

  init_module_database(&kmods, "krnl.dll", (hmodule_t) OSBASE, get_property(krnlcfg, "kernel", "libpath", "/boot"), find_section(krnlcfg, "modaliases"), 0);

  register_proc_inode("kmods", kmods_proc, NULL);
  register_proc_inode("umods", umods_proc, NULL);

  krnlmod = kmods.modules;
  set_pageframe_tag(krnlmod->hmod, get_image_header(krnlmod->hmod)->optional.size_of_image, 'KMOD');
}
