#ifndef OS_H
#define OS_H

typedef int handle_t;
typedef void *hmodule_t;
typedef unsigned long tls_t;
typedef __int64 systime_t;
typedef long time_t;
typedef unsigned int size_t;
typedef unsigned int ino_t;
typedef unsigned int devno_t;
typedef unsigned int blkno_t;
typedef unsigned int loff_t;

#include "vfs.h"

#ifdef OS_LIB
#define apicall __declspec(dllexport)
#else
#ifdef OSEXEC
#define apicall
#else
#define apicall __declspec(dllimport)
#endif
#endif

#define NULL  ((void *)0)
#define FALSE 0
#define TRUE  1

#define NOHANDLE ((handle_t) -1)

#define MAXPATH   256
#define PAGESIZE  4096

#define DLL_PROCESS_ATTACH      1
#define DLL_THREAD_ATTACH       2
#define DLL_THREAD_DETACH       3
#define DLL_PROCESS_DETACH      0

#define PAGE_NOACCESS           0x01     
#define PAGE_READONLY           0x02     
#define PAGE_READWRITE          0x04     
#define PAGE_WRITECOPY          0x08     
#define PAGE_EXECUTE            0x10     
#define PAGE_EXECUTE_READ       0x20     
#define PAGE_EXECUTE_READWRITE  0x40     
#define PAGE_GUARD              0x100     

#define MEM_COMMIT              0x1000     
#define MEM_RESERVE             0x2000     
#define MEM_DECOMMIT            0x4000     
#define MEM_RELEASE             0x8000     
//#define MEM_FREE                0x10000     

#define O_RDONLY                0x0000  // Open for reading only
#define O_WRONLY                0x0001  // Open for writing only
#define O_RDWR                  0x0002  // Open for reading and writing
#define O_APPEND                0x0008  // Writes done at EOF

#define O_CREAT                 0x0100  // Create and open file
#define O_TRUNC                 0x0200  // Open and truncate/
#define O_EXCL                  0x0400  // Open only if file doesn't already exist

#define SEEK_SET                0
#define SEEK_CUR                1
#define SEEK_END                2

#define FILE_STAT_DIRECTORY     1

#define LOG_LEVEL_MASK          0x0000000F
#define LOG_SUBSYS_MASK         0xFFFFFFF0

#define LOG_ERROR               1
#define LOG_WARNING             2
#define LOG_INFO                3
#define LOG_DEBUG               4
#define LOG_TRACE               5

#define LOG_MODULE              0x80000000
#define LOG_HEAP                0x40000000
#define LOG_APITRACE            0x20000000
#define LOG_AUX                 0x10000000

#define OK    0
#define EFAIL (-1)
#define INFINITE  0xFFFFFFFF

#ifndef OSEXEC

typedef char *va_list;

#define _INTSIZEOF(n)    ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))

#define va_start(ap, v)  (ap = (va_list) &v + _INTSIZEOF(v))
#define va_arg(ap, t)    (*(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))
#define va_end(ap)       (ap = (va_list) 0)

#endif

struct critsect
{
  long count;
  long recursion;
  handle_t owner;
  handle_t event;
};

typedef struct critsect *critsect_t;

#ifndef KERNEL

// OS API functions

apicall void panic(char *msg);
apicall void print_string(char *s);
apicall void set_break_handler(void (*handler)(int type));

apicall int vformat(char *buf, const char *fmt, va_list args);
apicall int format(char *buf, const char *fmt, ...);

apicall void syslog(int priority, const char *fmt, ...);

apicall void *mmap(void *addr, unsigned long size, int type, int protect);
apicall int munmap(void *addr, unsigned long size, int type);

apicall void *malloc(size_t size);
apicall void *realloc(void *mem, size_t size);
apicall void *calloc(size_t num, size_t size);
apicall void free(void *p);

apicall int close_handle(handle_t handle);
apicall handle_t dup_handle(handle_t handle);
apicall handle_t get_std_handle(int handlenum);

apicall char *get_file_path(char *filename, char *buffer, int size);
apicall int get_file_stat(char *filename, struct stat *buffer);
apicall int get_handle_stat(handle_t f, struct stat *buffer);

apicall int mkdir(char *dirname);
apicall handle_t opendir(char *name);
apicall int readdir(handle_t find, struct dirent *dirp, int count);

apicall handle_t open_file(char *name, int mode);
apicall int close_file(handle_t f);
apicall int read_file(handle_t f, void *data, int len);
apicall int write_file(handle_t f, void *data, int len);
apicall int set_file_pointer(handle_t f, int offset, int origin);

apicall hmodule_t get_module_handle(char *name);
apicall hmodule_t load_module(char *name);
apicall void *get_proc_address(hmodule_t hmod, char *procname);
apicall int get_module_filename(hmodule_t hmod, char *buffer, int size);

apicall handle_t begin_thread(void (__stdcall *startaddr)(void *), unsigned stacksize, void *arg, int suspended, int *threadid);
apicall int suspend_thread(handle_t thread);
apicall int resume_thread(handle_t thread);
apicall void end_thread(int retval);
apicall handle_t get_current_thread();
apicall unsigned long get_current_thread_id();
apicall int get_thread_context(handle_t thread, void *context);
apicall int get_thread_priority(handle_t thread);
apicall int set_thread_priority(handle_t thread, int priority);
apicall void sleep(int millisecs);

apicall int wait_for_object(handle_t obj, unsigned long timeout);
apicall int wait_for_objects(handle_t *objs, int count, int waitall, unsigned long timeout);

apicall void init_critsect(critsect_t cs);
apicall void delete_critsect(critsect_t cs);
apicall void enter_critsect(critsect_t cs);
apicall void leave_critsect(critsect_t cs);

apicall tls_t alloc_tls();
apicall void free_tls(tls_t index);
apicall void *get_tls(tls_t index);
apicall void set_tls(tls_t index, void *value);

apicall handle_t create_event(int manual_reset, int initial_state);
apicall void set_event(handle_t event);
apicall void reset_event(handle_t event);
apicall handle_t create_sem(unsigned int initial_count);
apicall unsigned int release_sem(handle_t sem, unsigned int count);

apicall unsigned long get_tick_count();
apicall systime_t get_time();

apicall struct sockcallops *get_sock_calls();

apicall void systemtime_to_filetime(void *st, void *ft);
apicall void get_system_time(void *st);

// intrinsic functions

void *memcpy(void *, const void *, size_t);
int memcmp(const void *, const void *, size_t);
void *memset(void *, int, size_t);

char *strcpy(char *, const char *);
char *strcat(char *, const char *);
int strcmp(const char *, const char *);
size_t strlen(const char *);

#endif

#endif
