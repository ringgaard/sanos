//
// sched.h
//
// Task scheduler
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

#ifndef SCHED_H
#define SCHED_H

typedef void (*threadproc_t)(void *arg);
typedef void (*dpcproc_t)(void *arg);
typedef void (*taskproc_t)(void *arg);

//#define NOPREEMPTION

#define DEFAULT_QUANTUM          36
#define QUANTUM_UNITS_PER_TICK   3
#define THREAD_PRIORITY_LEVELS   32

#define PAGES_PER_TCB     2

#define TCBSIZE           (PAGES_PER_TCB * PAGESIZE)
#define TCBMASK           (~(TCBSIZE - 1))
#define TCBESP            (TCBSIZE - 4)

#define DPC_QUEUED_BIT        0
#define DPC_EXECUTING_BIT     1
#define DPC_NORAND_BIT        2

#define DPC_QUEUED            (1 << DPC_QUEUED_BIT)
#define DPC_EXECUTING         (1 << DPC_EXECUTING_BIT)
#define DPC_NORAND            (1 << DPC_NORAND_BIT)

#define TASK_QUEUE_ACTIVE              1
#define TASK_QUEUE_ACTIVE_TASK_INVALID 2

#define TASK_QUEUED       1
#define TASK_EXECUTING    2

struct tcb {
  struct thread thread;
  unsigned long stack[(TCBSIZE - sizeof(struct thread)) / sizeof(unsigned long) - 1];
  unsigned long *esp;
};

struct dpc {
  dpcproc_t proc;
  void *arg;
  struct dpc *next;
  int flags;
};

struct task {
  taskproc_t proc;
  void *arg;
  struct task *next;
  int flags;
};

struct task_queue {
  struct task *head;
  struct task *tail;
  struct thread *thread;
  int maxsize;
  int size;
  int flags;
};

struct kernel_context {
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

#if 0
__inline __declspec(naked) struct thread *self() {
  __asm {
    mov eax, esp;
    and eax, TCBMASK
    ret
  }
}
#endif

#if 0
__inline struct thread *self() {
  unsigned long stkvar;
  return (struct thread *) (((unsigned long) &stkvar) & TCBMASK);
}
#endif

struct thread *self();

void mark_thread_running();

krnlapi void mark_thread_ready(struct thread *t, int charge, int boost);
krnlapi void enter_wait(int reason);
krnlapi int enter_alertable_wait(int reason);
krnlapi int interrupt_thread(struct thread *t);

krnlapi struct thread *create_kernel_thread(threadproc_t startaddr, void *arg, int priority, char *name);

int create_user_thread(void *entrypoint, unsigned long stacksize, char *name, struct thread **retval);
int init_user_thread(struct thread *t, void *entrypoint);
int allocate_user_stack(struct thread *t, unsigned long stack_reserve, unsigned long stack_commit);
int destroy_thread(struct thread *t);

struct thread *get_thread(tid_t tid);

int suspend_thread(struct thread *t);
int resume_thread(struct thread *t);
void terminate_thread(int exitcode);
void suspend_all_user_threads();
int schedule_alarm(unsigned int seconds);

int get_thread_priority(struct thread *t);
int set_thread_priority(struct thread *t, int priority);

krnlapi int init_task_queue(struct task_queue *tq, int priority, int maxsize, char *name);
krnlapi void init_task(struct task *task);
krnlapi int queue_task(struct task_queue *tq, struct task *task, taskproc_t proc, void *arg);

krnlapi void init_dpc(struct dpc *dpc);
krnlapi void queue_dpc(struct dpc *dpc, dpcproc_t proc, void *arg);
krnlapi void queue_irq_dpc(struct dpc *dpc, dpcproc_t proc, void *arg);

krnlapi void add_idle_task(struct task *task, taskproc_t proc, void *arg);

void idle_task();
krnlapi void yield();
void dispatch_dpc_queue();
void preempt_thread();
krnlapi void dispatch();
krnlapi int system_idle();

void init_sched();

__inline void check_dpc_queue() {
  if (dpc_queue_head) dispatch_dpc_queue();
}

__inline void check_preempt() {
#ifndef NOPREEMPTION
  if (preempt) preempt_thread();
#endif
}

__inline int signals_ready(struct thread *t) {
  return t->pending_signals & ~t->blocked_signals;
}

#endif
