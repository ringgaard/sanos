#ifndef TYPES_H
#define TYPES_H

typedef unsigned int devno_t;
typedef unsigned int blkno_t;
typedef unsigned int ino_t;
typedef unsigned int loff_t;

typedef unsigned int ino_t;
typedef long time_t;
typedef unsigned int size_t;

void *kmalloc(int size);
void kfree(void *p);

time_t time(time_t *timer);

void *memcpy(void *, const void *, size_t);
int memcmp(const void *, const void *, size_t);
void *memset(void *, int, size_t);

char *strcpy(char *, const char *);
char *strcat(char *, const char *);
int strcmp(const char *, const char *);
size_t strlen(const char *);

int atoi(const char *s);

#define NULL    ((void *)0)

#endif
