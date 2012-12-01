//
// edit.c
//
// Text editor
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

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef SANOS
#include <os.h>
#endif

#ifdef __linux__
#include <sys/ioctl.h>
#include <termios.h>
#define O_BINARY 0
#endif

#define MINEXTEND      32768
#define LINEBUF_EXTRA  32
#define TABSIZE        8

//
// Key codes
//

#define KEY_BACKSPACE   0x101
#define KEY_ESC         0x102
#define KEY_INS         0x103
#define KEY_DEL         0x104
#define KEY_LEFT        0x105
#define KEY_RIGHT       0x106
#define KEY_UP          0x107
#define KEY_DOWN        0x108
#define KEY_HOME        0x109
#define KEY_END         0x10A
#define KEY_ENTER       0x10B
#define KEY_TAB         0x10C
#define KEY_PGUP        0x10D
#define KEY_PGDN        0x10E

#define KEY_CTRL_LEFT   0x10F
#define KEY_CTRL_RIGHT  0x110
#define KEY_CTRL_UP     0x111
#define KEY_CTRL_DOWN   0x112

#define KEY_SHIFT_LEFT  0x113
#define KEY_SHIFT_RIGHT 0x114
#define KEY_SHIFT_UP    0x115
#define KEY_SHIFT_DOWN  0x116
#define KEY_SHIFT_PGUP  0x117
#define KEY_SHIFT_PGDN  0x118

#define KEY_CTRL_HOME   0x119
#define KEY_CTRL_END    0x11A

#define KEY_F1          0x11B

#define KEY_UNKNOWN     0xFFF

#define ctrl(c) ((c) - 0x60)

//
// Editor data block
//
// Structure of split buffer:
//
//    +------------------+------------------+------------------+
//    | text before gap  |        gap       |  text after gap  |
//    +------------------+------------------+------------------+
//    ^                  ^                  ^                  ^     
//    |                  |                  |                  |
//  start               gap                rest               end
//

struct editor {
  unsigned char *start;     // Start of text buffer
  unsigned char *gap;       // Start of gap
  unsigned char *rest;      // End of gap
  unsigned char *end;       // End of text buffer

  int toppos;               // Text position for current top screen line
  int topline;              // Line number for top of screen
  int margin;               // Position for leftmost column on screen

  int linepos;              // Text position for current line
  int line;                 // Current document line
  int col;                  // Current document column
  int lastcol;              // Remembered column from last horizontal navigation
  
  int anchor;               // Anchor position for selection
  unsigned char *clipboard; // Clipboard
  int clipsize;             // Clipboard size
  unsigned char *search;    // Search text.

  int refresh;              // Flag to trigger screen redraw
  int lineupdate;           // Flag to trigger redraw of current line
  int dirty;                // Dirty flag is set when the editor buffer has been changed

  int minihelp;             // Flag to control mini help in status line
  int newfile;              // File is a new file

  int cols;                 // Console columns
  int lines;                // Console lines

  unsigned char *linebuf;
  char filename[FILENAME_MAX];
};

//
// Editor buffer functions
//

int new_file(struct editor *ed, char *filename) {
  memset(ed, 0, sizeof(struct editor));
  strcpy(ed->filename, filename);

  ed->start = (unsigned char *) malloc(MINEXTEND);
  if (!ed->start) return -1;
#ifdef DEBUG
  memset(ed->start, 0, MINEXTEND);
#endif

  ed->gap = ed->start;
  ed->rest = ed->end = ed->gap + MINEXTEND;
  ed->anchor = -1;
  ed->newfile = 1;
  return 0;
}

int load_file(struct editor *ed, char *filename) {
  struct stat statbuf;
  int length;

  int f = open(filename, O_RDONLY | O_BINARY);
  if (f < 0) return -1;

  memset(ed, 0, sizeof(struct editor));
  strcpy(ed->filename, filename);

  if (fstat(f, &statbuf) < 0) goto err;
  length = statbuf.st_size;

  ed->start = (unsigned char *) malloc(length + MINEXTEND);
  if (!ed->start) goto err;
#ifdef DEBUG
  memset(ed->start, 0, length + MINEXTEND);
#endif
  if (read(f, ed->start, length) != length) goto err;

  ed->gap = ed->start + length;
  ed->rest = ed->end = ed->gap + MINEXTEND;
  ed->anchor = -1;

  close(f);
  return 0;

err:
  close(f);
  if (ed->start) free(ed->start);
  return -1;
}

int save_file(struct editor *ed, char *filename) {
  int f;

  if (!filename) filename = ed->filename;

  f = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);
  if (f < 0) return -1;

  if (write(f, ed->start, ed->gap - ed->start) != ed->gap - ed->start) goto err;
  if (write(f, ed->rest, ed->end - ed->rest) != ed->end - ed->rest) goto err;

  close(f);
  ed->dirty = 0;
  return 0;

err:
  close(f);
  return -1;
}

void delete_editor(struct editor *ed) {
  if (ed->start) free(ed->start);
  if (ed->linebuf) free(ed->linebuf);
  if (ed->clipboard) free(ed->clipboard);
  if (ed->search) free(ed->search);
}

int text_length(struct editor *ed) {
  return (ed->gap - ed->start) + (ed->end - ed->rest);
}

unsigned char *text_ptr(struct editor *ed, int pos) {
  unsigned char *p = ed->start + pos;
  if (p >= ed->gap) p += (ed->rest - ed->gap);
  return p;
}

void move_gap(struct editor *ed, int pos, int minsize) {
  int gapsize = ed->rest - ed->gap;
  unsigned char *p = text_ptr(ed, pos);
  if (minsize < 0) minsize = 0;

  if (minsize <= gapsize) {
    if (p != ed->rest) {
      if (p < ed->gap) {
        memmove(p + gapsize, p, ed->gap - p);
      } else {
        memmove(ed->gap, ed->rest, p - ed->rest);
      }
      ed->gap = ed->start + pos;
      ed->rest = ed->gap + gapsize;
    }
  } else {
    int newsize;
    unsigned char *start;
    unsigned char *gap;
    unsigned char *rest;
    unsigned char *end;

    if (gapsize + MINEXTEND > minsize) minsize = gapsize + MINEXTEND;
    newsize = (ed->end - ed->start) - gapsize + minsize;
    start = (unsigned char *) malloc(newsize); // TODO check for out of memory
    gap = start + pos;
    rest = gap + minsize;
    end = start + newsize;

    if (p < ed->gap) {
      memcpy(start, ed->start, pos);
      memcpy(rest, p, ed->gap - p);
      memcpy(end - (ed->end - ed->rest), ed->rest, ed->end - ed->rest);
    } else {
      memcpy(start, ed->start, ed->gap - ed->start);
      memcpy(start + (ed->gap - ed->start), ed->rest, p - ed->rest);
      memcpy(rest, p, ed->end - p);
    }

    free(ed->start);
    ed->start = start;
    ed->gap = gap;
    ed->rest = rest;
    ed->end = end;
  }

#ifdef DEBUG
  memset(ed->gap, 0, ed->rest - ed->gap);
#endif
}

void close_gap(struct editor *ed) {
  int len = text_length(ed);
  move_gap(ed, len, 1);
  ed->start[len] = 0;
}

int copy(struct editor *ed, unsigned char *buf, int pos, int len) {
  unsigned char *bufptr = buf;
  unsigned char *p = ed->start + pos;
  if (p >= ed->gap) p += (ed->rest - ed->gap);

  while (len > 0) {
    if (p == ed->end) break;

    *bufptr++ = *p;
    len--;

    if (++p == ed->gap) p = ed->rest;
  }

  return bufptr - buf;
}

void replace(struct editor *ed, int pos, int len, unsigned char *buf, int bufsize) {
  unsigned char *p = ed->start + pos;

  // Handle deletions at the edges of the gap
  if (bufsize == 0 && p <= ed->gap && p + len >= ed->gap) {
    ed->rest += len - (ed->gap - p);
    ed->gap = p;
    return;
  }

  // Move the gap
  move_gap(ed, pos + len, bufsize - len);

  // Replace contents
  memcpy(ed->start + pos, buf, bufsize);
  ed->gap = ed->start + pos + bufsize;

  // Mark buffer as dirty
  ed->dirty = 1;
}

void insert(struct editor *ed, int pos, unsigned char *buf, int bufsize) {
  replace(ed, pos, 0, buf, bufsize);
}

void erase(struct editor *ed, int pos, int len) {
  replace(ed, pos, len, NULL, 0);
}

int get(struct editor *ed, int pos) {
  unsigned char *p = text_ptr(ed, pos);
  if (p >= ed->end) return -1;
  return *p;
}

//
// Navigation functions
//

int line_length(struct editor *ed, int linepos) {
  int pos = linepos;
  while (1) {
    int ch = get(ed, pos);
    if (ch < 0 || ch == '\n' || ch == '\r') break;
    pos++;
  }

  return pos - linepos;
}

int line_start(struct editor *ed, int pos) {
  while (1) {
    if (pos == 0) break;
    if (get(ed, pos - 1) == '\n') break;
    pos--;
  }

  return pos;
}

int next_line(struct editor *ed, int pos) {
  while (1) {
    int ch = get(ed, pos);
    if (ch < 0) return -1;
    pos++;
    if (ch == '\n') return pos;
  }
}

int prev_line(struct editor *ed, int pos) {
  if (pos == 0) return -1;

  while (pos > 0) {
    int ch = get(ed, --pos);
    if (ch == '\n') break;
  }

  while (pos > 0) {
    int ch = get(ed, --pos);
    if (ch == '\n') return pos + 1;
  }

  return 0;
}

int column(struct editor *ed, int linepos, int col) {
  unsigned char *p = text_ptr(ed, linepos);
  int c = 0;
  while (col > 0) {
    if (p == ed->end) break;
    if (*p == '\t') {
      int spaces = TABSIZE - c % TABSIZE;
      c += spaces;
    } else {
      c++;
    }
    col--;
    if (++p == ed->gap) p = ed->rest;
  }
  return c;
}

void moveto(struct editor *ed, int pos, int center) {
  int scroll = 0;
  for (;;) {
    int cur = ed->linepos + ed->col;
    if (pos < cur) {
      if (pos >= ed->linepos) {
        ed->col = pos - ed->linepos;
      } else {
        ed->col = 0;
        ed->linepos = prev_line(ed, ed->linepos);
        ed->line--;

        if (ed->topline > ed->line) {
          ed->toppos = ed->linepos;
          ed->topline--;
          ed->refresh = 1;
          scroll = 1;
        }
      }
    } else if (pos > cur) {
      int next = next_line(ed, ed->linepos);
      if (next == -1) pos = text_length(ed);
      if (pos < next) {
        ed->col = pos - ed->linepos;
      } else {
        ed->col = 0;
        ed->linepos = next;
        ed->line++;

        if (ed->line >= ed->topline + ed->lines) {
          ed->toppos = next_line(ed, ed->toppos);
          ed->topline++;
          ed->refresh = 1;
          scroll = 1;
        }
      }
    } else {
      break;
    }
  }
  
  if (scroll && center) {
    int tl = ed->line - ed->lines / 2;
    if (tl < 0) tl = 0;
    for (;;) {
      if (ed->topline > tl) {
        ed->toppos = prev_line(ed, ed->toppos);
        ed->topline--;
      } else if (ed->topline < tl) {
        ed->toppos = next_line(ed, ed->toppos);
        ed->topline++;
      } else {
        break;
      }
    }
  }
}

//
// Text selection
//

int get_selection(struct editor *ed, int *start, int *end) {
  if (ed->anchor == -1) {
    *start = *end = -1;
    return 0;
  } else {
    int pos = ed->linepos + ed->col;
    if (pos == ed->anchor) {
      *start = *end = -1;
      return 0;
    } else if (pos < ed->anchor) {
      *start = pos;
      *end = ed->anchor;
    } else {
      *start = ed->anchor;
      *end = pos;
    }
  }
  return 1;
}

void update_selection(struct editor *ed, int select) {
  if (select) {
    if (ed->anchor == -1) ed->anchor = ed->linepos + ed->col;
    ed->refresh = 1;
  } else {
    if (ed->anchor != -1) ed->refresh = 1;
    ed->anchor = -1;
  }
}

int erase_selection(struct editor *ed) {
  int selstart, selend;
  
  if (!get_selection(ed, &selstart, &selend)) return 0;
  moveto(ed, selstart, 0);
  erase(ed, selstart, selend - selstart);
  ed->anchor = -1;
  ed->refresh = 1;
  return 1;
}

void select_all(struct editor *ed) {
  ed->anchor = 0;
  ed->refresh = 1;
  moveto(ed, text_length(ed), 0);
}

//
// Screen functions
//

void get_console_size(struct editor *ed) {
#ifdef SANOS
  struct term *term = gettib()->proc->term;
  ed->cols = term->cols;
  ed->lines = term->lines - 1;
#else
  struct winsize ws;

  ioctl(0, TIOCGWINSZ, &ws);
  ed->cols = ws.ws_col;
  ed->lines = ws.ws_row - 1;
#endif
}

void outch(char c) {
  putchar(c);
}

void outbuf(unsigned char *buf, int len) {
  fwrite(buf, 1, len, stdout);
}

void outstr(char *str) {
  fputs(str, stdout);
}

void clear_screen() {
  outstr("\033[0J");
}

void gotoxy(int col, int line) {
  char buf[32];

  sprintf(buf, "\033[%d;%dH", line + 1, col + 1);
  outstr(buf);
}

//
// Keyboard functions
//

int getkey() {
  int ch, shift, ctrl;

  ch = getchar();
  if (ch < 0) return ch;

  switch (ch) {
    case 0x08: return KEY_BACKSPACE;
    case 0x09: return KEY_TAB;
#ifdef SANOS
    case 0x0D: return gettib()->proc->term->type == TERM_CONSOLE ? KEY_ENTER : KEY_UNKNOWN;
    case 0x0A: return gettib()->proc->term->type != TERM_CONSOLE ? KEY_ENTER : KEY_UNKNOWN;
#else
    case 0x0D: return KEY_ENTER;
    case 0x0A: return KEY_ENTER;
#endif
    case 0x1B:
      ch = getchar();
      switch (ch) {
        case 0x1B: return KEY_ESC;
        case 0x4F:
          ch = getchar();
          switch (ch) {
            case 0x46: return KEY_END;
            case 0x48: return KEY_HOME;
            default: return KEY_UNKNOWN;
          }
          break;
          
        case 0x5B:
          shift = ctrl = 0;
          ch = getchar();
          if (ch == 0x31) {
            ch = getchar();
            if (ch != 0x3B) return KEY_UNKNOWN;
            ch = getchar();
            if (ch == 0x35) ctrl = 1;
            if (ch == 0x32) shift = 1;
            ch = getchar();
          }

          switch (ch) {
            case 0x31: return getchar() == 0x7E ? KEY_HOME : KEY_UNKNOWN;
            case 0x32: return getchar() == 0x7E ? KEY_INS : KEY_UNKNOWN;
            case 0x33: return getchar() == 0x7E ? KEY_DEL : KEY_UNKNOWN;
            case 0x34: return getchar() == 0x7E ? KEY_END : KEY_UNKNOWN;
            case 0x35: return getchar() == 0x7E ? KEY_PGUP : KEY_UNKNOWN;
            case 0x36: return getchar() == 0x7E ? KEY_PGDN : KEY_UNKNOWN;

            case 0x41: return shift ? KEY_SHIFT_UP : ctrl ? KEY_CTRL_UP : KEY_UP;
            case 0x42: return shift ? KEY_SHIFT_DOWN : ctrl ? KEY_CTRL_DOWN : KEY_DOWN;
            case 0x43: return shift ? KEY_SHIFT_RIGHT : ctrl ? KEY_CTRL_RIGHT : KEY_RIGHT;
            case 0x44: return shift ? KEY_SHIFT_LEFT : ctrl ? KEY_CTRL_LEFT : KEY_LEFT;

            default: return KEY_UNKNOWN;
          }
          break;

        default: return KEY_UNKNOWN;
      }
      break;

    case 0x00:
    case 0xE0:
      ch = getchar();
      switch (ch) {
        case 0x3B: return KEY_F1;
        case 0x47: return KEY_HOME;
        case 0x48: return KEY_UP;
        case 0x49: return KEY_PGUP;
        case 0x4B: return KEY_LEFT;
        case 0x4D: return KEY_RIGHT;
        case 0x4F: return KEY_END;
        case 0x50: return KEY_DOWN;
        case 0x51: return KEY_PGDN;
        case 0x52: return KEY_INS;
        case 0x53: return KEY_DEL;
        case 0x73: return KEY_CTRL_LEFT;
        case 0x74: return KEY_CTRL_RIGHT;
        case 0x75: return KEY_CTRL_END;
        case 0x77: return KEY_CTRL_HOME;
        case 0x8D: return KEY_CTRL_UP;
        case 0x91: return KEY_CTRL_DOWN;
        case 0xB8: return KEY_SHIFT_UP;
        case 0xB9: return KEY_SHIFT_PGUP;
        case 0xBB: return KEY_SHIFT_LEFT;
        case 0xBD: return KEY_SHIFT_RIGHT;
        case 0xC0: return KEY_SHIFT_DOWN;
        case 0xC1: return KEY_SHIFT_PGDN;
        default: return KEY_UNKNOWN;
      }
      break;

    case 0x7F: return KEY_BACKSPACE;

    default: return ch;
  }
}

int prompt(struct editor *ed, char *msg) {
  int maxlen, len, ch, selstart, selend;
  char *buf = ed->linebuf;

  gotoxy(0, ed->lines);
  outstr("\033[1m");
  outstr(msg);
  outstr("\033[K\033[0m");
  
  len = 0;
  if (get_selection(ed, &selstart, &selend)) {
    int sellen = selend - selstart;
    if (sellen < maxlen) {
      copy(ed, buf, selstart, sellen);
      while (len < sellen && buf[len] >= ' ') len++;
      outbuf(buf, len);
    }
  }
  
  maxlen = ed->cols - strlen(msg) - 1;
  for (;;) {
    fflush(stdout);
    ch = getkey();
    if (ch == KEY_ESC) {
      return 0;
    } else if (ch == KEY_ENTER) {
      buf[len] = 0;
      return 1;
    } else if (ch == KEY_BACKSPACE) {
      if (len > 0) {
        outstr("\b \b");
        len--;
      }
    } else if (ch >= ' ' && ch < 0x100 && len < maxlen) {
      outch(ch);
      buf[len++] = ch;
    }
  }
}

//
// Display functions
//

void display_message(struct editor *ed, char *msg) {
  gotoxy(0, ed->lines);
  outstr("\033[1m");
  outstr(msg);
  outstr("\033[K\033[0m");
  fflush(stdout);
}

void draw_full_statusline(struct editor *ed) {
  int namewidth = ed->cols - 40;
  gotoxy(0, ed->lines);
  if (ed->minihelp) {
    sprintf(ed->linebuf, "\033[1m%*.*s Ctrl-S=Save Ctrl-Q=Quit F1=Help %s\033[K\033[0m", -namewidth, namewidth, ed->filename, ed->newfile ? "(NEW)" : "");
    ed->minihelp = 0;
  } else {
    sprintf(ed->linebuf, "\033[1m%*.*s%c Ln %-6d Col %-4d Pos %-10d\033[K\033[0m", -namewidth, namewidth, ed->filename, ed->dirty ? '*' : ' ', ed->line + 1, column(ed, ed->linepos, ed->col) + 1, ed->linepos + ed->col + 1);
  }
  outstr(ed->linebuf);
}

void draw_statusline(struct editor *ed) {
  gotoxy(ed->cols - 40, ed->lines);
  sprintf(ed->linebuf, "\033[1m%c Ln %-6d Col %-4d Pos %-10d\033[K\033[0m", ed->dirty ? '*' : ' ', ed->line + 1, column(ed, ed->linepos, ed->col) + 1, ed->linepos + ed->col + 1);
  outstr(ed->linebuf);
}

void display_line(struct editor *ed, int pos, int fullline) {
  int hilite = 0;
  int col = 0;
  int margin = ed->margin;
  int maxcol = ed->cols + margin;
  unsigned char *bufptr = ed->linebuf;
  unsigned char *p = text_ptr(ed, pos);
  int selstart, selend, ch;

  get_selection(ed, &selstart, &selend);
  while (col < maxcol) {
    if (margin == 0) {
      if (!hilite && pos >= selstart && pos < selend) {
        memcpy(bufptr, "\033[7m", 4);
        bufptr += 4;
        hilite = 1;
      } else if (hilite && pos >= selend) {
        memcpy(bufptr, "\033[0m", 4);
        bufptr += 4;
        hilite = 0;
      }
    }
    
    if (p == ed->end) break;
    ch = *p;
    if (ch == '\r' || ch == '\n') break;

    if (ch == '\t') {
      int spaces = TABSIZE - col % TABSIZE;
      while (spaces > 0 && col < maxcol) {
        if (margin > 0) {
          margin--;
        } else {
          *bufptr++ = ' ';
        }
        col++;
        spaces--;
      }
    } else {
      if (margin > 0) {
        margin--;
      } else {
        *bufptr++ = ch;
      }
      col++;
    }

    if (++p == ed->gap) p = ed->rest;
    pos++;
  }

#ifdef __linux__
  if (col == margin) *bufptr++ = ' ';
#endif

  if (col < maxcol) {
    memcpy(bufptr, "\033[K", 3);
    bufptr += 3;
    if (fullline) {
      memcpy(bufptr, "\r\n", 2);
      bufptr += 2;
    }
  }

  if (hilite) {
    memcpy(bufptr, "\033[0m", 4);
    bufptr += 4;
  }

  outbuf(ed->linebuf, bufptr - ed->linebuf);
}

void update_line(struct editor *ed) {
  gotoxy(0, ed->line - ed->topline);
  display_line(ed, ed->linepos, 0);
}

void draw_screen(struct editor *ed) {
  int pos;
  int i;

  gotoxy(0, 0);
  pos = ed->toppos;
  for (i = 0; i < ed->lines; i++) {
    if (pos < 0) {
      outstr("\033[K\r\n");
    } else {
      display_line(ed, pos, 1);
      pos = next_line(ed, pos);
    }
  }
}

void position_cursor(struct editor *ed) {
  int col = column(ed, ed->linepos, ed->col);
  gotoxy(col - ed->margin, ed->line - ed->topline);
}

//
// Cursor movement
//

void adjust(struct editor *ed) {
  int col;
  int ll = line_length(ed, ed->linepos);
  ed->col = ed->lastcol;
  if (ed->col > ll) ed->col = ll;

  col = column(ed, ed->linepos, ed->col);
  while (col < ed->margin) {
    ed->margin -= 4;
    if (ed->margin < 0) ed->margin = 0;
    ed->refresh = 1;
  }

  while (col - ed->margin >= ed->cols) {
    ed->margin += 4;
    ed->refresh = 1;
  }
}

void up(struct editor *ed, int select) {
  int newpos = prev_line(ed, ed->linepos);
  if (newpos < 0) return;

  update_selection(ed, select);

  ed->linepos = newpos;
  ed->line--;
  if (ed->line < ed->topline) {
    ed->toppos = ed->linepos;
    ed->topline = ed->line;
    ed->refresh = 1;
  }

  adjust(ed);
}

void down(struct editor *ed, int select)
{
  int newpos = next_line(ed, ed->linepos);
  if (newpos < 0) return;

  update_selection(ed, select);

  ed->linepos = newpos;
  ed->line++;

  if (ed->line >= ed->topline + ed->lines) {
    ed->toppos = next_line(ed, ed->toppos);
    ed->topline++;
    ed->refresh = 1;
  }

  adjust(ed);
}

void left(struct editor *ed, int select) {
  update_selection(ed, select);
  if (ed->col > 0) {
    ed->col--;
  } else {
    int newpos = prev_line(ed, ed->linepos);
    if (newpos < 0) return;

    ed->col = line_length(ed, newpos);
    ed->linepos = newpos;
    ed->line--;
    if (ed->line < ed->topline) {
      ed->toppos = ed->linepos;
      ed->topline = ed->line;
      ed->refresh = 1;
    }
  }

  ed->lastcol = ed->col;
  adjust(ed);
}

void right(struct editor *ed, int select) {
  update_selection(ed, select);
  if (ed->col < line_length(ed, ed->linepos)) {
    ed->col++;
  } else {
    int newpos = next_line(ed, ed->linepos);
    if (newpos < 0) return;

    ed->col = 0;
    ed->linepos = newpos;
    ed->line++;

    if (ed->line >= ed->topline + ed->lines) {
      ed->toppos = next_line(ed, ed->toppos);
      ed->topline++;
      ed->refresh = 1;
    }
  }

  ed->lastcol = ed->col;
  adjust(ed);
}

void home(struct editor *ed) {
  ed->col = ed->lastcol = 0;
  adjust(ed);
}

void end(struct editor *ed) {
  ed->col = ed->lastcol = line_length(ed, ed->linepos);
  adjust(ed);
}

void top(struct editor *ed) {
  ed->toppos = ed->topline = ed->margin = 0;
  ed->linepos = ed->line = ed->col = ed->lastcol = 0;
  ed->refresh = 1;
}

void bottom(struct editor *ed) {
  for (;;) {
    int newpos = next_line(ed, ed->linepos);
    if (newpos < 0) break;

    ed->linepos = newpos;
    ed->line++;

    if (ed->line >= ed->topline + ed->lines) {
      ed->toppos = next_line(ed, ed->toppos);
      ed->topline++;
      ed->refresh = 1;
    }
  }
  ed->col = ed->lastcol = line_length(ed, ed->linepos);
  adjust(ed);
}

void pageup(struct editor *ed, int select) {
  int i;

  update_selection(ed, select);
  if (ed->line < ed->lines) {
    ed->linepos = ed->toppos = 0;
    ed->line = ed->topline = 0;
  } else {
    for (i = 0; i < ed->lines; i++) {
      int newpos = prev_line(ed, ed->linepos);
      if (newpos < 0) return;

      ed->linepos = newpos;
      ed->line--;

      if (ed->topline > 0) {
        ed->toppos = prev_line(ed, ed->toppos);
        ed->topline--;
      }
    }
  }

  ed->refresh = 1;
  adjust(ed);
}

void pagedown(struct editor *ed, int select) {
  int i;

  update_selection(ed, select);
  for (i = 0; i < ed->lines; i++) {
    int newpos = next_line(ed, ed->linepos);
    if (newpos < 0) break;

    ed->linepos = newpos;
    ed->line++;

    ed->toppos = next_line(ed, ed->toppos);
    ed->topline++;
  }

  ed->refresh = 1;
  adjust(ed);
}

//
// Text editing
//

void insert_char(struct editor *ed, unsigned char ch) {
  erase_selection(ed);
  insert(ed, ed->linepos + ed->col, &ch, 1);
  ed->col++;
  ed->lastcol = ed->col;
  adjust(ed);
  if (!ed->refresh) ed->lineupdate = 1;
}

void newline(struct editor *ed) {
  int p;
  unsigned char ch;

  erase_selection(ed);
  insert(ed, ed->linepos + ed->col, "\r\n", 2);
  ed->col = ed->lastcol = 0;
  ed->line++;
  p = ed->linepos;
  ed->linepos = next_line(ed, ed->linepos);
  for (;;) {
    ch = get(ed, p++);
    if (ch == ' ' || ch == '\t') {
      insert(ed, ed->linepos + ed->col, &ch, 1);
      ed->col++;
    } else {
      break;
    }
  }
  ed->lastcol = ed->col;
  
  ed->refresh = 1;

  if (ed->line >= ed->topline + ed->lines) {
    ed->toppos = next_line(ed, ed->toppos);
    ed->topline++;
    ed->refresh = 1;
  }

  adjust(ed);
}

void backspace(struct editor *ed) {
  if (erase_selection(ed)) return;
  if (ed->linepos + ed->col == 0) return;
  if (ed->col == 0) {
    int pos = ed->linepos;
    erase(ed, --pos, 1);
    if (get(ed, pos - 1) == '\r') erase(ed, --pos, 1);

    ed->line--;
    ed->linepos = line_start(ed, pos);
    ed->col = pos - ed->linepos;
    ed->refresh = 1;

    if (ed->line < ed->topline) {
      ed->toppos = ed->linepos;
      ed->topline = ed->line;
    }
  } else {
    ed->col--;
    erase(ed, ed->linepos + ed->col, 1);
    ed->lineupdate = 1;
  }

  ed->lastcol = ed->col;
  adjust(ed);
}

void del(struct editor *ed) {
  int pos, ch;
  
  if (erase_selection(ed)) return;
  pos = ed->linepos + ed->col;
  ch = get(ed, pos);
  if (ch < 0) return;

  erase(ed, pos, 1);
  if (ch == '\r') {
    ch = get(ed, pos);
    if (ch == '\n') erase(ed, pos, 1);
  }

  if (ch == '\n') {
    ed->refresh = 1;
  } else {
    ed->lineupdate = 1;
  }
}

//
// Clipboard
//

void copy_selection(struct editor *ed) {
  int selstart, selend;

  if (!get_selection(ed, &selstart, &selend)) return;
  ed->clipsize = selend - selstart;
  ed->clipboard = (unsigned char *) realloc(ed->clipboard, ed->clipsize);
  copy(ed, ed->clipboard, selstart, ed->clipsize);
}

void cut_selection(struct editor *ed) {
  copy_selection(ed);
  erase_selection(ed);
}

void paste_selection(struct editor *ed) {
  erase_selection(ed);
  insert(ed, ed->linepos + ed->col, ed->clipboard, ed->clipsize);
  moveto(ed, ed->linepos + ed->col + ed->clipsize, 0);
  ed->refresh = 1;
}

//
// Editor Commands
//

void save(struct editor *ed) {
  int rc;

  if (!ed->dirty && !ed->newfile) return;
  rc = save_file(ed, NULL);
  if (rc < 0) {
    char msg[128];

    sprintf(msg, "Error %d saving document (%s)", errno, strerror(errno));
    display_message(ed, msg);
    sleep(5);
  }

  ed->refresh = 1;
}

void find(struct editor *ed, int next) {
  int slen;

  if (!next) {
    if (!prompt(ed, "Find: ")) {
      ed->refresh = 1;
      return;
    }    
    if (ed->search) free(ed->search);
    ed->search = strdup(ed->linebuf);
  }

  slen = strlen(ed->search);
  if (slen > 0) {
    unsigned char *match;
    
    close_gap(ed);
    match = strstr(ed->start + ed->linepos + ed->col, ed->search);
    if (match != NULL) {
      int pos = match - ed->start;
      ed->anchor = pos;
      moveto(ed, pos + slen, 1);
    } else {
      outch('\007');
    }
  }
  ed->refresh = 1;
}

int quit(struct editor *ed) {
  int ch;

  if (!ed->dirty) return 1;
  display_message(ed, "Quit without saving changes (y/n)? ");
  ch = getchar();
  if (ch == 'y' || ch == 'Y') return 1;
  ed->refresh = 1;
  return 0;
}

void redraw_screen(struct editor *ed) {
  get_console_size(ed);
  draw_screen(ed);
}

void help(struct editor *ed) {
  clear_screen();
  outstr("Editor Command Summary\r\n");
  outstr("======================\r\n\r\n");
  outstr("<up>        Move one line up (*)         F1      Help\r\n");
  outstr("<down>      Move one line down (*)       Ctrl+S  Save file\r\n");
  outstr("<left>      Move one character left (*)  Ctrl+Q  Quit\r\n");
  outstr("<right>     Move one character right (*) Ctrl+A  Select all\r\n");
  outstr("<pgup>      Move one page up (*)         Ctrl+L  Redraw screen\r\n");
  outstr("<pgdn>      Move one page down (*)       Ctrl+C  Copy selection to clipboard\r\n");
  outstr("<home>      Move to start of line        Ctrl+X  Cut selection to clipboard\r\n");
  outstr("<end>       Move to end of line          Ctrl+V  Paste selection from clipboard\r\n");
  outstr("Ctrl+<home> Move to start of file        Ctrl+F  Find text\r\n");
  outstr("Ctrl+<end>  Move to end of file          Ctrl+N  Find next\r\n");
  outstr("<backspace> Delete previous character\r\n");
  outstr("<delete>    Delete current character\r\n");
  outstr("\r\n(*) Extends selection if combined with Shift\r\n");
  outstr("\r\nPress any key to continue...");
  fflush(stdout);

  getkey();
  draw_screen(ed);
  draw_full_statusline(ed);
}

//
// Editor
//

void edit(struct editor *ed) {
  int done = 0;
  int key;

  ed->refresh = 1;
  ed->minihelp = 1;
  while (!done) {
    if (ed->refresh) {
      draw_screen(ed);
      draw_full_statusline(ed);
      ed->refresh = 0;
      ed->lineupdate = 0;
    } else if (ed->lineupdate) {
      update_line(ed);
      ed->lineupdate = 0;
      draw_statusline(ed);
    } else {
      draw_statusline(ed);
    }

    position_cursor(ed);
    fflush(stdout);
    key = getkey();

    if (key >= ' ' && key <= 0xFF) {
      insert_char(ed, (unsigned char) key);
    } else {
      switch (key) {
        case KEY_F1: help(ed); break;
        case KEY_UP: up(ed, 0); break;
        case KEY_DOWN: down(ed, 0); break;
        case KEY_LEFT: left(ed, 0); break;
        case KEY_RIGHT: right(ed, 0); break;
        case KEY_SHIFT_UP: up(ed, 1); break;
        case KEY_SHIFT_DOWN: down(ed, 1); break;
        case KEY_SHIFT_LEFT: left(ed, 1); break;
        case KEY_SHIFT_RIGHT: right(ed, 1); break;
        case KEY_HOME: home(ed); break;
        case KEY_END: end(ed); break;
        case KEY_CTRL_HOME: top(ed); break;
        case KEY_CTRL_END: bottom(ed); break;
        case KEY_PGUP: pageup(ed, 0); break;
        case KEY_PGDN: pagedown(ed, 0); break;
        case KEY_SHIFT_PGUP: pageup(ed, 1); break;
        case KEY_SHIFT_PGDN: pagedown(ed, 1); break;
        case KEY_ENTER: newline(ed); break;
        case KEY_BACKSPACE: backspace(ed); break;
        case KEY_DEL: del(ed); break;
        case KEY_TAB: insert_char(ed, '\t'); break;
        case ctrl('a'): select_all(ed); break;
        case ctrl('c'): copy_selection(ed); break;
        case ctrl('f'): find(ed, 0); break;
        case ctrl('n'): find(ed, 1); break;
        case ctrl('x'): cut_selection(ed); break;
        case ctrl('v'): paste_selection(ed); break;
        case ctrl('s'): save(ed); break;
        case ctrl('q'): if (quit(ed)) done = 1; break;
        case ctrl('l'): redraw_screen(ed); break;
      }
    }
  }
}

//
// main
//

int main(int argc, char *argv[]) {
  struct editor editbuf;
  struct editor *ed = &editbuf;
  int rc;
  sigset_t blocked_sigmask, orig_sigmask;
#ifdef __linux__
  struct termios tio;
  struct termios orig_tio;
#endif

  if (argc != 2) {
    printf("usage: edit <filename>\n");
    return 0;
  }
  
  rc = load_file(ed, argv[1]);
  if (rc < 0 && errno == ENOENT) rc = new_file(ed, argv[1]);
  if (rc < 0) {
    perror(argv[1]);
    return 0;
  }

  setvbuf(stdout, NULL, 0, 8192);

#ifdef __linux__
  tcgetattr(0, &orig_tio);
  cfmakeraw(&tio);  
  tcsetattr(0, TCSANOW, &tio);
  outstr("\033[3 q");  // xterm
  outstr("\033]50;CursorShape=2\a");  // KDE
#endif

  get_console_size(ed);
  ed->linebuf = malloc(ed->cols + LINEBUF_EXTRA);
  
  sigemptyset(&blocked_sigmask);
  sigaddset(&blocked_sigmask, SIGINT);
  sigaddset(&blocked_sigmask, SIGTSTP);
  sigaddset(&blocked_sigmask, SIGABRT);
  sigprocmask(SIG_BLOCK, &blocked_sigmask, &orig_sigmask);

  edit(ed);

  gotoxy(0, ed->lines + 1);
  outstr("\033[K");
#ifdef __linux__
  tcsetattr(0, TCSANOW, &orig_tio);   
#endif

  delete_editor(ed);

  setbuf(stdout, NULL);
  sigprocmask(SIG_SETMASK, &orig_sigmask, NULL);
  return 0;
}

