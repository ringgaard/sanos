#include "sh.h"

int pushfile(struct inputfile **file, int fd) {
  struct inputfile *inp;

  if (fd == -1) return 0;
  inp = (struct inputfile *) malloc(sizeof(struct inputfile));
  if (!inp) return -1;

  inp->prev = *file;
  inp->lineno = 1;
  inp->fd = fd;
  inp->buf = inp->ptr = inp->end = NULL;

  *file = inp;
  return 0;
}

int pushstr(struct inputfile **file, char *str) {
  struct inputfile *inp;
  char *newstr;

  if (!str) return 0;
  newstr = strdup(str);
  if (!newstr) return -1;

  inp = (struct inputfile *) malloc(sizeof(struct inputfile));
  if (!inp) return -1;

  inp->prev = *file;
  inp->lineno = 1;
  inp->fd = -1;
  inp->buf = inp->ptr = newstr;
  inp->end = inp->buf + strlen(newstr);

  *file = inp;
  return 0;
}

int popfile(struct inputfile **file) {
  struct inputfile *inp;

  inp = *file;
  if (!inp) return 0;
  if (inp->fd != -1) close(inp->fd);
  if (inp->buf) free(inp->buf);

  *file = inp->prev;
  free(inp);

  return 0;
}

void popallfiles(struct inputfile **file) {
  while (*file) popfile(file);
}

int pgetc(struct inputfile **file, int peek) {
  while (*file) {
    struct inputfile *inp = *file;
    
    if (inp->ptr != inp->end) {
      if (peek) {
        return *inp->ptr;
      } else {
        int ch = *(inp->ptr)++;
        if (ch == '\n') inp->lineno++;
        return ch;
      }
    }

    if (inp->fd != -1) {
      int rc;

      if (!inp->buf) inp->buf = inp->ptr = inp->end = malloc(FILEBUFSIZ);
      if (!inp->buf) return -1;

      rc = read(inp->fd, inp->buf, FILEBUFSIZ);
      if (rc < 0) {
        return -1;
      } else if (rc == 0){
        if (popfile(file) < 0) return -1;
      } else {
        inp->ptr = inp->buf;
        inp->end = inp->buf + rc;
      }
    } else {
      if (popfile(file) < 0) return -1;
    }
  }

  return -1;
}
