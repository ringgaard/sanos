#include <os.h>
#include <syscall.h>

#include "heap.h"
#include "mod.h"

#define SYSLOG
//#define TRACEAPI

unsigned long loglevel = LOG_DEBUG | LOG_HEAP | LOG_APITRACE | LOG_AUX;

#ifdef TRACEAPI
#define TRACE(s) if (syscalls && (loglevel & LOG_APITRACE)) syscalls->print_string(s " called\n");
#else
#define TRACE(s)
#endif

struct syscallops *syscalls;
struct critsect heap_lock;

char *strncpy(char *dest, const char *source, size_t count)
{
  char *start = dest;

  while (count && (*dest++ = *source++)) count--;
  if (count) while (--count) *dest++ = '\0';
  return start;
}

char *strdup(char *s)
{
  char *t;
  int len;

  if (!s) return NULL;
  len = strlen(s);
  t = (char *) malloc(len + 1);
  memcpy(t, s, len + 1);
  return t;
}

int stricmp(const char *s1, const char *s2)
{
  char f,l;

  do 
  {
    f = ((*s1 <= 'Z') && (*s1 >= 'A')) ? *s1 + 'a' - 'A' : *s1;
    l = ((*s2 <= 'Z') && (*s2 >= 'A')) ? *s2 + 'a' - 'A' : *s2;
    s1++;
    s2++;
  } while ((f) && (f == l));

  return (int) (f - l);
}

char *itos(int n, char *s, int radix)
{
  char *p;
  char *firstdig;
  char temp;
  unsigned digval;

  p = s;

  if (n < 0)
  {
    *p++ = '-';
    n = -n;
  }

  firstdig = p;

  do
  {
    digval = (unsigned) (n % radix);
    n /= radix;

    if (digval > 9)
      *p++ = (char) (digval - 10 + 'a');
    else
      *p++ = (char) (digval + '0');
  } while (n > 0);

  *p-- = '\0';

  do 
  {
    temp = *p;
    *p = *firstdig;
    *firstdig = temp;
    p--;
    firstdig++;
  } while (firstdig < p);

  return s;
}

void panic(char *msg)
{
  //TRACE("panic");
  syscalls->panic(msg);
}

void print_string(char *s)
{
  //TRACE("print_string");
  syscalls->print_string(s);
}

void set_break_handler(void (*handler)(int type))
{
  TRACE("set_break_handler");
  syscalls->set_break_handler(handler);
}

#ifdef SYSLOG
void syslog(int priority, const char *fmt,...)
{
  va_list args;
  char buffer[1024];

  if (!(priority & LOG_SUBSYS_MASK)) priority |= LOG_AUX;

  if (((unsigned) priority & LOG_LEVEL_MASK) >= (loglevel & LOG_LEVEL_MASK) && (priority & loglevel & LOG_SUBSYS_MASK) != 0)
  {
    va_start(args, fmt);
    vformat(buffer, fmt, args);
    va_end(args);
    print_string(buffer);
  }
}
#else
void syslog(int priority, const char *fmt,...)
{
}
#endif

void *mmap(void *addr, unsigned long size, int type, int protect)
{
  TRACE("mmap");
  return syscalls->mmap(addr, size, type, protect);
}

int munmap(void *addr, unsigned long size, int type)
{
  TRACE("munmap");
  return syscalls->munmap(addr, size, type);
}

void *malloc(size_t size)
{
  void *p;

  TRACE("malloc");

  enter_critsect(&heap_lock);
  p = heap_alloc(size);
  leave_critsect(&heap_lock);
  return p;
}

void *realloc(void *mem, size_t size)
{
  void *p;

  TRACE("realloc");

  enter_critsect(&heap_lock);
  p = heap_realloc(mem, size);
  leave_critsect(&heap_lock);
  return p;
}

void *calloc(size_t num, size_t size)
{
  void *p;

  TRACE("calloc");

  enter_critsect(&heap_lock);
  p = heap_calloc(num, size);
  leave_critsect(&heap_lock);
  return p;
}

void free(void *p)
{
  TRACE("free");

  enter_critsect(&heap_lock);
  heap_free(p);
  leave_critsect(&heap_lock);
}

int close_handle(handle_t handle)
{
  TRACE("close_handle");
  return syscalls->close_handle(handle);
}

handle_t dup_handle(handle_t handle)
{
  TRACE("dup_handle");
  return syscalls->dup_handle(handle);
}

handle_t get_std_handle(int handlenum)
{
  TRACE("get_std_handle");
  return syscalls->get_std_handle(handlenum);
}

char *get_file_path(char *filename, char *buffer, int size)
{
  char *retval;

  TRACE("get_file_path");
  retval = syscalls->get_file_path(filename, buffer, size);
  syslog(LOG_AUX | LOG_DEBUG, "filepath: %s -> %s\n", filename, buffer);
  return retval;
}

int get_file_stat(char *filename, struct stat *buffer)
{
  int rc;

  TRACE("get_file_stat");
  rc = syscalls->get_file_stat(filename, buffer);
  return rc;
}

int get_handle_stat(handle_t f, struct stat *buffer)
{
  TRACE("get_handle_stat");
  return syscalls->get_handle_stat(f, buffer);
}

int mkdir(char *dirname)
{
  TRACE("mkdir");
  return syscalls->mkdir(dirname);
}

handle_t opendir(char *name)
{
  TRACE("opendir");
  return syscalls->opendir(name);
}

int readdir(handle_t find, struct dirent *dirp, int count)
{
  TRACE("readdir");
  return syscalls->readdir(find, dirp, count);
}

handle_t open_file(char *name, int mode)
{
  TRACE("open_file");
  syslog(LOG_DEBUG, "open file %s\n", name);
  return syscalls->open_file(name, mode);
}

int close_file(handle_t f)
{
  TRACE("close_file");
  return syscalls->close_file(f);
}

int read_file(handle_t f, void *data, int len)
{
  TRACE("read_file");
  return syscalls->read_file(f, data, len);
}

int write_file(handle_t f, void *data, int len)
{
  TRACE("write_file");
  return syscalls->write_file(f, data, len);
}

int set_file_pointer(handle_t f, int offset, int origin)
{
  TRACE("set_file_pointer");
  return syscalls->set_file_pointer(f, offset, origin);
}

handle_t begin_thread(void (__stdcall *startaddr)(void *), unsigned stacksize, void *arg, int suspended, int *threadid)
{
  TRACE("begin_thread");
  return syscalls->begin_thread(startaddr, stacksize, arg, suspended, threadid);
}

int suspend_thread(handle_t thread)
{
  TRACE("suspend_thread");
  return syscalls->suspend_thread(thread);
}

int resume_thread(handle_t thread)
{
  TRACE("resume_thread");
  return syscalls->resume_thread(thread);
}

void end_thread(int retval)
{
  TRACE("end_thread");
  syscalls->end_thread(retval);
}

handle_t get_current_thread()
{
  //TRACE("get_current_thread");
  return syscalls->get_current_thread();
}

unsigned long get_current_thread_id()
{
  TRACE("get_current_thread_id");
  return syscalls->get_current_thread_id();
}

int get_thread_context(handle_t thread, void *context)
{
  TRACE("get_thread_context");
  return syscalls->get_thread_context(thread, context);
}

int get_thread_priority(handle_t thread)
{
  TRACE("get_thread_priority");
  return syscalls->get_thread_priority(thread);
}

int set_thread_priority(handle_t thread, int priority)
{
  TRACE("set_thread_priority");
  return syscalls->set_thread_priority(thread, priority);
}

void sleep(int millisecs)
{
  //TRACE("sleep");
  syscalls->sleep(millisecs);
}

int wait_for_object(handle_t obj, unsigned long timeout)
{
  TRACE("wait_for_object");
  return syscalls->wait_for_object(obj, timeout);
}

int wait_for_objects(handle_t *objs, int count, int waitall, unsigned long timeout)
{
  TRACE("wait_for_objects");
  return syscalls->wait_for_objects(objs, count, waitall, timeout);
}

tls_t alloc_tls()
{
  TRACE("alloc_tls");
  return syscalls->alloc_tls();
}

void free_tls(tls_t index)
{
  TRACE("free_tls");
  syscalls->free_tls(index);
}

void *get_tls(tls_t index)
{
  //TRACE("get_tls");
  return syscalls->get_tls(index);
}

void set_tls(tls_t index, void *value)
{
  TRACE("set_tls");
  syscalls->set_tls(index, value);
}

handle_t create_event(int manual_reset, int initial_state)
{
  TRACE("create_event");
  return syscalls->create_event(manual_reset, initial_state);
}

void set_event(handle_t event)
{
  TRACE("set_event");
  syscalls->set_event(event);
}

void reset_event(handle_t event)
{
  TRACE("reset_event");
  syscalls->reset_event(event);
}

handle_t create_sem(unsigned int initial_count)
{
  TRACE("create_sem");
  return syscalls->create_sem(initial_count);
}

unsigned int release_sem(handle_t sem, unsigned int count)
{
  TRACE("release_sem");
  return syscalls->release_sem(sem, count);
}

unsigned long get_tick_count()
{
  TRACE("get_tick_count");
  return syscalls->get_tick_count();
}

systime_t get_time()
{
  TRACE("get_time");
  return syscalls->get_time();
}

struct sockcallops *get_sock_calls()
{
  TRACE("get_sock_calls");
  return syscalls->get_sock_calls();
}

void systemtime_to_filetime(void *st, void *ft)
{
  TRACE("systemtime_to_filetime");
  syscalls->systemtime_to_filetime(st, ft);
}

void get_system_time(void *st)
{
  TRACE("get_system_time");
  syscalls->get_system_time(st);
}

int __stdcall start(hmodule_t hmod, struct syscallops *sc, char *cmdline)
{
  char *pgm;
  hmodule_t hexecmod;
  int rc;

  // Save reference to system call vector
  syscalls = sc;

  // Initialize heap lock
  init_critsect(&heap_lock);

  // Initialize module table
  init_modules(hmod, "\\os");

  // Parse command line
  while (*cmdline && *cmdline != ' ') cmdline++;
  while (*cmdline == ' ') cmdline++;
  pgm = cmdline;
  while (*cmdline && *cmdline != ' ') cmdline++;
  if (*cmdline == ' ') 
  {
    *cmdline++ = 0;
    while (*cmdline == ' ') cmdline++;
  }

  hexecmod = load_module(pgm);
  if (hexecmod == NULL) panic("unable to load executable");
  syslog(LOG_DEBUG, "exec main(%s)\n", cmdline);

  rc = ((int (__stdcall *)(hmodule_t, char *, int)) get_entrypoint(hexecmod))(hexecmod, cmdline, 0);

  if (rc != 0) syslog(LOG_DEBUG, "Exitcode: %s\n", rc);

  return rc;
}
