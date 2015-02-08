//
// vga.h
//
// VESA BIOS Extension definitions
//
// Copyright (C) 2015 Michael Ringgaard. All rights reserved.
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

#ifndef VGA_H
#define VGA_H

#define VESA_ADDR(ptr) (((ptr) & 0xFFFF) | ((ptr) >> 12))

#pragma pack(push, 1)

// VGA driver ioctl codes

#define IOCTL_VGA_GET_VIDEO_MODE   1024
#define IOCTL_VGA_GET_FRAME_BUFFER 1025

// VESA BIOS extension information

struct vesa_info {
  unsigned char vesa_signature[4];
  unsigned short vesa_version;
  unsigned long oem_string_ptr;
  unsigned char capabilities[4];
  unsigned long video_mode_ptr;
  unsigned short total_memory;
  unsigned short oem_software_rev;
  unsigned long  oem_vendor_name_ptr;
  unsigned long  oem_product_name_ptr;
  unsigned long  oem_product_rev_ptr;
  unsigned char  reserved[222];
  unsigned char  oem_data[256];
};

// VESA mode information

struct vesa_mode_info {
  unsigned short attributes;
  unsigned char wina_attributes;
  unsigned char winb_attributes;
  unsigned short win_granularity;
  unsigned short win_size;
  unsigned short wina_segment;
  unsigned short winb_segment;
  unsigned long win_func_ptr;
  unsigned short bytes_per_scanline;

  unsigned short x_resolution;
  unsigned short y_resolution;
  unsigned char x_char_size;
  unsigned char y_char_size;
  unsigned char number_of_planes;
  unsigned char bits_per_pixel;
  unsigned char number_of_banks;
  unsigned char memory_model;
  unsigned char bank_size;
  unsigned char number_of_image_pages;
  unsigned char reserved_page;

  unsigned char red_mask_size;
  unsigned char red_mask_pos;
  unsigned char green_mask_size;
  unsigned char green_mask_pos;
  unsigned char blue_mask_size;
  unsigned char blue_mask_pos;
  unsigned char reserved_mask_size;
  unsigned char reserved_mask_pos;
  unsigned char direct_color_mode_info;

  unsigned long phys_base_ptr;
  unsigned long off_screen_mem_offset;
  unsigned short off_screen_mem_size;

  unsigned char reserved[206];
};

#pragma pack(pop)

#endif

