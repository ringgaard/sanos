//
// sched.c
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

#include <os/krnl.h>

#define DEFAULT_STACK_SIZE (1 * M)

int testidlecount = 0; // TEST
int stucked = 0;

int preempt = 0;
int in_dpc = 0;
unsigned long dpc_time = 0;
unsigned long dpc_total = 0;
unsigned long dpc_lost = 0;

struct thread *idle_thread;
struct thread *ready_queue_head[THREAD_PRIORITY_LEVELS];
struct thread *ready_queue_tail[THREAD_PRIORITY_LEVELS];
struct thread *threadlist;

struct dpc *dpc_queue_head;
struct dpc *dpc_queue_tail;

struct task_queue sys_task_queue;

__declspec(naked) void switch_context(struct thread *t)
{
  __asm
  {
    // Save registers on current kernel stack
    push    ebp
    push    ebx
    push    edi
    push    esi

    // Store kernel stack pointer in tcb
    mov	    eax, esp
    and	    eax, TCBMASK
    add	    eax, TCBESP
    mov	    [eax], esp

    // Get stack pointer for new thread and store in esp0
    mov	    eax, 20[esp]
    add	    eax, TCBESP
    mov	    esp, [eax]
    mov	    ebp, TSS_ESP0
    mov	    [ebp], eax

    // Restore registers from new kernel stack
    pop	    esi
    pop	    edi
    pop	    ebx
    pop	    ebp

    ret
  }
}

static void insert_before(struct thread *t1, struct thread *t2)
{
  t2->next = t1;
  t2->prev = t1->prev;
  t1->prev->next = t2;
  t1->prev = t2;
}

static void insert_after(struct thread *t1, struct thread *t2)
{
  t2->next = t1->next;
  t2->prev = t1;
  t1->next->prev = t2;
  t1->next = t2;
}

static void remove(struct thread *t)
{
  t->next->prev = t->prev;
  t->prev->next = t->next;
}

static void init_thread_stack(struct thread *t, void *startaddr, void *arg)
{
  struct tcb *tcb = (struct tcb *) t;
  unsigned long *esp = (unsigned long *) &tcb->esp;
  
  *--esp = (unsigned long) arg;
  *--esp = (unsigned long) 0;
  *--esp = (unsigned long) startaddr;
  *--esp = 0;
  *--esp = 0;
  *--esp = 0;
  *--esp = 0;
  tcb->esp = esp;
}

void enter_wait(int reason)
{
  struct thread *t = self();

  t->state = THREAD_STATE_WAITING;
  t->wait_reason = reason;
  dispatch();
}

void mark_thread_ready(struct thread *t)
{
  int prio = t->priority;

  // Set thread state to ready
  t->state = THREAD_STATE_READY;

  if (t->quantum > 0)
  {
    // Thread has some quantum left. Insert it at the head of the
    // ready queue for its priority.

    t->next_ready = ready_queue_head[prio];
    if (ready_queue_tail[prio] == NULL) ready_queue_tail[prio] = t;
    ready_queue_head[prio] = t;
  }
  else
  {
    // The thread has exhausted its CPU quantum. Assign a new quantum 
    // and insert it at the end of the ready queue for its priority.
    
    t->quantum = DEFAULT_QUANTUM;
    
    t->next_ready = NULL;
    if (ready_queue_tail[prio] != NULL) ready_queue_tail[prio]->next_ready = t;
    if (ready_queue_head[prio] == NULL) ready_queue_head[prio] = t;
    ready_queue_tail[prio] = t;
  }
}

void preempt_thread()
{
  struct thread *t = self();
  int prio = t->priority;

  // Enable interrupt in case we have been called in interupt context
  sti();

  // Count number of preemptions
  t->preempts++;

  // Thread is ready to run 
  t->state = THREAD_STATE_READY;

  // Assign a new quantum
  t->quantum = DEFAULT_QUANTUM;
    
  // Insert thread at the end of the ready queue for its priority.
  t->next_ready = NULL;
  if (ready_queue_tail[prio] != NULL) ready_queue_tail[prio]->next_ready = t;
  if (ready_queue_head[prio] == NULL) ready_queue_head[prio] = t;
  ready_queue_tail[prio] = t;

  // Relinquish CPU
  dispatch();
}

void mark_thread_running()
{
  struct thread *t;
  struct tib *tib;
  struct segment *seg;

  // Set thread state to running
  t = self();
  t->state = THREAD_STATE_RUNNING;
  t->context_switches++;

  // Set FS register to point to current TIB
  tib = t->tib;
  if (tib)
  {
    // Update descriptor
    seg = &syspage->gdt[GDT_TIB];
    seg->base_low = (unsigned short)((unsigned long) tib & 0xFFFF);
    seg->base_med = (unsigned char)(((unsigned long) tib >> 16) & 0xFF);
    seg->base_high = (unsigned char)(((unsigned long) tib >> 24) & 0xFF);

    // Reload FS register
    __asm
    {
      mov ax, SEL_TIB + SEL_RPL3
      mov fs, ax
    }
  }
}

void threadstart(void *arg)
{
  struct thread *t = self();
  unsigned long *stacktop;
  void *entrypoint;

  // Mark thread as running to reload fs register
  mark_thread_running();

  // Setup arguments on user stack
  stacktop = (unsigned long *) t->tib->stacktop;
  *(--stacktop) = (unsigned long) (t->tib);
  *(--stacktop) = 0;

  // Switch to usermode and start excuting thread routine
  entrypoint = t->entrypoint;
  __asm
  {
    mov eax, stacktop
    mov ebx, entrypoint

    push SEL_UDATA + SEL_RPL3
    push eax
    pushfd
    push SEL_UTEXT + SEL_RPL3
    push ebx
    iretd

    cli
    hlt
  }
}

static struct thread *create_thread(threadproc_t startaddr, void *arg, int priority)
{
  // Allocate a new aligned thread control block
  struct thread *t = (struct thread *) alloc_pages_align(PAGES_PER_TCB, PAGES_PER_TCB, 'TCB');
  if (!t) return NULL;
  memset(t, 0, PAGES_PER_TCB * PAGESIZE);
  init_thread(t, priority);

  // Initialize the thread kernel stack to start executing the task function
  init_thread_stack(t, (void *) startaddr, arg);

  // Add thread to thread list
  insert_before(threadlist, t);

  return t;
}

struct thread *create_kernel_thread(threadproc_t startaddr, void *arg, int priority, char *name)
{
  struct thread *t;

  // Create new thread object
  t = create_thread(startaddr, arg, priority);
  if (!t) return NULL;
  t->name = name;

  // Mark thread as ready to run
  mark_thread_ready(t);

  // Notify debugger
  dbg_notify_create_thread(t, startaddr);

  return t;
}

int create_user_thread(void *entrypoint, unsigned long stacksize, struct thread **retval)
{
  struct thread *t;
  int rc;

  // Determine stacksize
  if (stacksize == 0)
    stacksize = DEFAULT_STACK_SIZE;
  else
    stacksize = PAGES(stacksize) * PAGESIZE;

  // Create and initialize new TCB and suspend thread
  t = create_thread(threadstart, NULL, PRIORITY_NORMAL);
  if (!t) return -ENOMEM;
  t->name = "user";
  t->suspend_count++;

  // Create and initialize new TIB
  rc = init_user_thread(t, entrypoint);
  if (rc < 0) return rc;

  // Allocate user stack with one committed page
  rc = allocate_user_stack(t, stacksize, PAGESIZE);
  if (rc < 0) return rc;

  // Allocate self handle
  t->hndl = halloc(&t->object);

  // Notify debugger
  dbg_notify_create_thread(t, entrypoint);

  *retval = t;
  return 0;
}

int init_user_thread(struct thread *t, void *entrypoint)
{
  struct tib *tib;

  // Allocate and initialize thread information block for thread
  tib = mmap(NULL, sizeof(struct tib), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE, 'TIB');
  if (!tib) return -ENOMEM;

  t->entrypoint = entrypoint;
  t->tib = tib;
  tib->self = tib;
  tib->tlsbase = &tib->tls;
  tib->pid = 1;
  tib->tid = t->id;
  tib->peb = peb;

  return 0;
}

int allocate_user_stack(struct thread *t, unsigned long stack_reserve, unsigned long stack_commit)
{
  char *stack;
  struct tib *tib;

  stack = mmap(NULL, stack_reserve, MEM_RESERVE, PAGE_READWRITE, 'STK');
  if (!stack) return -ENOMEM;

  tib = t->tib;
  tib->stackbase = stack;
  tib->stacktop = stack + stack_reserve;
  tib->stacklimit = stack + (stack_reserve - stack_commit);

  if (!mmap(tib->stacklimit, stack_commit, MEM_COMMIT, PAGE_READWRITE, 'STK')) return -ENOMEM;
  if (!mmap(tib->stackbase, stack_reserve - stack_commit, MEM_COMMIT, PAGE_READWRITE | PAGE_GUARD, 'STK')) return -ENOMEM;

  return 0;
}

static void destroy_tcb(void *arg)
{
  // Deallocate TCB
  free_pages(arg, PAGES_PER_TCB);

  // Set the TASK_QUEUE_ACTIVE_TASK_INVALID flag, to inform the sys task queue that the
  // executing task is invalid. The destroy_tcb task is placed in the TCB, and we dont want
  // the flag to be updated after this task finish executing, because we have deallocated the
  // task.
  sys_task_queue.flags |= TASK_QUEUE_ACTIVE_TASK_INVALID;
}

int destroy_thread(struct thread *t)
{
  struct task *task;

  // We can only remove terminated threads
  if (t->state != THREAD_STATE_TERMINATED) panic("thread terminated in invalid state");
 
  // Deallocate user context
  if (t->tib)
  {
    // Deallocate user stack
    if (t->tib->stackbase) 
    {
      munmap(t->tib->stackbase, (char *) (t->tib->stacktop) - (char *) (t->tib->stackbase), MEM_RELEASE);
      t->tib->stackbase = NULL;
    }

    // Deallocate TIB
    munmap(t->tib, sizeof(struct tib), MEM_RELEASE);
    t->tib = NULL;
  }

  // Notify debugger
  dbg_notify_exit_thread(t);

  // Remove thread from thread list
  remove(t);

  // Add task to delete the TCB
  task = (struct task *) (t + 1);
  init_task(task);
  queue_task(&sys_task_queue, task, destroy_tcb, t);

  return 0;
}

struct thread *get_thread(tid_t tid)
{
  struct thread *t = threadlist;

  while (1)
  {
    if (t->id == tid) return t;
    t = t->next;
    if (t == threadlist) return NULL;
  }
}

int suspend_thread(struct thread *t)
{
  int prevcount = t->suspend_count;

  t->suspend_count++;
  return prevcount;
}

int resume_thread(struct thread *t)
{
  int prevcount = t->suspend_count;

  if (t->suspend_count > 0)
  {
    t->suspend_count--;
    if (t->suspend_count == 0) 
    {
      if (t->state == THREAD_STATE_READY || t->state == THREAD_STATE_INITIALIZED) mark_thread_ready(t);
    }
  }

  return prevcount;
}

void terminate_thread(int exitcode)
{
  struct thread *t = self();
  t->state = THREAD_STATE_TERMINATED;
  t->exitcode = exitcode;
  hfree(t->hndl);
  dispatch();
}

static void task_queue_task(void *tqarg)
{
  struct task_queue *tq = tqarg;
  struct task *task;
  taskproc_t proc;
  void *arg;

  while (1)
  {
    // Wait until tasks arrive on the task queue
    while (tq->head == NULL) enter_wait(THREAD_WAIT_TASK);

    // Get next task from task queue
    task = tq->head;
    tq->head = task->next;
    if (tq->tail == task) tq->tail = NULL;
    tq->size--;

    // Execute task
    task->flags &= ~TASK_QUEUED;
    if ((task->flags & TASK_EXECUTING) == 0)
    {
      task->flags |= TASK_EXECUTING;
      proc = task->proc;
      arg = task->arg;
      tq->flags |= TASK_QUEUE_ACTIVE;
  
      proc(arg);

      if (!(tq->flags & TASK_QUEUE_ACTIVE_TASK_INVALID)) task->flags &= ~TASK_EXECUTING;
      tq->flags &= ~(TASK_QUEUE_ACTIVE | TASK_QUEUE_ACTIVE_TASK_INVALID);

      yield();
    }
  }
}

int init_task_queue(struct task_queue *tq, int priority, int maxsize, char *name)
{
  memset(tq, 0, sizeof(struct task_queue));
  tq->maxsize = maxsize;
  tq->thread = create_kernel_thread(task_queue_task, tq, priority, name);

  return 0;
}

void init_task(struct task *task)
{
  task->proc = NULL;
  task->arg = NULL;
  task->next = NULL;
  task->flags = 0;
}

int queue_task(struct task_queue *tq, struct task *task, taskproc_t proc, void *arg)
{
  if (!tq) tq = &sys_task_queue;
  if (task->flags & TASK_QUEUED) return -EBUSY;
  if (tq->maxsize != INFINITE && tq->size >= tq->maxsize) return -EAGAIN;

  task->proc = proc;
  task->arg = arg;
  task->next = NULL;
  task->flags |= TASK_QUEUED;

  if (tq->tail)
  {
    tq->tail->next = task;
    tq->tail = task;
  }
  else
    tq->head = tq->tail = task;

  tq->size++;

  if ((tq->flags & TASK_QUEUE_ACTIVE) == 0 && tq->thread->state == THREAD_STATE_WAITING)
  {
    mark_thread_ready(tq->thread);
  }

  return 0;
}

void init_dpc(struct dpc *dpc)
{
  dpc->proc = NULL;
  dpc->arg = NULL;
  dpc->next = NULL;
  dpc->flags = 0;
}

void queue_irq_dpc(struct dpc *dpc, dpcproc_t proc, void *arg)
{
  if (dpc->flags & DPC_QUEUED) 
  {
    if (dpc_queue_head == NULL)
    {
      extern int debug_nointr;

      debug_nointr = 1;
      panic("bad dpc queue");
    }

    if (stucked) 
    {
      //kprintf("dpc flags %d next %p head %p tail %p\n", dpc->flags, dpc->next, dpc_queue_head, dpc_queue_tail);
      kprintf("?");
    }
    dpc_lost++;
    return;
  }

  dpc->proc = proc;
  dpc->arg = arg;
  dpc->next = NULL;
  if (dpc_queue_tail) dpc_queue_tail->next = dpc;
  dpc_queue_tail = dpc;
  if (!dpc_queue_head) dpc_queue_head = dpc;
  set_bit(&dpc->flags, DPC_QUEUED_BIT);
  if (stucked) kprintf(">");
}

void queue_dpc(struct dpc *dpc, dpcproc_t proc, void *arg)
{
  cli();
  queue_irq_dpc(dpc, proc, arg);
  sti();
}

void dispatch_dpc_queue()
{
  struct dpc *dpc;
  dpcproc_t proc;
  void *arg;

  if (stucked) kprintf("#");
  if (in_dpc) panic("sched: nested execution of dpc queue");
  in_dpc = 1;

  while (1)
  {
    // Get next deferred procedure call
    cli();
    if (dpc_queue_head)
    {
      dpc = dpc_queue_head;
      dpc_queue_head = dpc->next;
      if (dpc_queue_tail == dpc) dpc_queue_tail = NULL;
    }
    else
      dpc = NULL;
    sti();
    if (!dpc) break;

    // Execute DPC
    clear_bit(&dpc->flags, DPC_QUEUED_BIT);
    if ((dpc->flags & DPC_EXECUTING) == 0)
    {
      set_bit(&dpc->flags, DPC_EXECUTING_BIT);
      proc = dpc->proc;
      arg = dpc->arg;
      testidlecount = 0;

      proc(arg);

      dpc_total++;
      clear_bit(&dpc->flags, DPC_EXECUTING_BIT);
    }
  }

  in_dpc = 0;
}

void dispatch()
{
  int prio;
  struct thread *curthread = self();
  struct thread *t;

  // Clear preemption flag
  preempt = 0;

  // Execute all queued DPC's
  check_dpc_queue();

  // Find next thread to run
  while (1)
  {
    prio = THREAD_PRIORITY_LEVELS - 1;
    t = NULL;
    while ((t = ready_queue_head[prio]) == NULL && prio > 0) prio--;
    if (t == NULL) panic("No thread ready to run");

    // Remove thread from ready queue
    ready_queue_head[prio] = t->next_ready;
    if (t->next_ready == NULL) ready_queue_tail[prio] = NULL;
    t->next_ready = NULL;

    // Check for suspended thread
    if (t->suspend_count == 0) break;
  }

  // If current thread has been selected to run again then just return
  if (t == curthread) return;

  // Save fpu state if fpu has been used
  if (curthread->flags & THREAD_FPU_ENABLED)
  {
    fpu_disable(curthread->fpustate);
    t->flags &= ~THREAD_FPU_ENABLED;
  }

  // Switch to new thread
  switch_context(t);

  // Mark new thread as running
  mark_thread_running();
}

void yield()
{
  // Mark thread as ready to run
  mark_thread_ready(self());

  // Dispatch next thread
  dispatch();
}

void idle_task()
{
  while (1) 
  {
    mark_thread_ready(self());
    dispatch();

    if ((eflags() & EFLAG_IF) == 0) panic("sched: interrupts disabled in idle loop");

    {
      extern struct dpc timerdpc;
      extern int debug_nointr;

      if ((timerdpc.flags & 1) != 0 && dpc_queue_head == NULL)
      {
	debug_nointr = 1;
	panic("dpc queue corrupted");
      }
    }

    if (++testidlecount > 1000000) 
    {
      stucked = 1;
      kprintf("sched: kernel stucked\n");
      if (dpc_queue_head)
	kprintf("dpc_queue_head %p next %p flags %d\n", dpc_queue_head, dpc_queue_head->next, dpc_queue_head->flags);
      else
	kprintf("dpc queue empty\n");

      testidlecount = 0;
    }

    //check_dpc_queue();
    //sti();
    //halt();
  }
}

static int threads_proc(struct proc_file *pf, void *arg)
{
  static char *threadstatename[] = {"init", "ready", "run", "wait", "term"};
  static char *waitreasonname[] = {"wait", "fileio", "taskq", "sockio", "sleep"};
  struct thread *t = threadlist;
  char *state;
  unsigned long stksiz;

  pprintf(pf, "tid tcb      hndl state  prio s #h   user kernel ctxtsw stksiz name\n");
  pprintf(pf, "--- -------- ---- ------ ---- - -- ------ ------ ------ ------ --------------\n");
  while (1)
  {
    if (t->state == THREAD_STATE_WAITING)
      state = waitreasonname[t->wait_reason];
    else
      state = threadstatename[t->state];

    if (t->tib)
      stksiz = (char *) (t->tib->stacktop) - (char *) (t->tib->stacklimit);
    else
      stksiz = 0;

    pprintf(pf,"%3d %p %4d %-6s %3d  %1d %2d%7d%7d%7d%6dK %s\n",
            t->id, t, t->hndl, state, t->priority, 
	    t->suspend_count, t->object.handle_count, 
	    t->utime, t->stime, t->context_switches,
	    stksiz / K,
	    t->name ? t->name : "");

    t = t->next;
    if (t == threadlist) break;
  }

  return 0;
}

static int dpcs_proc(struct proc_file *pf, void *arg)
{
  pprintf(pf, "dpc time  : %8d\n", dpc_time);
  pprintf(pf, "total dpcs: %8d\n", dpc_total);
  pprintf(pf, "lost dpcs: %8d\n", dpc_lost);
  return 0;
}

void init_sched()
{
  // Initialize scheduler
  dpc_queue_head = dpc_queue_tail = NULL;
  memset(ready_queue_head, 0, sizeof(ready_queue_head));
  memset(ready_queue_tail, 0, sizeof(ready_queue_tail));

  // The initial kernel thread will later become the idle thread
  idle_thread = self();
  threadlist = idle_thread;

  // The idle thread is always ready to run
  memset(idle_thread, 0, sizeof(struct thread));
  idle_thread->object.type = OBJECT_THREAD;
  idle_thread->priority = PRIORITY_IDLE;
  idle_thread->state = THREAD_STATE_RUNNING;
  idle_thread->next = idle_thread;
  idle_thread->prev = idle_thread;
  idle_thread->name = "idle";

  // Initialize system task queue
  init_task_queue(&sys_task_queue, PRIORITY_NORMAL /*PRIORITY_SYSTEM*/, INFINITE, "systask");

  // Register /proc/threads and /proc/dpcs
  register_proc_inode("threads", threads_proc, NULL);
  register_proc_inode("dpcs", dpcs_proc, NULL);
}
