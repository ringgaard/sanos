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

#include <os.h>
#include <stdio.h>
#include <string.h>

#define MINEXTEND 32768

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
#define KEY_CTRL_RIGHT  0x111
#define KEY_CTRL_HOME   0x112
#define KEY_CTRL_END    0x113

#define KEY_UNKNOWN     0xFFF

#define CTRL(c) ((c) - 0x60)

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

struct editor
{
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

  int refresh;              // Flag to trigger screen redraw
  int lineupdate;           // Flag to trigger redraw of current line
  int dirty;                // Dirty flag is set when the editor buffer has been changed

  int minihelp;             // Flag to control mini help in status line
  int newfile;              // File is a new file

  int cols;
  int lines;
  char *linebuf;
  char filename[MAXPATH];
};

//
// Editor buffer functions
//

int new_file(struct editor *ed, char *filename)
{
  memset(ed, 0, sizeof(struct editor));
  strcpy(ed->filename, filename);

  ed->start = (unsigned char *) malloc(MINEXTEND);
  if (!ed->start) return -1;
#ifdef DEBUG
  memset(ed->start, 0, MINEXTEND);
#endif

  ed->gap = ed->start;
  ed->rest = ed->end = ed->gap + MINEXTEND;
  ed->newfile = 1;
  return 0;
}

int load_file(struct editor *ed, char *filename)
{
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

  close(f);
  return 0;

err:
  close(f);
  if (ed->start) free(ed->start);
  return -1;
}

int save_file(struct editor *ed, char *filename)
{
  int f;

  if (!filename) filename = ed->filename;

  f = open(filename, O_CREAT | O_TRUNC, 0644);
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

void delete_editor(struct editor *ed)
{
  if (ed->start) free(ed->start);
  if (ed->linebuf) free(ed->linebuf);
}

void move_gap(struct editor *ed, int pos, int minsize)
{
  int gapsize = ed->rest - ed->gap;
  unsigned char *p = ed->start + pos;
  if (p >= ed->gap) p += (ed->rest - ed->gap);
  if (minsize < 0) minsize = 0;

  if (minsize <= gapsize)
  {
    if (p != ed->rest)
    {
      if (p < ed->gap)
        memmove(p + gapsize, p, ed->gap - p);
      else
        memmove(ed->gap, ed->rest, p - ed->rest);

      ed->gap = ed->start + pos;
      ed->rest = ed->gap + gapsize;
    }
  }
  else
  {
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

    if (p < ed->gap)
    {
      memcpy(start, ed->start, pos);
      memcpy(rest, p, ed->gap - p);
      memcpy(end - (ed->end - ed->rest), ed->rest, ed->end - ed->rest);
    }
    else
    {
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

int copy(struct editor *ed, unsigned char *buf, int pos, int len)
{
  unsigned char *bufptr = buf;
  unsigned char *p = ed->start + pos;
  if (p >= ed->gap) p += (ed->rest - ed->gap);

  while (len > 0)
  {
    if (p == ed->end) break;

    *bufptr++ = *p;
    len--;

    if (++p == ed->gap) p = ed->rest;
  }

  return bufptr - buf;
}

void replace(struct editor *ed, int pos, int len, unsigned char *buf, int bufsize)
{
  unsigned char *p = ed->start + pos;

  // Handle deletions at the edges of the gap
  if (bufsize == 0 && p <= ed->gap && p + len >= ed->gap)
  {
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

void insert(struct editor *ed, int pos, unsigned char *buf, int bufsize)
{
  replace(ed, pos, 0, buf, bufsize);
}

void erase(struct editor *ed, int pos, int len)
{
  replace(ed, pos, len, NULL, 0);
}

__inline int get(struct editor *ed, int pos)
{
  unsigned char *p = ed->start + pos;
  if (p >= ed->gap) p += (ed->rest - ed->gap);
  if (p >= ed->end) return -1;
  return *p;
}

int copy_line(struct editor *ed, unsigned char *buf, int pos, int margin, int len)
{
  unsigned char *bufptr = buf;
  unsigned char *p = ed->start + pos;
  if (p >= ed->gap) p += (ed->rest - ed->gap);

  while (len > 0)
  {
    if (p == ed->end) break;
    if (*p == '\r' || *p == '\n') break;

    if (margin > 0)
      margin--;
    else
    {
      *bufptr++ = *p;
      len--;
    }

    if (++p == ed->gap) p = ed->rest;
  }

  return bufptr - buf;
}

//
// Navigation functions
//

int text_length(struct editor *ed)
{
  return (ed->gap - ed->start) + (ed->end - ed->rest);
}

int line_length(struct editor *ed, int linepos)
{
  int pos = linepos;
  while (1)
  {
    int ch = get(ed, pos);
    if (ch < 0 || ch == '\n' || ch == '\r') break;
    pos++;
  }

  return pos - linepos;
}

int line_start(struct editor *ed, int pos)
{
  while (1)
  {
    if (pos == 0) break;
    if (get(ed, pos - 1) == '\n') break;
    pos--;
  }

  return pos;
}

int next_line(struct editor *ed, int pos)
{
  while (1)
  {
    int ch = get(ed, pos);
    if (ch < 0) return -1;
    pos++;
    if (ch == '\n') return pos;
  }
}

int prev_line(struct editor *ed, int pos)
{
  if (pos == 0) return -1;

  while (pos > 0)
  {
    int ch = get(ed, --pos);
    if (ch == '\n') break;
  }

  while (pos > 0)
  {
    int ch = get(ed, --pos);
    if (ch == '\n') return pos + 1;
  }

  return 0;
}

//
// Keyboard functions
//

int getkey()
{
  int ch;

  ch = getchar();
  if (ch < 0) return ch;

  switch (ch)
  {
    case 0x08: return KEY_BACKSPACE;
    case 0x09: return KEY_TAB;
#ifdef SANOS
    case 0x0D: return gettib()->proc->term->type == TERM_CONSOLE ? KEY_ENTER : KEY_UNKNOWN;
    case 0x0A: return gettib()->proc->term->type != TERM_CONSOLE ? KEY_ENTER : KEY_UNKNOWN;
#else
    case 0x0D: return KEY_ENTER;
#endif
    case 0x1B:
      ch = getchar();
      switch (ch)
      {
        case 0x1B: return KEY_ESC;
        case 0x5B:
          ch = getchar();

          switch (ch)
          {
            case 0x31: return getchar() == 0x7E ? KEY_HOME : KEY_UNKNOWN;
            case 0x32: return getchar() == 0x7E ? KEY_INS : KEY_UNKNOWN;
            case 0x33: return getchar() == 0x7E ? KEY_DEL : KEY_UNKNOWN;
            case 0x34: return getchar() == 0x7E ? KEY_END : KEY_UNKNOWN;
            case 0x35: return getchar() == 0x7E ? KEY_PGUP : KEY_UNKNOWN;
            case 0x36: return getchar() == 0x7E ? KEY_PGDN : KEY_UNKNOWN;

            case 0x41: return KEY_UP;
            case 0x42: return KEY_DOWN;
            case 0x43: return KEY_RIGHT;
            case 0x44: return KEY_LEFT;

            default: return KEY_UNKNOWN;
          }
          break;

        default: return KEY_UNKNOWN;
      }
      break;

    case 0x00:
    case 0xE0:
      ch = getchar();
      switch (ch)
      {
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
        default: return KEY_UNKNOWN;
      }
      break;

    case 0x7F: return KEY_BACKSPACE;

    default: return ch;
  }
}

//
// Screen functions
//

void outch(char c)
{
  putchar(c);
}

void outbuf(char *buf, int len)
{
  fwrite(buf, 1, len, stdout);
}

void outstr(char *str)
{
  fputs(str, stdout);
}

void clear_screen()
{
  outstr("\033[0J");
}

void gotoxy(int col, int line)
{
  char buf[32];

  sprintf(buf, "\033[%d;%dH", line + 1, col + 1);
  outstr(buf);
}

//
// Display functions
//

void display_message(struct editor *ed, char *msg)
{
  gotoxy(0, ed->lines);
  outstr("\033[1m");
  outstr(msg);
  outstr("\033[K\033[0m");
  fflush(stdout);
}

void draw_full_statusline(struct editor *ed)
{
  char buf[128];

  gotoxy(0, ed->lines);
  if (ed->minihelp)
  {
    sprintf(buf, "\033[1m%-44.44s Ctrl-S=Save Ctrl-Q=Quit %s\033[K\033[0m", ed->filename, ed->newfile ? "(NEW)" : "");
    ed->minihelp = 0;
  }
  else
    sprintf(buf, "\033[1m%-44.44s%c Ln %-6d Col %-4d Pos %-10d\033[K\033[0m", ed->filename, ed->dirty ? '*' : ' ', ed->line + 1, ed->col + 1, ed->linepos + ed->col + 1);
  outstr(buf);
}

void draw_statusline(struct editor *ed)
{
  char buf[128];

  gotoxy(44, ed->lines);
  sprintf(buf, "\033[1m%c Ln %-6d Col %-4d Pos %-10d\033[K\033[0m", ed->dirty ? '*' : ' ', ed->line + 1, ed->col + 1, ed->linepos + ed->col + 1);
  outstr(buf);
}

void update_line(struct editor *ed, int col)
{
  int len;

  len = copy_line(ed, ed->linebuf, ed->linepos, ed->margin + col, ed->cols - col);
  gotoxy(col, ed->line - ed->topline);
  outbuf(ed->linebuf, len);
  if (len + col < ed->cols) outstr("\033[K");
}

void draw_screen(struct editor *ed)
{
  int pos;
  int i;

  gotoxy(0, 0);
  pos = ed->toppos;
  for (i = 0; i < ed->lines; i++)
  {
    int len;

    if (pos < 0) 
    {
      outstr("\033[K\r\n");
    }
    else
    {
      len = copy_line(ed, ed->linebuf, pos, ed->margin, ed->cols);
      outbuf(ed->linebuf, len);
      if (len < ed->cols) outstr("\033[K\r\n");
      pos = next_line(ed, pos);
    }
  }
}

void position_cursor(struct editor *ed)
{
  gotoxy(ed->col - ed->margin, ed->line - ed->topline);
}

//
// Cursor movement
//

void adjust(struct editor *ed)
{
  int ll = line_length(ed, ed->linepos);
  ed->col = ed->lastcol;
  if (ed->col > ll) ed->col = ll;

  while (ed->col < ed->margin)
  {
    ed->margin -= 4;
    if (ed->margin < 0) ed->margin = 0;
    ed->refresh = 1;
  }

  while (ed->col - ed->margin >= ed->cols) 
  {
    ed->margin += 4;
    ed->refresh = 1;
  }
}

void up(struct editor *ed)
{
  int newpos = prev_line(ed, ed->linepos);
  if (newpos < 0) return;

  ed->linepos = newpos;
  ed->line--;
  if (ed->line < ed->topline)
  {
    ed->toppos = ed->linepos;
    ed->topline = ed->line;
    ed->refresh = 1;
  }

  adjust(ed);
}

void down(struct editor *ed)
{
  int newpos = next_line(ed, ed->linepos);
  if (newpos < 0) return;

  ed->linepos = newpos;
  ed->line++;

  if (ed->line >= ed->topline + ed->lines)
  {
    ed->toppos = next_line(ed, ed->toppos);
    ed->topline++;
    ed->refresh = 1;
  }

  adjust(ed);
}

void left(struct editor *ed)
{
  if (ed->col > 0) 
    ed->col--;
  else
  {
    int newpos = prev_line(ed, ed->linepos);
    if (newpos < 0) return;

    ed->col = line_length(ed, newpos);
    ed->linepos = newpos;
    ed->line--;
    if (ed->line < ed->topline)
    {
      ed->toppos = ed->linepos;
      ed->topline = ed->line;
      ed->refresh = 1;
    }
  }

  ed->lastcol = ed->col;
  adjust(ed);
}

void right(struct editor *ed)
{
  if (ed->col < line_length(ed, ed->linepos))
    ed->col++;
  else
  {
    int newpos = next_line(ed, ed->linepos);
    if (newpos < 0) return;

    ed->col = 0;
    ed->linepos = newpos;
    ed->line++;

    if (ed->line >= ed->topline + ed->lines)
    {
      ed->toppos = next_line(ed, ed->toppos);
      ed->topline++;
      ed->refresh = 1;
    }
  }

  ed->lastcol = ed->col;
  adjust(ed);
}

void home(struct editor *ed)
{
  ed->col = ed->lastcol = 0;
  adjust(ed);
}

void end(struct editor *ed)
{
  ed->col = ed->lastcol = line_length(ed, ed->linepos);
  adjust(ed);
}

void top(struct editor *ed)
{
  ed->toppos = ed->topline = ed->margin = 0;
  ed->linepos = ed->line = ed->col = ed->lastcol = 0;
  ed->refresh = 1;
}

void bottom(struct editor *ed)
{
  for (;;) {
    int newpos = next_line(ed, ed->linepos);
    if (newpos < 0) break;

    ed->linepos = newpos;
    ed->line++;

    if (ed->line >= ed->topline + ed->lines)
    {
      ed->toppos = next_line(ed, ed->toppos);
      ed->topline++;
      ed->refresh = 1;
    }
  }
  ed->col = ed->lastcol = line_length(ed, ed->linepos);
  adjust(ed);
}

void pageup(struct editor *ed)
{
  int i;

  if (ed->line < ed->lines)
  {
    ed->linepos = ed->toppos = 0;
    ed->line = ed->topline = 0;
  }
  else
  {
    for (i = 0; i < ed->lines; i++)
    {
      int newpos = prev_line(ed, ed->linepos);
      if (newpos < 0) return;

      ed->linepos = newpos;
      ed->line--;

      if (ed->topline > 0)
      {
        ed->toppos = prev_line(ed, ed->toppos);
        ed->topline--;
      }
    }
  }

  ed->refresh = 1;
  adjust(ed);
}

void pagedown(struct editor *ed)
{
  int i;

  for (i = 0; i < ed->lines; i++)
  {
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

void insert_char(struct editor *ed, unsigned char ch)
{
  insert(ed, ed->linepos + ed->col, &ch, 1);
  ed->col++;
  ed->lastcol = ed->col;
  adjust(ed);
  if (!ed->refresh) update_line(ed, ed->col - ed->margin - 1);
}

void newline(struct editor *ed)
{
  insert(ed, ed->linepos + ed->col, "\r\n", 2);
  ed->col = ed->lastcol = 0;
  ed->line++;
  ed->linepos = next_line(ed, ed->linepos);
  ed->lastcol = ed->col;
  ed->refresh = 1;

  if (ed->line >= ed->topline + ed->lines)
  {
    ed->toppos = next_line(ed, ed->toppos);
    ed->topline++;
    ed->refresh = 1;
  }

  adjust(ed);
}

void backspace(struct editor *ed)
{
  if (ed->linepos + ed->col == 0) return;
  if (ed->col == 0)
  {
    int pos = ed->linepos;
    erase(ed, --pos, 1);
    if (get(ed, pos - 1) == '\r') erase(ed, --pos, 1);

    ed->line--;
    ed->linepos = line_start(ed, pos);
    ed->col = pos - ed->linepos;
    ed->refresh = 1;

    if (ed->line < ed->topline)
    {
      ed->toppos = ed->linepos;
      ed->topline = ed->line;
    }
  }
  else
  {
    ed->col--;
    erase(ed, ed->linepos + ed->col, 1);
    ed->lineupdate = 1;
  }

  ed->lastcol = ed->col;
  adjust(ed);
}

void del(struct editor *ed)
{
  int pos = ed->linepos + ed->col;
  int ch = get(ed, pos);
  if (ch < 0) return;

  erase(ed, pos, 1);
  if (ch == '\r')
  {
    ch = get(ed, pos);
    if (ch == '\n') erase(ed, pos, 1);
  }

  if (ch == '\n')
    ed->refresh = 1;
  else
    ed->lineupdate = 1;
}

//
// Editor Commands
//

void save(struct editor *ed)
{
  int rc;

  if (!ed->dirty && !ed->newfile) return;
  rc = save_file(ed, NULL);
  if (rc < 0)
  {
    char msg[128];

    sprintf(msg, "Error %d saving document (%s)", errno, strerror(errno));
    display_message(ed, msg);
    sleep(5);
  }

  ed->refresh = 1;
}

int quit(struct editor *ed)
{
  int ch;

  if (!ed->dirty) return 1;
  display_message(ed, "Quit without saving changes (y/n)? ");
  ch = getchar();
  if (ch == 'y' || ch == 'Y') return 1;
  ed->refresh = 1;
  return 0;
}

void detab(struct editor *ed)
{
  int pos, linepos, tabs, dirty;
  unsigned char *p;
  
  tabs = 0;
  p = ed->start;
  while (p < ed->end)
  {
    if (p == ed->gap) 
    {
      p = ed->rest;
      continue;
    }
    if (*p == '\t') {
      tabs = 1;
      break;
    }
    if (++p == ed->gap) p = ed->rest;
  }
  if (!tabs) return;

  dirty = ed->dirty;
  linepos = 0;
  pos = 0;
  for (;;)
  {
    int ch = get(ed, pos);
    if (ch < 0) break;
    if (ch == '\t')
    {
      int spaces = 8 - linepos % 8;
      replace(ed, pos, 1, "        ", spaces);
    }
    pos++;
  }
  ed->dirty = dirty;
}

//
// main
//

int main(int argc, char *argv[])
{
  struct editor editbuf;
  struct editor *ed = &editbuf;
  int rc;
  int key;
  int done;
  struct term *term;

  if (argc != 2)
  {
    printf("usage: edit <filename>\n");
    return 0;
  }
  
  rc = load_file(ed, argv[1]);
  if (rc < 0 && errno == ENOENT) rc = new_file(ed, argv[1]);
  if (rc < 0) 
  {
    perror(argv[1]);
    return 0;
  }
  detab(ed);

  setvbuf(stdout, NULL, 0, 8192);

  term = gettib()->proc->term;
  ed->cols = term->cols;
  ed->lines = term->lines - 1;
  ed->linebuf = malloc(ed->cols);

  ed->refresh = 1;
  ed->minihelp = 1;
  done = 0;
  while (!done)
  {
    if (ed->refresh)
    {
      draw_screen(ed);
      draw_full_statusline(ed);
      ed->refresh = 0;
      ed->lineupdate = 0;
    }
    else if (ed->lineupdate)
    {
      update_line(ed, 0);
      ed->lineupdate = 0;
      draw_statusline(ed);
    }
    else
      draw_statusline(ed);

    position_cursor(ed);
    fflush(stdout);
    key = getkey();

    switch (key)
    {
      case KEY_UP: up(ed); break;
      case KEY_DOWN: down(ed); break;
      case KEY_LEFT: left(ed); break;
      case KEY_RIGHT: right(ed); break;
      case KEY_HOME: home(ed); break;
      case KEY_END: end(ed); break;
      case KEY_CTRL_HOME: top(ed); break;
      case KEY_CTRL_END: bottom(ed); break;
      case KEY_PGUP: pageup(ed); break;
      case KEY_PGDN: pagedown(ed); break;
      case KEY_ENTER: newline(ed); break;
      case KEY_BACKSPACE: backspace(ed); break;
      case KEY_DEL: del(ed); break;
      case CTRL('s'): save(ed); break;
      case CTRL('q'): if (quit(ed)) done = 1; break;
      case CTRL('l'): 
        ed->cols = term->cols;
        ed->lines = term->lines - 1;
        ed->linebuf = realloc(ed->linebuf, ed->cols);
        draw_screen(ed);
        break;

      default: 
        if (key >= ' ' && key <= 0xFF) insert_char(ed, (unsigned char) key);
    }
  }

  delete_editor(ed);

  gotoxy(0, term->lines);
  outstr("\033[K");
  setbuf(stdout, NULL);
  return 0;
}
