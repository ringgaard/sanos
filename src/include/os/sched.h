//
// sched.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
// 
// Task scheduler
//

#ifndef SCHED_H
#define SCHED_H

typedef void (*threadproc_t)(void *arg);
typedef void (*dpcproc_t)(void *arg);
typedef void (*taskproc_t)(void *arg);

#define DEFAULT_QUANTUM          12
#define THREAD_PRIORITY_LEVELS   8

#define PAGES_PER_TCB     2

#define TCBSIZE           (PAGES_PER_TCB * PAGESIZE)
#define TCBMASK           (~(TCBSIZE - 1))
#define TCBESP            (TCBSIZE - 4)

#define DPC_QUEUED        1
#define DPC_EXECUTING     2

#define TASK_QUEUE_ACTIVE              1
#define TASK_QUEUE_ACTIVE_TASK_INVALID 2

#define TASK_QUEUED       1
#define TASK_EXECUTING    2

struct tcb
{
  struct thread thread;
  unsigned long stack[(TCBSIZE - sizeof(struct thread)) / sizeof(unsigned long) - 1];
  unsigned long *esp;
};

struct dpc
{
  dpcproc_t proc;
  void *arg;
  struct dpc *next;
  int flags;
};

struct task
{
  taskproc_t proc;
  void *arg;
  struct task *next;
  int flags;
};

struct task_queue
{
  struct task *head;
  struct task *tail;
  struct thread *thread;
  int maxsize;
  int size;
  int flags;
};

struct kernel_context
{
  unsigned long esi, edi;
  unsigned long ebx, ebp;
  unsigned long eip;
  char stack[0];
};

extern struct thread *idlethread;
extern struct thread *threadlist;
extern struct task_queue sys_task_queue;

extern struct dpc *dpc_queue_head;
extern struct dpc *dpc_queue_tail;

extern int in_dpc;
extern int preempt;
extern unsigned long dpc_time;
extern unsigned long dpc_total;

#if 0
__inline __declspec(naked) struct thread *self()
{
  __asm
  {
    mov eax, esp;
    and eax, TCBMASK
    ret
  }
}
#endif

__inline struct thread *self()
{
  unsigned long stkvar;
  return (struct thread *) (((unsigned long) &stkvar) & TCBMASK);
}

void mark_thread_running();

krnlapi void mark_thread_ready(struct thread *t);
krnlapi void enter_wait(int reason);

krnlapi struct thread *create_kernel_thread(threadproc_t startaddr, void *arg, int priority, char *name);

int create_user_thread(void *entrypoint, unsigned long stacksize, struct thread **retval);
int init_user_thread(struct thread *t, void *entrypoint);
int allocate_user_stack(struct thread *t, unsigned long stack_reserve, unsigned long stack_commit);
int destroy_thread(struct thread *t);

struct thread *get_thread(tid_t tid);

int suspend_thread(struct thread *t);
int resume_thread(struct thread *t);
void terminate_thread(int exitcode);

int init_task_queue(struct task_queue *tq, int priority, int maxsize, char *name);
void init_task(struct task *task);
int queue_task(struct task_queue *tq, struct task *task, taskproc_t proc, void *arg);

krnlapi void init_dpc(struct dpc *dpc);
krnlapi void queue_dpc(struct dpc *dpc, dpcproc_t proc, void *arg);
krnlapi void queue_irq_dpc(struct dpc *dpc, dpcproc_t proc, void *arg);

void idle_task();
krnlapi void yield();
void dispatch_dpc_queue();
void preempt_thread();
krnlapi void dispatch();

void init_sched();

__inline void check_dpc_queue()
{
  if (dpc_queue_head) dispatch_dpc_queue();
}

__inline void check_preempt()
{
  if (preempt) preempt_thread();
}

#endif
