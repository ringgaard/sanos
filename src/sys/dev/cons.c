//
// cons.c
//
// Console device driver
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

#define CTRL(c) ((c) - 'A' + 1)

dev_t consdev = NODEV;
static int cursoff = 0;
static unsigned int kbd_timeout = INFINITE;

void sound(unsigned short freq) 
{
  unsigned short freqdiv;
  unsigned char b;

  freqdiv = 1193180 / freq;

  // Counter 2 select, binary 16-bit counter, first bits 0-7
  _outp(0x43, 0xB6); 
         
  // First bits 0-7
  _outp(0x42, (unsigned char) freqdiv);
  
  // Then bits 8-15
  _outp(0x42, (unsigned char) (freqdiv >> 8)); 

  // Only output if bits are not correctly set
  b = _inp(0x61);
  if (b != (b | 3)) _outp(0x61, b | 3);
}

void nosound() 
{
  unsigned char b;

   // KB controller port B
  b = _inp(0x61);

  // Disable speaker + clock 2 gate
  b &= 0xFC;

  // Output
  _outp(0x61, b);
}
 
void beep() 
{
  sound(1000);
  msleep(250);
  nosound();
}

static int console_ioctl(struct dev *dev, int cmd, void *args, size_t size)
{
  int freq;

  switch (cmd)
  {
    case IOCTL_GETDEVSIZE:
      return 0;

    case IOCTL_GETBLKSIZE:
      return 1;

    case IOCTL_SET_KEYTIMEOUT:
      if (!args || size != 4) return -EINVAL;
      kbd_timeout = *(unsigned int *) args;
      return 0;

    case IOCTL_CTRLALTDEL_ENABLED:
      if (!args || size != 4) return -EINVAL;
      ctrl_alt_del_enabled = *(int *) args;
      return 0;

    case IOCTL_SET_KEYMAP:
      if (!args || size != 4) return -EINVAL;
      return change_keyboard_map_id(*(int *) args);

    case IOCTL_BEEP:
      beep();
      return 0;

    case IOCTL_SOUND:
      if (!args || size != 4) return -EINVAL;
      freq = *(int *) args;
      if (freq < 0 || freq > 0xFFFF) return -EINVAL;

      if (freq == 0)
	nosound();
      else
        sound((unsigned short) freq);

      return 0;

    case IOCTL_REBOOT:
      stop(EXITOS_REBOOT);
      return 0;

    case IOCTL_KBHIT:
      if (cursoff)
      {
	cursoff = 0;
	show_cursor();
      }
      return kbhit();
  }
  
  return -ENOSYS;
}

static int console_read(struct dev *dev, void *buffer, size_t count, blkno_t blkno, int flags)
{
  char *p = (char *) buffer;
  int ch;
  int n;

  for (n = 0; n < (int) count; n++)
  {
    if (cursoff)
    {
      cursoff = 0;
      show_cursor();
    }

    ch = getch(n ? 0 : kbd_timeout);
    if (ch < 0) return n ? n : ch;
    
    if (ch < ' ')
    {
      if (ch == CTRL('C')) send_user_signal(self(), SIGINT);
      if (ch == CTRL('Z')) send_user_signal(self(), SIGTSTP);
      if (ch == CTRL('\\')) send_user_signal(self(), SIGABRT);
    }
    
    *p++ = ch;
  }

  return count; 
}

static int console_write(struct dev *dev, void *buffer, size_t count, blkno_t blkno, int flags)
{
  if (!cursoff)
  {
    cursoff = 1;
    hide_cursor();
  }

  print_buffer((char *) buffer, count);

  return count;
}

struct driver console_driver =
{
  "console",
  DEV_TYPE_STREAM,
  console_ioctl,
  console_read,
  console_write
};

int __declspec(dllexport) console(struct unit *unit, char *opts)
{
  init_keyboard(get_num_option(opts, "resetkbd", 0));
  dev_make("console", &console_driver, NULL, NULL);
  consdev = dev_open("console");
  register_proc_inode("screen", screen_proc, NULL);

  return 0;
}
