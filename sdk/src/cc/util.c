//
//  util.c - Tiny C Compiler for Sanos
// 
//  Copyright (c) 2001-2004 Fabrice Bellard
//  Copyright (c) 2011-2012 Michael Ringgaard
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "cc.h"

// True if isid(c) || isnum(c)
static unsigned char isidnum_table[256];

// Memory management
#ifdef MEM_DEBUG
int mem_cur_size;
int mem_max_size;
#endif

void *tcc_malloc(unsigned long size) {
  void *ptr;
  ptr = malloc(size);
  if (!ptr && size) error("memory full");
#ifdef MEM_DEBUG
  mem_cur_size += malloc_usable_size(ptr);
  if (mem_cur_size > mem_max_size) mem_max_size = mem_cur_size;
#endif
  return ptr;
}

void *tcc_mallocz(unsigned long size) {
  void *ptr;
  ptr = tcc_malloc(size);
  memset(ptr, 0, size);
  return ptr;
}

void *tcc_realloc(void *ptr, unsigned long size) {
  void *newptr;
#ifdef MEM_DEBUG
  mem_cur_size -= malloc_usable_size(ptr);
#endif
  newptr = realloc(ptr, size);
#ifdef MEM_DEBUG
  // NOTE: count not correct if alloc error, but not critical
  mem_cur_size += malloc_usable_size(newptr);
  if (mem_cur_size > mem_max_size) cmem_max_size = mem_cur_size;
#endif
  return newptr;
}

char *tcc_strdup(const char *str) {
  char *ptr;
  ptr = tcc_malloc(strlen(str) + 1);
  strcpy(ptr, str);
  return ptr;
}

void tcc_free(void *ptr) {
#ifdef MEM_DEBUG
  mem_cur_size -= malloc_usable_size(ptr);
#endif
  free(ptr);
}

// Extract the basename of a file name
char *tcc_basename(const char *name) {
  char *p = strchr(name, 0);
  while (p > name && p[-1] != '/' && p[-1] != '\\') --p;
  return p;
}

// Extract the file extension of a file name
char *tcc_fileextension(const char *name) {
  char *b = tcc_basename(name);
  char *e = strrchr(b, '.');
  return e ? e : strchr(b, 0);
}

// Copy a string and truncate it
char *pstrcpy(char *buf, int buf_size, const char *s) {
  char *q, *q_end;
  int c;

  if (buf_size > 0) {
    q = buf;
    q_end = buf + buf_size - 1;
    while (q < q_end) {
      c = *s++;
      if (c == '\0') break;
      *q++ = c;
    }
    *q = '\0';
  }
  return buf;
}

// strcat and truncate
char *pstrcat(char *buf, int buf_size, const char *s) {
  int len;
  len = strlen(buf);
  if (len < buf_size) pstrcpy(buf + len, buf_size - len, s);
  return buf;
}

int strstart(const char *str, const char *val, const char **ptr) {
  const char *p, *q;
  p = str;
  q = val;
  while (*q != '\0') {
    if (*p != *q) return 0;
    p++;
    q++;
  }
  if (ptr) *ptr = p;
  return 1;
}

// Space exlcuding newline
int is_space(int c) {
  return c == ' ' || c == '\t' || c == '\v' || c == '\f' || c == '\r';
}

int is_id(int c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

int is_num(int c) {
  return c >= '0' && c <= '9';
}

int is_idnum(int c) {
  return c >= 0 && isidnum_table[c];
}

int is_oct(int c) {
  return c >= '0' && c <= '7';
}

int to_upper(int c) {
  if (c >= 'a' && c <= 'z') {
    return c - 'a' + 'A';
  } else {
    return c;
  }
}

void dynarray_add(void ***ptab, int *nb_ptr, void *data) {
  int nb, nb_alloc;
  void **pp;
  
  nb = *nb_ptr;
  pp = *ptab;
  // Every power of two we double array size
  if ((nb & (nb - 1)) == 0) {
    if (!nb) {
      nb_alloc = 1;
    } else {
      nb_alloc = nb * 2;
    }
    pp = tcc_realloc(pp, nb_alloc * sizeof(void *));
    if (!pp) error("memory full");
    *ptab = pp;
  }
  pp[nb++] = data;
  *nb_ptr = nb;
}

void dynarray_reset(void *pp, int *n) {
  void **p;
  for (p = *(void***) pp; *n; ++p, --*n) {
    if (*p) tcc_free(*p);
  }
  tcc_free(*(void**) pp);
  *(void**)pp = NULL;
}

int dynarray_assoc(void **pp, int n, int key) {
  int i;
  for (i = 0; i < n; ++i, ++pp) {
    if (key == **(int **) pp) return i;
  }
  return -1;
}

void strcat_vprintf(char *buf, int buf_size, const char *fmt, va_list ap) {
  int len;
  len = strlen(buf);
  vsnprintf(buf + len, buf_size - len, fmt, ap);
}

void strcat_printf(char *buf, int buf_size, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  strcat_vprintf(buf, buf_size, fmt, ap);
  va_end(ap);
}

void cstr_new(CString *cstr) {
  memset(cstr, 0, sizeof(CString));
}

void cstr_free(CString *cstr) {
  tcc_free(cstr->data_allocated);
  cstr_new(cstr);
}

void cstr_reset(CString *cstr) {
  cstr_free(cstr);
}

void cstr_realloc(CString *cstr, int new_size) {
  int size;
  void *data;

  size = cstr->size_allocated;
  if (size == 0) size = 8;
  while (size < new_size) size = size * 2;
  data = tcc_realloc(cstr->data_allocated, size);
  if (!data) error("memory full");
  cstr->data_allocated = data;
  cstr->size_allocated = size;
  cstr->data = data;
}

void cstr_ccat(CString *cstr, int ch) {
  int size;
  size = cstr->size + 1;
  if (size > cstr->size_allocated) cstr_realloc(cstr, size);
  ((unsigned char *)cstr->data)[size - 1] = ch;
  cstr->size = size;
}

void cstr_cat(CString *cstr, const char *str) {
  int c;
  for (;;) {
    c = *str;
    if (c == '\0') break;
    cstr_ccat(cstr, c);
    str++;
  }
}

void cstr_wccat(CString *cstr, int ch) {
  int size;
  size = cstr->size + sizeof(nwchar_t);
  if (size > cstr->size_allocated) cstr_realloc(cstr, size);
  *(nwchar_t *)(((unsigned char *)cstr->data) + size - sizeof(nwchar_t)) = ch;
  cstr->size = size;
}

void add_char(CString *cstr, int c)
{
  if (c == '\'' || c == '\"' || c == '\\') {
    // TODO: could be more precise if char or string
    cstr_ccat(cstr, '\\');
  }
  if (c >= 32 && c <= 126) {
    cstr_ccat(cstr, c);
  } else {
    cstr_ccat(cstr, '\\');
    if (c == '\n') {
      cstr_ccat(cstr, 'n');
    } else {
      cstr_ccat(cstr, '0' + ((c >> 6) & 7));
      cstr_ccat(cstr, '0' + ((c >> 3) & 7));
      cstr_ccat(cstr, '0' + (c & 7));
    }
  }
}

void error_ex(TCCState *s1, int is_warning, const char *fmt, va_list ap)
{
  char buf[2048];
  BufferedFile **f;
  
  buf[0] = '\0';
  if (file) {
    for (f = s1->include_stack; f < s1->include_stack_ptr; f++) {
      strcat_printf(buf, sizeof(buf), "In file included from %s:%d:\n", (*f)->filename, (*f)->line_num);
    }
    if (file->line_num > 0) {
      strcat_printf(buf, sizeof(buf), "%s:%d: ", file->filename, file->line_num);
    } else {
      strcat_printf(buf, sizeof(buf), "%s: ", file->filename);
    }
  } else {
    strcat_printf(buf, sizeof(buf), "cc: ");
  }
  if (is_warning) {
    strcat_printf(buf, sizeof(buf), "warning: ");
  }
  strcat_vprintf(buf, sizeof(buf), fmt, ap);

  if (!s1->error_func) {
    // Default case: stderr
    fprintf(stderr, "%s\n", buf);
  } else {
    s1->error_func(s1->error_opaque, buf);
  }
  if (!is_warning || s1->warn_error) s1->nb_errors++;
}

void error(const char *fmt, ...) {
  TCCState *s1 = tcc_state;
  va_list ap;

  va_start(ap, fmt);
  error_ex(s1, 0, fmt, ap);
  va_end(ap);
  // Better than nothing: in some cases, we accept to handle errors
  if (s1->error_set_jmp_enabled) {
    longjmp(s1->error_jmp_buf, 1);
  } else {
    exit(1);
  }
}

void warning(const char *fmt, ...) {
  TCCState *s1 = tcc_state;
  va_list ap;

  if (s1->warn_none) return;

  va_start(ap, fmt);
  error_ex(s1, 1, fmt, ap);
  va_end(ap);
}

// Error without aborting current compilation
void error_noabort(const char *fmt, ...) {
  TCCState *s1 = tcc_state;
  va_list ap;

  va_start(ap, fmt);
  error_ex(s1, 0, fmt, ap);
  va_end(ap);
}

void expect(const char *msg) {
  error("%s expected", msg);
}

void *load_data(int fd, unsigned long file_offset, unsigned long size) {
  void *data;

  data = tcc_malloc(size);
  lseek(fd, file_offset, SEEK_SET);
  read(fd, data, size);
  return data;
}

void init_util(void) {
  int i;

  // Init isidnum table
  for (i = 0; i < 256; i++) isidnum_table[i] = is_id(i) || is_num(i);
}

