//
// video.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// 6845 Video Controller
//

#include <os/krnl.h>

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
  // Set video base address (use mapped video base)
  vidmem = (unsigned char *) VIDBASE_ADDRESS;

  // Get cursor position
  _outp(VIDEO_PORT_REG, VIDEO_REG_CURSOR_MSB);
  cursor_pos = _inp(VIDEO_PORT_DATA);
  cursor_pos <<= 8;
  _outp(VIDEO_PORT_REG, VIDEO_REG_CURSOR_LSB);
  cursor_pos += _inp(VIDEO_PORT_DATA);
  cursor_pos <<= 1;
}

void show_cursor()
{
  // Set new cursor position
  unsigned int pos = cursor_pos >> 1;
  _outp(VIDEO_PORT_REG, VIDEO_REG_CURSOR_LSB);
  _outp(VIDEO_PORT_DATA, pos & 0x0ff);
  _outp(VIDEO_PORT_REG, VIDEO_REG_CURSOR_MSB);
  _outp(VIDEO_PORT_DATA, pos >> 8);
}

void hide_cursor()
{
  // Set cursor position off screen
  unsigned int pos = COLS * LINES + 1;
  _outp(VIDEO_PORT_REG, VIDEO_REG_CURSOR_LSB);
  _outp(VIDEO_PORT_DATA, pos & 0x0ff);
  _outp(VIDEO_PORT_REG, VIDEO_REG_CURSOR_MSB);
  _outp(VIDEO_PORT_DATA, pos >> 8);
}

static void scroll_up()
{
  int i;
  unsigned short *p;

  // Scroll screen up
  memcpy(vidmem, vidmem + LINESIZE, SCREENSIZE - LINESIZE);

  // Clear bottom row
  p = (unsigned short *) (vidmem + SCREENSIZE - LINESIZE);
  for (i = 0; i < COLS; i++) *p++ = 0x0720;
}

void print_buffer(const char *str, int len)
{
  int ch;
  int i;
  unsigned short *p;

  if (!str) return;
  while ((ch = *str++) && len--)
  {
    switch (ch)
    {
      case '\n': // Newline
	cursor_pos = (cursor_pos / LINESIZE + 1) * LINESIZE;
	break;

      case '\r': // Carriage return
	cursor_pos = (cursor_pos / LINESIZE) * LINESIZE;
	break;

      case '\t':
	cursor_pos = (cursor_pos / (TABSTOP * CELLSIZE) + 1) * (TABSTOP * CELLSIZE);
	break;
       
      case 8: // Backspace
	if (cursor_pos > 0) cursor_pos -= CELLSIZE;
	vidmem[cursor_pos] = 0x20;
	break;

      case 12: // Formfeed
	p = (unsigned short *) vidmem;
	for (i = 0; i < COLS * LINES; i++) *p++ = 0x0720;
        cursor_pos = 0;
	break;

      default: // Normal character
	vidmem[cursor_pos++] = ch;
	vidmem[cursor_pos++] = 0x07;
	break;
    }

    // Scroll if position is off-screen
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
  show_cursor();
}

void print_char(int ch)
{
  char c = ch;
  print_buffer(&c, 1);
  show_cursor();
}

void set_cursor(int x, int y)
{
  cursor_pos = (x * COLS + y) * CELLSIZE;
}

void clear_screen()
{
  int i;
  unsigned short *p;

  // Fill screen with background color
  p = (unsigned short *) vidmem;
  for (i = 0; i < COLS * LINES; i++) *p++ = 0x0720;

  // Set cursor to upper-left corner of the screen
  cursor_pos = 0;
  show_cursor();
}
