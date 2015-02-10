//
// vga.c
//
// VGA graphics driver
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
#include <os/vga.h>

struct vga {
  struct vesa_mode_info *mode;  // Graphics mode
  unsigned char *fb;            // Frame buffer
  unsigned int fbsize;          // Frame buffer size
};

static int vga_ioctl(struct dev *dev, int cmd, void *args, size_t size) {
  struct vga *vga = (struct vga *) dev->privdata;

  switch (cmd) {
    case IOCTL_GETDEVSIZE:
      return vga->fbsize;

    case IOCTL_GETBLKSIZE:
      return 1;

    case IOCTL_VGA_GET_VIDEO_MODE:
      if (!args) return -EINVAL;
      if (size > sizeof(struct vesa_mode_info)) size = sizeof(struct vesa_mode_info);
      memcpy(args, vga->mode, size);
      return 0;

    case IOCTL_VGA_GET_FRAME_BUFFER:
      if (!args || size != sizeof(unsigned char *)) return -EINVAL;
      *((unsigned char **) args) = vga->fb;
      return 0;
  }

  return -ENOSYS;
}

static int vga_read(struct dev *dev, void *buffer, size_t count, blkno_t blkno, int flags) {
  struct vga *vga = (struct vga *) dev->privdata;

  if (blkno > vga->fbsize) return -EFAULT;
  if (blkno + count > vga->fbsize) count = vga->fbsize - blkno;
  if (count == 0) return 0;
  memcpy(buffer, vga->fb + blkno, count);

  return count;
}

static int vga_write(struct dev *dev, void *buffer, size_t count, blkno_t blkno, int flags) {
  struct vga *vga = (struct vga *) dev->privdata;

  if (blkno > vga->fbsize) return -EFAULT;
  if (blkno + count > vga->fbsize) count = vga->fbsize - blkno;
  if (count == 0) return 0;
  memcpy(vga->fb + blkno, buffer, count);

  return count;
}

static int color_mask(int size, int pos) {
  return size ? ((1 << size) - 1) << pos : 0;
}

static int vgainfo_proc(struct proc_file *pf, void *arg) {
  struct vga *vga = (struct vga *) arg;
  struct vesa_mode_info *info = vga->mode;

  pprintf(pf, "Resolution.......... : %dx%dx%d\n", info->x_resolution, info->y_resolution, info->bits_per_pixel);
  pprintf(pf, "Color resolution.... : R=%d G=%d B=%d A=%d\n", 
          1 << info->red_mask_size, 
          1 << info->green_mask_size, 
          1 << info->blue_mask_size, 
          1 << info->reserved_mask_size);
  pprintf(pf, "Character size...... : %dx%d\n", info->x_char_size, info->y_char_size);
  pprintf(pf, "VGA attributes...... : %x\n", info->attributes);
  pprintf(pf, "Bytes per scanline.. : %d\n", info->bytes_per_scanline);
  pprintf(pf, "Memory model........ : %d\n", info->memory_model);
  pprintf(pf, "Planes.............. : %d\n", info->number_of_planes);
  pprintf(pf, "Banks............... : %d\n", info->number_of_banks);
  pprintf(pf, "Bank size........... : %d\n", info->bank_size);
  pprintf(pf, "Image pages......... : %d\n", info->number_of_image_pages);
  pprintf(pf, "Color masks......... : R=%08x G=%08x B=%08x A=%08x\n", 
          color_mask(info->red_mask_size, info->red_mask_pos),
          color_mask(info->green_mask_size, info->green_mask_pos),
          color_mask(info->blue_mask_size, info->blue_mask_pos),
          color_mask(info->reserved_mask_size, info->reserved_mask_pos));
  pprintf(pf, "Reserved page....... : %d\n", info->reserved_page);
  pprintf(pf, "Direct color mode... : %d\n", info->direct_color_mode_info);
  pprintf(pf, "Frame buffer addr... : phys: %08x virt: %08X size: %d\n", info->phys_base_ptr, vga->fb, vga->fbsize);
  pprintf(pf, "Off-screen memory... : %08x (%d bytes)\n", info->off_screen_mem_offset, info->off_screen_mem_size);
  pprintf(pf, "Window A segment.... : %d\n", info->wina_segment);
  pprintf(pf, "Window B segment.... : %d\n", info->winb_segment);
  pprintf(pf, "Window func......... : %08x\n", info->win_func_ptr);

  return 0;
}

struct driver vga_driver = {
  "vga",
  DEV_TYPE_BLOCK,
  vga_ioctl,
  vga_read,
  vga_write
};

int __declspec(dllexport) vga(struct unit *unit) {
  struct vga *vga;
  struct vesa_mode_info *mode = (struct vesa_mode_info *) syspage->vgainfo;
  dev_t devno;

  // Check if graphics mode is enabled
  if (!mode->phys_base_ptr) return 0;

  // Allocate memory for device
  vga = kmalloc(sizeof(struct vga));
  if (vga == NULL) return -ENOMEM;
  memset(vga, 0, sizeof(struct vga));
  vga->mode = mode;

  // Make frame buffer user accessible
  vga->fbsize = mode->y_resolution * mode->bytes_per_scanline;
  vga->fb = miomap(mode->phys_base_ptr, vga->fbsize, PAGE_READWRITE);
  if (!vga->fb) return -ENOMEM;

  devno = dev_make("fb#", &vga_driver, unit, vga);
  kprintf(KERN_INFO "%s: VGA %dx%dx%d, frame buffer %x (%d)\n", 
          device(devno)->name, 
          mode->x_resolution, mode->y_resolution, mode->bits_per_pixel,
          mode->phys_base_ptr, vga->fbsize);

  // VGA information
  register_proc_inode("vgainfo", vgainfo_proc, vga);

  return 0;
}

