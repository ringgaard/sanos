//
// inifile.c
//
// Property files
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

#ifdef KERNEL
#include <os/krnl.h>
#include <stdlib.h>
#else
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inifile.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

static char *trimstr(char *s, char *end) {
  char *str;
  char *t;
  int ch;
  int i;
  
  while (end > s) {
    if (*(end - 1) == ' ' || *(end - 1) == '\t') {
      end--;
    } else {
      break;
    }
  }
  if (end == s) return NULL;

  t = str = (char *) malloc(end - s + 1);
  while (s < end) {
    if (*s == '^') {
      s++;
      ch = 0;
      for (i = 0; i < 2; i++) {
        if (s == end) break;

        if (*s >= '0' && *s <= '9') {
          ch = (ch << 4) + *s - '0';
        } else if (*s >= 'A' && *s <= 'F') {
          ch = (ch << 4) + *s + 10 - 'A';
        } else if (*s >= 'a' && *s <= 'f') {
          ch = (ch << 4) + *s + 10 - 'a';
        } else {
          break;
        }

        s++;
      }
      *t++ = ch;
    } else {
      *t++ = *s++;
    }
  }

  *t = 0;
  return str;
}

struct section *find_section(struct section *sect, char *name) {
  while (sect) {
    if (strcmp(sect->name, name) == 0) return sect;
    sect = sect->next;
  }

  return NULL;
}

int get_section_size(struct section *sect) {
  struct property *prop;
  int n;
  
  if (!sect) return 0;
  prop = sect->properties;

  n = 0;
  while (prop) {
    n++;
    prop = prop->next;
  }

  return n;
}

char *find_property(struct section *sect, char *name) {
  struct property *prop;
  
  if (!sect) return NULL;
  prop = sect->properties;

  while (prop) {
    if (strcmp(prop->name, name) == 0) return prop->value ? prop->value : "";
    prop = prop->next;
  }

  return NULL;
}

char *get_property(struct section *sections, char *sectname, char *propname, char *defval) {
  struct section *sect;
  char *val;

  sect = find_section(sections, sectname);
  if (!sect) return defval;

  val = find_property(sect, propname);
  return val ? val : defval;
}

int get_numeric_property(struct section *sections, char *sectname, char *propname, int defval) {
  char *val;

  val = get_property(sections, sectname, propname, NULL);
  return val ? atoi(val) : defval;
}

void free_properties(struct section *sect) {
  struct section *nextsect;
  struct property *prop;
  struct property *nextprop;

  while (sect) {
    if (sect->name) free(sect->name);
    
    prop = sect->properties;
    while (prop) {
      if (prop->name) free(prop->name);
      if (prop->value) free(prop->value);
      
      nextprop = prop->next;
      free(prop);
      prop = nextprop;
    }

    nextsect = sect->next;
    free(sect);
    sect = nextsect;
  }
}

struct section *parse_properties(char *props) {
  struct section *secthead = NULL;
  struct section *sect = NULL;
  struct property *prop = NULL;
  char *p;
  char *end;
  char *split;

  p = props;
  while (*p) {
    // Skip white at start of line
    while (*p == ' ' || *p == '\t') p++;
    
    // Skip comments
    if (*p == '#' || *p == ';') {
      while (*p && *p != '\r' && *p != '\n') p++;
      if (*p == '\r') p++;
      if (*p == '\n') p++;
      continue;
    }

    // Skip blank lines
    if (*p == 0 || *p == '\r' || *p == '\n') {
      if (*p == '\r') p++;
      if (*p == '\n') p++;
      continue;
    }

    // Check for section or property
    if (*p == '[') {
      struct section *newsect;

      p++;
      end = p;
      while (*end && *end != ']') end++;

      newsect = (struct section *) malloc(sizeof(struct section));
      if (!newsect) return NULL;

      newsect->name = trimstr(p, end);
      newsect->next = NULL;
      newsect->properties = NULL;
      if (!secthead) secthead = newsect;
      if (sect) sect->next = newsect;
      sect = newsect;
      prop = NULL;

      p = end;
      if (*p == ']') p++;
    } else {
      struct property *newprop;

      end = p;
      split = NULL;
      while (*end && *end != '\r' && *end != '\n') {
        if (!split && (*end == '=' || *end == ':')) split = end;
        end++;
      }

      if (sect) {
        newprop = (struct property *) malloc(sizeof(struct property));
        if (!newprop) return NULL;

        if (split) {
          newprop->name = trimstr(p, split);
          split++;
          while (*split == ' ' || *split == '\t') split++;
          newprop->value = trimstr(split, end);
        } else {
          newprop->name = trimstr(p, end);
          newprop->value = NULL;
        }

        newprop->next = NULL;
        if (prop) {
          prop->next = newprop;
        } else {
          sect->properties = newprop;
        }

        prop = newprop;
      }

      p = end;
      if (*p == '\r') p++;
      if (*p == '\n') p++;
    }
  }

  return secthead;
}

#ifndef KERNEL

void list_properties(int f, struct section *sect) {
  struct property *prop;

  while (sect) {
    write(f, "[", 1);
    write(f, sect->name, strlen(sect->name));
    write(f, "]\r\n", 3);

    prop = sect->properties;
    while (prop) {
      write(f, prop->name, strlen(prop->name));
      if (prop->value) {
        write(f, "=", 1);
        write(f, prop->value, strlen(prop->value));
      }
      write(f, "\r\n", 2);
      prop = prop->next;
    }
    
    if (sect->next) write(f, "\r\n", 2);
    sect = sect->next;
  }
}

struct section *read_properties(char *filename) {
  int f;
  int size;
  struct stat buffer;
  char *props;
  struct section *sect;

  f = open(filename, O_BINARY);
  if (f < 0) return NULL;

  if (fstat(f, &buffer) < 0) {
    close(f);
    return NULL;
  }
  size = (int) buffer.st_size;

  props = (char *) malloc(size + 1);
  if (!props) {
    close(f);
    return NULL;
  }

  if (read(f, props, size) != size) {
    free(props);
    close(f);
    return NULL;
  }

  close(f);

  props[size] = 0;

  sect = parse_properties(props);
  free(props);

  return sect;
}

#endif
