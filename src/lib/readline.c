//
// readline.c
//
// Read line with editing
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

#define MAX_HISTORY     64

#define KEY_BACKSPACE   0x101
#define KEY_ESC         0x102
#define KEY_INS         0x103
#define KEY_DEL         0x104
#define KEY_LEFT        0x105
#define KEY_RIGHT       0x106
#define KEY_UP          0x107
#define KEY_DOWN        0x108
#define KEY_CTRL_LEFT   0x109
#define KEY_CTRL_RIGHT  0x10A
#define KEY_HOME        0x10B
#define KEY_END         0x10C
#define KEY_ENTER       0x10D
#define KEY_TAB         0x10E
#define KEY_EOF         0x10F

#define KEY_UNKNOWN     0xFFF

static int insmode = 1;
static int history_len = 0;
static char *history[MAX_HISTORY];
int _break_on_escape = 0;

void add_to_history(char *line) {
  int i;

  if (!line || !*line) return;
  if (history_len > 0 && strcmp(line, history[history_len - 1]) == 0) return;

  if (history_len == MAX_HISTORY) {
    free(history[0]);
    for (i = 0; i < history_len - 1; i++) history[i] = history[i + 1];
    history_len--;
  }

  history[history_len] = malloc(strlen(line) + 1);
  if (history[history_len]) {
    strcpy(history[history_len], line);
    history_len++;
  }
}

static int like(char *str, char *mask) {
  if (!str) str = "";
  if (!mask) mask = "";

  while (*mask) {
    if (*mask == '*') {
      while (*mask == '*') mask++;
      if (*mask == 0) return 1;
      while (*str) {
        if (like(str, mask)) return 1;
        str++;
      }
      return 0;
    } else if (*mask == *str || *mask == '?' && *str != 0) {
      str++;
      mask++;
    } else {
      return 0;
    }
  }

  return *str == 0;
}

static int delimchar(int ch) {
  if (ch == ' ' || ch == ',' || ch == ';') return 1;
  return 0;
}

static int find_dir(char *buf, int start, int end, int split, char *mask) {
  char path[MAXPATH];
  int wildcards;
  int idx;
  int dir;
  
  if (start == split) {
    path[0] = '.';
    path[1] = 0;
  } else {
    memcpy(path, buf + start, split - start);
    path[split - start] = 0;
  }

  dir = _opendir(path);
  if (dir < 0) return dir;

  if (split == end) {
    *mask++ = '*';
  } else {
    wildcards = 0;
    idx = split;
    while (idx < end) {
      if (buf[idx] == '*' || buf[idx] == '?') wildcards = 1;
      *mask++ = buf[idx++];
    }

    if (!wildcards) *mask++ = '*';
  }

  *mask++ = 0;
  return dir;
}

static int getkey() {
  int ch;

  ch = getchar();
  if (ch < 0) return ch;

  switch (ch) {
    case 0x08: return KEY_BACKSPACE;
    case 0x09: return KEY_TAB;
    case 0x0D: return gettib()->proc->term->type == TERM_CONSOLE ? KEY_ENTER : KEY_UNKNOWN;
    case 0x0A: return gettib()->proc->term->type != TERM_CONSOLE ? KEY_ENTER : KEY_UNKNOWN;
    case 0x04: return KEY_EOF;

    case 0x1B:
      ch = getchar();
      switch (ch) {
        case 0x1B: return KEY_ESC;
        case 0x5B:
          ch = getchar();
          switch (ch) {
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
      switch (ch) {
        case 0x47: return KEY_HOME;
        case 0x48: return KEY_UP;
        case 0x4B: return KEY_LEFT;
        case 0x4D: return KEY_RIGHT;
        case 0x4F: return KEY_END;
        case 0x50: return KEY_DOWN;
        case 0x52: return KEY_INS;
        case 0x53: return KEY_DEL;
        case 0x73: return KEY_CTRL_LEFT;
        case 0x74: return KEY_CTRL_RIGHT;
        default: return KEY_UNKNOWN;
      }
      break;

    case 0x7F: return KEY_BACKSPACE;

    default: return ch;
  }
}

int readline(char *buf, int size) {
  int idx;
  int len;
  int key;
  int i;
  int done;
  int hist_idx;
  int dir;

  if (size <= 0) {
    errno = EINVAL;
    return -1;
  }

  idx = 0;
  len = 0;
  done = 0;
  hist_idx = history_len;
  while (!done) {
    fflush(stdout);
    key = getkey();
    if (key < 0) return key;

    if (key == KEY_TAB) {
      int start;
      int end;
      int split;
      char mask[MAXPATH];
      struct direntry dirent;

      start = idx;
      while (start > 0 && !delimchar(buf[start - 1])) start--;
      end = split = start;
      while (end < len && !delimchar(buf[end])) {
        if (buf[end] == PS1 || buf[end] == PS2) split = end + 1;
        end++;
      }

      dir = find_dir(buf, start, end, split, mask);
      if (dir >= 0) {
        while (_readdir(dir, &dirent, 1) > 0) {
          int newlen = len - (end - split) + dirent.namelen;
          
          if (like(dirent.name, mask) && newlen < size - 1) {
            memmove(buf + split + dirent.namelen, buf + end, len - end);
            memcpy(buf + split, dirent.name, dirent.namelen);

            while (idx < split) putchar(buf[idx++]);
            while (idx > split)  {
              putchar('\b');
              idx--;
            }

            for (i = split; i < newlen; i++) putchar(buf[i]);
            if (newlen < len) {
              for (i = newlen; i < len; i++) putchar(' ');
              for (i = newlen; i < len; i++) putchar('\b');
            }

            end = split + dirent.namelen;
            len = newlen;
            idx = end;

            for (i = end; i < len; i++) putchar('\b');

            fflush(stdout);
            key = getkey();
            if (key < 0) break;
            if (key != KEY_TAB) break;
          }
        }
        close(dir);
        if (key < 0) return key;
      }
    }

    switch (key) {
      case KEY_LEFT:
        if (idx > 0) {
          putchar('\b');
          idx--;
        }
        break;

      case KEY_RIGHT:
        if (idx < len) {
          putchar(buf[idx]);
          idx++;
        }
        break;

      case KEY_CTRL_LEFT:
        if (idx > 0) {
          putchar('\b');
          idx--;
        }
        while (idx > 0 && buf[idx - 1] != ' ') {
          putchar('\b');
          idx--;
        }
        break;

      case KEY_CTRL_RIGHT:
        while (idx < len && buf[idx] != ' ') {
          putchar(buf[idx]);
          idx++;
        }
        if (idx < len) {
          putchar(buf[idx]);
          idx++;
        }
        break;

      case KEY_HOME:
        while (idx > 0) {
          putchar('\b');
          idx--;
        }
        break;

      case KEY_END:
        while (idx < len) {
          putchar(buf[idx]);
          idx++;
        }
        break;

      case KEY_DEL:
        if (idx < len) {
          len--;
          memmove(buf + idx, buf + idx + 1, len - idx);
          for (i = idx; i < len; i++) putchar(buf[i]);
          putchar(' ');
          putchar('\b');
          for (i = idx; i < len; i++) putchar('\b');
        }
        break;

      case KEY_INS:
        insmode = !insmode;
        break;

      case KEY_BACKSPACE:
        if (idx > 0) {
          putchar('\b');
          idx--;
          len--;
          memmove(buf + idx, buf + idx + 1, len - idx);
          for (i = idx; i < len; i++) putchar(buf[i]);
          putchar(' ');
          putchar('\b');
          for (i = idx; i < len; i++) putchar('\b');
        }
        break;

      case KEY_ESC:
        if (_break_on_escape) {
          buf[len] = 0;
          errno = EINTR;
          return -1;
        } else {
          for (i = 0; i < idx; i++) putchar('\b');
          for (i = 0; i < len; i++) putchar(' ');
          for (i = 0; i < len; i++) putchar('\b');
          idx = len = 0;
        }
        break;

      case KEY_EOF:
        if (len == 0) return -1;
        break;

      case KEY_ENTER:
        putchar('\r');
        putchar('\n');
        done = 1;
        break;

      case KEY_UP:
        if (hist_idx > 0) {
          hist_idx--;
          for (i = 0; i < idx; i++) putchar('\b');
          for (i = 0; i < len; i++) putchar(' ');
          for (i = 0; i < len; i++) putchar('\b');
          len = strlen(history[hist_idx]);
          if (len > size - 1) len = size - 1;
          idx = len;
          memcpy(buf, history[hist_idx], len);
          for (i = 0; i < len; i++) putchar(buf[i]);
        }
        break;

      case KEY_DOWN:
        if (hist_idx < history_len - 1) {
          hist_idx++;
          for (i = 0; i < idx; i++) putchar('\b');
          for (i = 0; i < len; i++) putchar(' ');
          for (i = 0; i < len; i++) putchar('\b');
          len = strlen(history[hist_idx]);
          if (len > size - 1) len = size - 1;
          idx = len;
          memcpy(buf, history[hist_idx], len);
          for (i = 0; i < len; i++) putchar(buf[i]);
        }
        break;

      case KEY_UNKNOWN:
        break;

      default:
        if (key >= 0x20 && key <= 0xFF) {
          if (insmode) {
            if (len < size - 1) {
              if (idx < len) memmove(buf + idx + 1, buf + idx, len - idx);
              buf[idx] = key;
              len++;
              for (i = idx; i < len; i++) putchar(buf[i]);
              idx++;
              for (i = idx; i < len; i++) putchar('\b');
            }
          } else {
            if (idx < size - 1) {
              buf[idx] = key;
              putchar(buf[idx]);
              if (idx == len) len++;
              idx++;
            }
          }
        }
    }
  }

  buf[len] = 0;

  add_to_history(buf);

  return len;
}
