//
// sched.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
// 
// Task scheduler
//

#ifndef SCHED_H
#define SCHED_H

typedef void (*taskproc_t)(void *arg);
typedef void (*dpcproc_t)(void *arg);

#define THREAD_PRIORITY_LEVELS   8

#define PAGES_PER_TCB     2

#define TCBSIZE           (PAGES_PER_TCB * PAGESIZE)
#define TCBMASK           (~(TCBSIZE - 1))
#define TCBESP            (TCBSIZE - 4)

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
  int active;
};

extern int resched;
extern int idle;
extern struct thread *idlethread;
extern struct thread *threadlist;

void mark_thread_running();

krnlapi void mark_thread_ready(struct thread *t);

krnlapi struct thread *current_thread();
krnlapi struct thread *create_kernel_thread(taskproc_t task, void *arg, int priority);

int create_user_thread(void *entrypoint, unsigned long stacksize, struct thread **retval);
int init_user_thread(struct thread *t, void *entrypoint);
int allocate_user_stack(struct thread *t, unsigned long stack_reserve, unsigned long stack_commit);
int destroy_thread(struct thread *t);

int suspend_thread(struct thread *t);
int resume_thread(struct thread *t);
void terminate_thread(int exitcode);

krnlapi void init_dpc(struct dpc *dpc);
krnlapi void queue_dpc(struct dpc *dpc, dpcproc_t proc, void *arg);
krnlapi void queue_irq_dpc(struct dpc *dpc, dpcproc_t proc, void *arg);

void idle_task();
krnlapi void yield();
void dispatch_dpc_queue();
krnlapi void dispatch();

void init_sched();

#endif
