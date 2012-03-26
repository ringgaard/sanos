#ifndef INPUT_H
#define INPUT_H

#define FILEBUFSIZ 512

struct inputfile
{
  struct inputfile *prev;
  int lineno;
  int fd;

  char *buf;
  char *ptr;
  char *end;
};

int pushfile(struct inputfile **file, int fd);
int pushstr(struct inputfile **file, char *str);
int popfile(struct inputfile **file);
void popallfiles(struct inputfile **file);

int pgetc(struct inputfile **file, int peek);

#endif
