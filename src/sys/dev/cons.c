//
// cons.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Console device driver
//

#include <os/krnl.h>

#define SYSLOG_SIZE (512 * K)

devno_t consdev = NODEV;
static int cursoff = 0;
static int kprint_enabled = 1;
static unsigned int kbd_timeout = INFINITE;
static char syslog[SYSLOG_SIZE];
static unsigned int syslog_start;
static unsigned int syslog_end;
static unsigned int syslog_size;

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

    case IOCTL_KPRINT_ENABLED:
      if (!args || size != 4) return -EINVAL;
      kprint_enabled = *(int *) args;
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

static void add_to_syslog(char *buf, int size)
{
  while (size-- > 0)
  {
    if (syslog_size == SYSLOG_SIZE)
    {
      while (syslog[syslog_start] != '\n' && syslog_size > 0) 
      {
	syslog_size--;
	syslog_start++;
	if (syslog_start == SYSLOG_SIZE) syslog_start = 0;
      }

      if (syslog_size > 0)
      {
	syslog_size--;
	syslog_start++;
	if (syslog_start == SYSLOG_SIZE) syslog_start = 0;
      }
    }

    syslog[syslog_end++] = *buf++;
    if (syslog_end == SYSLOG_SIZE) syslog_end = 0;
    syslog_size++;
  }
}

void kprintf(const char *fmt,...)
{
  va_list args;
  char buffer[1024];
  int len;

  va_start(args, fmt);
  len = vsprintf(buffer, fmt, args);
  va_end(args);
    
  add_to_syslog(buffer, len);
  
  //if (debugging) dbg_output(buffer);

  if (kprint_enabled)
  {
    if (consdev == NODEV)
      print_string(buffer);
    else
    {
      dev_write(consdev, buffer, len, 0);
    }
  }
}

static int syslog_ioctl(struct dev *dev, int cmd, void *args, size_t size)
{
  switch (cmd)
  {
    case IOCTL_GETDEVSIZE:
      return syslog_size;

    case IOCTL_GETBLKSIZE:
      return 1;
  }

  return -ENOSYS;
}

static int syslog_read(struct dev *dev, void *buffer, size_t count, blkno_t blkno)
{
  char *ptr;
  unsigned int idx;
  unsigned int n;

  if (blkno > syslog_size) return -EFAULT;
  if (blkno + count > syslog_size) count = syslog_size - blkno;
  if (count == 0) return 0;
  
  ptr = (char *) buffer;
  idx = (syslog_start + blkno) % SYSLOG_SIZE;
  n = count;
  while (n-- > 0)
  {
    *ptr++ = syslog[idx++];
    if (idx == SYSLOG_SIZE) idx = 0;
  }

  return count;
}

static int syslog_write(struct dev *dev, void *buffer, size_t count, blkno_t blkno)
{
  return -ENOSYS;
}

struct driver syslog_driver =
{
  "syslog",
  DEV_TYPE_BLOCK,
  syslog_ioctl,
  syslog_read,
  syslog_write
};

int __declspec(dllexport) install_console()
{
  init_keyboard();
  dev_make("console", &console_driver, NULL, NULL);
  dev_make("syslog", &syslog_driver, NULL, NULL);
  consdev = dev_open("console");

  return 0;
}
