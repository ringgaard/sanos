#ifndef SYSCALL_H

struct syscallops
{
  // Debug
  void (*panic)(char *msg);
  void (*print_string)(char *s);
  void (*set_break_handler)(void (*handler)(int type));
  
  // Memory
  void *(*mmap)(void *addr, unsigned long size, int type, int protect);
  int (*munmap)(void *addr, unsigned long size, int type);

  // Handles
  int (*close_handle)(handle_t handle);
  handle_t (*dup_handle)(handle_t handle);
  handle_t (*get_std_handle)(int handlenum);

  // Files
  char *(*get_file_path)(char *filename, char *buffer, int size);
  int (*get_file_stat)(char *filename, struct stat *buffer);
  int (*get_handle_stat)(handle_t f, struct stat *buffer);

  // Directories
  int (*mkdir)(char *dirname);
  handle_t (*opendir)(char *filename);
  int (*readdir)(handle_t find, struct dirent *dirp, int count);

  // I/O
  handle_t (*open_file)(char *name, int mode);
  int (*close_file)(handle_t f);
  int (*read_file)(handle_t f, void *data, int len);
  int (*write_file)(handle_t f, void *data, int len);
  int (*set_file_pointer)(handle_t f, int offset, int origin);

  // Threads
  handle_t (*begin_thread)(void (__stdcall *startaddr)(void *), unsigned stacksize, void *arg, int suspended, int *threadid);
  int (*suspend_thread)(handle_t thread);
  int (*resume_thread)(handle_t thread);
  void (*end_thread)(int retval);
  handle_t (*get_current_thread)();
  unsigned long (*get_current_thread_id)();
  int (*get_thread_context)(handle_t thread, void *context);
  int (*get_thread_priority)(handle_t thread);
  int (*set_thread_priority)(handle_t thread, int priority);
  void (*sleep)(int millisecs);

  // Synchronization
  int (*wait_for_object)(handle_t obj, unsigned long timeout);
  int (*wait_for_objects)(handle_t *objs, int count, int waitall, unsigned long timeout);

  // TLS
  tls_t (*alloc_tls)();
  void (*free_tls)(tls_t index);
  void *(*get_tls)(tls_t index);
  void (*set_tls)(tls_t index, void *value);

  // Events
  handle_t (*create_event)(int manual_reset, int initial_state);
  void (*set_event)(handle_t event);
  void (*reset_event)(handle_t event);

  // Semaphore
  handle_t (*create_sem)(unsigned int initial_count);
  unsigned int (*release_sem)(handle_t sem, unsigned int count);

  // Time
  unsigned long (*get_tick_count)();
  systime_t (*get_time)();

  // Sockets
  struct sockcallops *(*get_sock_calls)();

  // Junk
  void (*systemtime_to_filetime)(void *st, void *ft);
  void (*get_system_time)(void *st);
};

#endif
