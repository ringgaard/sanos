//
// cons.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Console device driver
//

#include <os/krnl.h>

devno_t consdev = NODEV;
static int cursoff = 0;
static unsigned int kbd_timeout = INFINITE;

static int console_ioctl(struct dev *dev, int cmd, void *args, size_t size)
{
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
      keymap = *(int *) args;
      return 0;
  }
  
  return -ENOSYS;
}

static int console_read(struct dev *dev, void *buffer, size_t count, blkno_t blkno)
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
    if (ch < 0) return n;
    
    *p++ = ch;
  }

  return count; 
}

static int console_write(struct dev *dev, void *buffer, size_t count, blkno_t blkno)
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

void init_cons()
{
  dev_make("console", &console_driver, NULL);
  consdev = dev_open("console");
}

void kprintf(const char *fmt,...)
{
  va_list args;
  char buffer[1024];

  va_start(args, fmt);
  vsprintf(buffer, fmt, args);
  va_end(args);
  
  if (consdev == NODEV)
    print_string(buffer);
  else
    dev_write(consdev, buffer, strlen(buffer), 0);
}
