//
// video.c
//
// 6845 Video Controller
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

#define VIDEO_PORT_REG       0x3D4
#define VIDEO_PORT_DATA      0x3D5

#define VIDEO_REG_CURSOR_MSB 0x0E
#define VIDEO_REG_CURSOR_LSB 0x0F

#define COLS                 80
#define LINES                25
#define CELLSIZE             2
#define LINESIZE             (COLS * CELLSIZE)
#define SCREENSIZE           (LINESIZE * LINES)
#define TABSTOP              8

#define ATTR_NORMAL          0x07
#define ATTR_INVERSE         0x70

unsigned char *vidmem;
int cursor_pos;
unsigned char video_attr = ATTR_NORMAL;
int video_state = 0;
int saved_cursor_pos = 0;
int linewrap = 1;

static int color_map[8] = {0, 4, 2, 6, 1, 5, 3, 7};

void init_video() {
  // Set video base address (use mapped video base)
  vidmem = (unsigned char *) VIDBASE_ADDRESS;

  // Get cursor position
  outp(VIDEO_PORT_REG, VIDEO_REG_CURSOR_MSB);
  cursor_pos = inp(VIDEO_PORT_DATA) & 0xFF;
  cursor_pos <<= 8;
  outp(VIDEO_PORT_REG, VIDEO_REG_CURSOR_LSB);
  cursor_pos += inp(VIDEO_PORT_DATA) & 0xFF;
  cursor_pos <<= 1;
}

void show_cursor() {
  // Set new cursor position
  unsigned int pos = cursor_pos >> 1;
  outp(VIDEO_PORT_REG, VIDEO_REG_CURSOR_LSB);
  outp(VIDEO_PORT_DATA, pos & 0xFF);
  outp(VIDEO_PORT_REG, VIDEO_REG_CURSOR_MSB);
  outp(VIDEO_PORT_DATA, pos >> 8);
}

void hide_cursor() {
  // Set cursor position off screen
  unsigned int pos = COLS * LINES + 1;
  outp(VIDEO_PORT_REG, VIDEO_REG_CURSOR_LSB);
  outp(VIDEO_PORT_DATA, pos & 0x0ff);
  outp(VIDEO_PORT_REG, VIDEO_REG_CURSOR_MSB);
  outp(VIDEO_PORT_DATA, pos >> 8);
}

void set_cursor(int col, int line) {
  cursor_pos = (line * COLS + col) * CELLSIZE;
}

static void clear_line(unsigned char *p) {
  int i;
  unsigned char attr = video_attr;

  for (i = 0; i < COLS; i++) {
    *p++ = ' ';
    *p++ = attr;
  }
}

static void scroll_up() {
  // Scroll screen up
  memcpy(vidmem, vidmem + LINESIZE, SCREENSIZE - LINESIZE);

  // Clear bottom row
  clear_line(vidmem + SCREENSIZE - LINESIZE);
}

static void scroll_down() {
  // Scroll screen down
  memcpy(vidmem + LINESIZE, vidmem, SCREENSIZE - LINESIZE);

  // Clear top row
  clear_line(vidmem);
}

static void handle_sequence(int x, int y, char ch) {
  switch (ch) {
    case 'H': { // Position
      int line = x;
      int col = y;

      if (line < 1) line = 1;
      if (line > LINES) line = LINES;
      if (col < 1) col = 1;
      if (col > COLS) col = COLS;
      set_cursor(col - 1, line - 1);
      break;
    }
    
    case 'J': // Clear screen/eos
      if (x == 1) {
        unsigned char *p = vidmem + cursor_pos;
        while (p < vidmem + SCREENSIZE) {
          *p++ = ' ';
          *p++ = video_attr;
        }
      } else {
        unsigned char *p = vidmem;
        while (p < vidmem + SCREENSIZE) {
          *p++ = ' ';
          *p++ = video_attr;
        }
        cursor_pos = 0;
      }
      break;
    
    case 'K': { // Clear to end of line
      int pos = cursor_pos;
      do {
        vidmem[pos++] = ' ';
        vidmem[pos++] = video_attr;
      } while ((pos) % LINESIZE != 0);
      break;
    }
    
    case 'm': // Set character enhancements
      // Modified for ANSI color attributes 3/15/07 - C Girdosky
      if (x >= 30 && x <= 37) {
        // Foreground color
        video_attr = color_map[x - 30] + (video_attr & 0xF8);
      } else if (x >= 40 && x <= 47) {
        // Background color
        video_attr = (color_map[x - 40] << 4) + (video_attr & 0x8F);
      } else if (x == 1) {
        // High intensity foreground
        video_attr = video_attr | 8;
      } else if (x == 2) {
        // Low intensity foreground
        video_attr = video_attr & ~8;
      } else if (x == 5) {
        // High intensity background
        video_attr = video_attr | 128;
      } else if (x == 6) {
        // Low intensity background
        video_attr = video_attr & ~128;
      } else if (x == 7) {
        // Reverse
        video_attr = ((video_attr & 0xF0) >> 4) + ((video_attr & 0x0F) << 4); 
      } else if (x == 8) {
        // Invisible make forground match background
        video_attr = ((video_attr & 0xF0) >> 4) + (video_attr & 0xF0);
      } else {
        video_attr = ATTR_NORMAL;
      }
      break;

    case 'A':  // Cursor up
      while (x-- > 0) {
        cursor_pos -= LINESIZE;
        if (cursor_pos < 0) {
          cursor_pos += LINESIZE;
          break;
        }
      }
      break;

    case 'B': // Cursor down
      while (x-- > 0) {
        cursor_pos += LINESIZE;
        if (cursor_pos >= SCREENSIZE) {
          cursor_pos -= LINESIZE;
          break;
        }
      }
      break;
    
    case 'C': // Cursor right
      cursor_pos += x * CELLSIZE;
      if (cursor_pos >= SCREENSIZE) cursor_pos = SCREENSIZE - 1;
      break;
    
    case 'D': // Cursor left
      cursor_pos -= x * CELLSIZE;
      if (cursor_pos < 0) cursor_pos = 0;
      break;

    case 'L': // Insert line
      while (x-- > 0) {
        int sol = cursor_pos - cursor_pos % LINESIZE;
        memcpy(vidmem + sol + LINESIZE, vidmem + sol, SCREENSIZE - LINESIZE - sol);
        clear_line(vidmem + sol * LINESIZE);
      }
      break;

    case 'M': // Delete line
      while (x-- > 0) {
        int sol = cursor_pos - cursor_pos % LINESIZE;
        memcpy(vidmem + sol, vidmem + sol + LINESIZE, SCREENSIZE - LINESIZE - sol);
        clear_line(vidmem + SCREENSIZE - LINESIZE);
      }
      break;

    case '@': // Insert character
      while (x-- > 0) {
        memcpy(vidmem + cursor_pos + CELLSIZE, vidmem + cursor_pos, SCREENSIZE - cursor_pos - 1);
        vidmem[cursor_pos] = ' ';
        vidmem[cursor_pos + 1] = video_attr;
      }
      break;

    case 'P': // Delete character
      while (x-- > 0) {
        memcpy(vidmem + cursor_pos, vidmem + cursor_pos + CELLSIZE, SCREENSIZE - cursor_pos - 1);
        vidmem[SCREENSIZE - 2] = ' ';
        vidmem[SCREENSIZE - 1] = video_attr;
      }
      break;
    
    case 's': // Save cursor
      saved_cursor_pos = cursor_pos;
      break;

    case 'u': // Restore cursor
      cursor_pos = saved_cursor_pos;
      break;

    case 'h': // Enable line wrapping
      if (x == 7) linewrap = 1;
      break;

    case 'l': // Disable line wrapping
      if (x == 7) linewrap = 0;
      break;
  }
}

static int handle_multichar(int state, char ch) {
  static int x, y;

  switch (state) {
    case 1: // Escape has arrived
      switch (ch) {
        case '[': // Extended sequence
          return 2;

        case 'P': // Cursor down a line
          cursor_pos += LINESIZE;
          if (cursor_pos >= SCREENSIZE) cursor_pos -= LINESIZE;
          return 0;

        case 'K': // Cursor left
          if (cursor_pos > 0) cursor_pos -= CELLSIZE;
          return 0;

        case 'H': // Cursor up
          cursor_pos -= LINESIZE;
          if (cursor_pos < 0) cursor_pos += LINESIZE;
          return 0;

        case 'D': // Scroll forward
          scroll_up();
          return 0;

        case 'M': // Scroll reverse
          scroll_down();
          return 0;

        case 'G':  // Cursor home
          cursor_pos = 0;
          return 0;

        case '(': // Extended char set
          return 5;

        case 'c': // Reset
          cursor_pos = 0;
          video_attr = ATTR_NORMAL;
          linewrap = 1;
          return 0;

        default:
          return 0;
      }

    case 2: // Seen Esc-[
      if (ch == '?') return 3;
      if (ch >= '0' && ch <= '9') {
        x = ch - '0';
        return 3;
      }

      if (ch == 's' || ch == 'u') {
        handle_sequence(0, 0, ch);
      } else if (ch == 'r' || ch == 'm') {
        handle_sequence(0, 0, ch);
      } else {
        handle_sequence(1, 1, ch);
      }
      return 0;

    case 3: // Seen Esc-[<digit>
      if (ch >= '0' && ch <= '9') {
        x = x * 10 + (ch - '0');
        return 3;
      }
      if (ch == ';') {
        y = 0;
        return 4;
      }
      handle_sequence(x, 1, ch);
      return 0;

    case 4:  // Seen Esc-[<digits>;
      if (ch >= '0' && ch <= '9') {
        y = y * 10 + (ch - '0');
        return 4;
      }
      handle_sequence(x, y, ch);
      return 0;

    case 5: // Seen Esc-(
      // Ignore char set selection
      return 0;

    default:
      return 0;
  }
}

void print_buffer(const char *str, int len) {
  int ch;
  int i;
  unsigned char *p;
  char *end;
  unsigned char attr = video_attr;

  if (!str) return;
  
  end = (char *) str + len;
  while (str < end) {
    ch = *str++;

    // If we are inside a multi-character sequence handle it using state machine
    if (video_state > 0) {
      video_state = handle_multichar(video_state, ch);
      attr = video_attr;
      continue;
    }

    if (ch >= ' ') {
      vidmem[cursor_pos++] = ch;
      vidmem[cursor_pos++] = attr;
    } else {
      switch (ch) {
        case 0:
          break;

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
          break;

        case 12: // Formfeed
          p = (unsigned char *) vidmem;
          for (i = 0; i < COLS * LINES; i++) {
            *p++ = ' ';
            *p++ = attr;
          }
          cursor_pos = 0;
          break;

        case 27: // Escape
          video_state = 1;
          break;

        default: // Normal character
          vidmem[cursor_pos++] = ch;
          vidmem[cursor_pos++] = attr;
      }
    }

    // Scroll if position is off-screen
    if (cursor_pos >= SCREENSIZE) {
      if (linewrap) {
        scroll_up();
        cursor_pos -= LINESIZE;
      } else {
        cursor_pos = SCREENSIZE - CELLSIZE;
      }
    }
  }
}

void print_string(const char *str) {
  if (str) print_buffer(str, strlen(str));
  show_cursor();
}

void print_char(int ch) {
  char c = ch;
  print_buffer(&c, 1);
  show_cursor();
}

void clear_screen() {
  int i;
  unsigned char *p;
  unsigned char attr = video_attr;

  // Fill screen with background color
  p = (unsigned char *) vidmem;
  for (i = 0; i < COLS * LINES; i++) {
    *p++ = ' ';
    *p++ = attr;
  }

  // Set cursor to upper-left corner of the screen
  cursor_pos = 0;
  show_cursor();
}

int screen_proc(struct proc_file *pf, void *arg) {
  int i, j;
  unsigned char *p;

  // Dump screen to proc files
  p = (unsigned char *) vidmem;
  for (i = 0; i < LINES; i++) {
    for (j = 0; j < COLS; j++) {
      proc_write(pf, p, 1);
      p += 2;
    }
    proc_write(pf, "\r\n", 2);
  }

  return 0;
}
