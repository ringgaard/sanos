//
// video.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// 6845 Video Controller
//

#include <os.h>

vsprintf(buffer, fmt, args);

#define VIDEO_BASE           0xB8000

#define VIDEO_PORT_REG 	     0x3D4
#define VIDEO_PORT_DATA      0x3D5

#define VIDEO_REG_CURSOR_MSB 0x0E
#define VIDEO_REG_CURSOR_LSB 0x0F

#define COLS                 80
#define LINES                25
#define CELLSIZE             2
#define LINESIZE             (COLS * CELLSIZE)
#define SCREENSIZE           (LINESIZE * LINES)
#define TABSTOP              8

unsigned char *vidmem;
unsigned int cursor_pos;

void init_video()
{
  // set video base address
  vidmem = (unsigned char *) VIDEO_BASE;

  // get cursor position
  _outp(VIDEO_PORT_REG, VIDEO_REG_CURSOR_MSB);
  cursor_pos = _inp(VIDEO_PORT_DATA);
  cursor_pos <<= 8;
  _outp(VIDEO_PORT_REG, VIDEO_REG_CURSOR_LSB);
  cursor_pos += _inp(VIDEO_PORT_DATA);
  cursor_pos <<= 1;
}

static void move_cursor()
{
  // set new cursor position
  unsigned int pos = cursor_pos >> 1;
  _outp(VIDEO_PORT_REG, VIDEO_REG_CURSOR_LSB);
  _outp(VIDEO_PORT_DATA, pos & 0x0ff);
  _outp(VIDEO_PORT_REG, VIDEO_REG_CURSOR_MSB);
  _outp(VIDEO_PORT_DATA, pos >> 8);
}

static void scroll_up()
{
  int i;
  unsigned short *p;

  // scroll screen up
  memcpy(vidmem, vidmem + LINESIZE, SCREENSIZE - LINESIZE);

  // clear bottom row
  p = (unsigned short *) (vidmem + SCREENSIZE - LINESIZE);
  for (i = 0; i < COLS; i++) *p++ = 0x0720;
}

static void print_buffer(const char *str, int len)
{
  int ch;

  if (!str) return;
  while ((ch = *str++) && len--)
  {
    switch (ch)
    {
      case '\n': // newline
	cursor_pos = (cursor_pos / LINESIZE + 1) * LINESIZE;
	break;

      case '\r': // carriage return
	cursor_pos = (cursor_pos / LINESIZE) * LINESIZE;
	break;

      case '\t':
	cursor_pos = (cursor_pos / (TABSTOP * CELLSIZE) + 1) * (TABSTOP * CELLSIZE);
	break;
       
      case 8: // backspace
	if (cursor_pos > 0) cursor_pos -= CELLSIZE;
	vidmem[cursor_pos] = 0x20;
	break;

      default: // normal character
	vidmem[cursor_pos++] = ch;
	vidmem[cursor_pos++] = 0x07;
	break;
    }

    // scroll if position is off-screen
    if (cursor_pos >= SCREENSIZE)
    {
      scroll_up();
      cursor_pos -= LINESIZE;
    }
  }
}

void print_string(const char *str)
{
  print_buffer(str, -1);
  move_cursor();
}

void print_char(int ch)
{
  char c = ch;
  print_buffer(&c, 1);
  move_cursor();
}

void set_cursor(int x, int y)
{
  cursor_pos = (x * COLS + y) * CELLSIZE;
  move_cursor();
}

void clear_screen()
{
  int i;
  unsigned short *p;

  // fill screen with background color
  p = (unsigned short *) vidmem;
  for (i = 0; i < COLS * LINES; i++) *p++ = 0x0720;

  // set cursor to upper-left corner of the screen
  cursor_pos = 0;
  move_cursor();
}

void kprintf(const char *fmt,...)
{
  va_list args;
  char buffer[1024];

  va_start(args, fmt);
  vsprintf(buffer, fmt, args);
  va_end(args);

  print_string(buffer);
}
