//
// sched.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Task scheduler
//

#include <os/krnl.h>

#define DEFAULT_STACK_SIZE (1 * M)

int resched = 0;
int idle = 0;
int in_dpc = 0;
unsigned long dpc_time = 0;
unsigned long dpc_total = 0;

struct thread *idle_thread;
struct thread *ready_queue_head[THREAD_PRIORITY_LEVELS];
struct thread *ready_queue_tail[THREAD_PRIORITY_LEVELS];

struct thread *threadlist;
struct dpc *dpc_queue;

struct task_queue sys_task_queue;

__declspec(naked) void context_switch(struct thread *t)
{
  __asm
  {
    // Save register on current kernel stack
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

    // Restore register from new kernel stack
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
  t->state = THREAD_STATE_READY;
  t->next_ready = NULL;

  if (ready_queue_tail[t->priority] != NULL) ready_queue_tail[t->priority]->next_ready = t;
  if (ready_queue_head[t->priority] == NULL) ready_queue_head[t->priority] = t;
  ready_queue_tail[t->priority] = t;
  resched = 1;
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
  struct thread *t = (struct thread *) alloc_pages_align(PAGES_PER_TCB, PAGES_PER_TCB);
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
  tib = mmap(NULL, sizeof(struct tib), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
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

  stack = mmap(NULL, stack_reserve, MEM_RESERVE, PAGE_READWRITE);
  if (!stack) return -ENOMEM;

  tib = t->tib;
  tib->stackbase = stack;
  tib->stacktop = stack + stack_reserve;
  tib->stacklimit = stack + (stack_reserve - stack_commit);

  if (!mmap(tib->stacklimit, stack_commit, MEM_COMMIT, PAGE_READWRITE)) return -ENOMEM;
  if (!mmap(tib->stackbase, stack_reserve - stack_commit, MEM_COMMIT, PAGE_READWRITE | PAGE_GUARD)) return -ENOMEM;

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
  if (dpc->flags & DPC_QUEUED) return;

  dpc->proc = proc;
  dpc->arg = arg;
  dpc->next = dpc_queue;
  dpc_queue = dpc;
  dpc->flags |= DPC_QUEUED;
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

  if (in_dpc) panic("sched: nested execution of dpc queue");

  while (1)
  {
    // Get next deferred procedure call
    cli();
    if (dpc_queue)
    {
      dpc = dpc_queue;
      dpc_queue = dpc->next;
    }
    else
      dpc = NULL;
    sti();
    if (!dpc) return;

    // Execute DPC
    dpc->flags &= ~DPC_QUEUED;
    if ((dpc->flags & DPC_EXECUTING) == 0)
    {
      dpc->flags |= DPC_EXECUTING;
      proc = dpc->proc;
      arg = dpc->arg;
      
      in_dpc = 1;
      proc(arg);
      in_dpc = 0;
      dpc_total++;

      dpc->flags &= ~DPC_EXECUTING;
    }
  }
}

void dispatch()
{
  int prio;
  struct thread *t;

  // Clear rescheduling flag
  resched = 0;

  // Execute all queued DPC's
  dispatch_dpc_queue();

  // Find next thread to run
  while (1)
  {
    prio = THREAD_PRIORITY_LEVELS - 1;
    while (ready_queue_head[prio] == 0 && prio > 0) prio--;
    t = ready_queue_head[prio];
    if (t == NULL) panic("No thread ready to run");

    // Remove thread from ready queue
    ready_queue_head[prio] = t->next_ready;
    if (t->next_ready == NULL) ready_queue_tail[prio] = NULL;
    t->next_ready = NULL;

    // Check for suspended thread
    if (t->suspend_count == 0) break;
  }

  // If current thread has been selected to run again then just return
  if (t == self()) return;

  // Save fpu state if fpu has been used
  if (self()->flags & THREAD_FPU_ENABLED)
  {
    fpu_disable(self()->fpustate);
    t->flags &= ~THREAD_FPU_ENABLED;
  }

  // Switch to new thread
  context_switch(t);

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
    //idle = 1;
    //halt();
    //idle = 0;

    if (resched)
    {
      mark_thread_ready(self());
      dispatch();
    }

    dispatch_dpc_queue();
  }
}

void update_thread_times(struct context *ctxt)
{
  struct thread *t;

  if (in_dpc)
    dpc_time++;
  else
  {
    t = self();
    if (ctxt->eip < OSBASE)
      t->utime++;
    else
      t->stime++;
  }
}

static int threads_proc(struct proc_file *pf, void *arg)
{
  static char *threadstatename[] = {"init", "ready", "run", "wait", "term"};
  static char *waitreasonname[] = {"wait", "fileio", "taskq", "sockio", "sleep"};
  struct thread *t = threadlist;
  char *state;

  pprintf(pf, "tid tcb      hndl state  prio tib      s #h  utime  stime ctxtsw name\n");
  pprintf(pf, "--- -------- ---- ------ ---- -------- - -- ------ ------ ------ ---------------\n");
  while (1)
  {
    if (t->state == THREAD_STATE_WAITING)
      state = waitreasonname[t->wait_reason];
    else
      state = threadstatename[t->state];

    pprintf(pf,"%3d %p %4d %-6s %3d  %p %1d %2d%7d%7d%7d %s\n",
            t->id, t, t->hndl, state, t->priority, t->tib, 
	    t->suspend_count, t->object.handle_count, t->utime, t->stime, t->context_switches, t->name ? t->name : "");

    t = t->next;
    if (t == threadlist) break;
  }

  return 0;
}

static int dpcs_proc(struct proc_file *pf, void *arg)
{
  pprintf(pf, "dpc time  : %8d\n", dpc_time);
  pprintf(pf, "total dpcs: %8d\n", dpc_total);
  return 0;
}

void init_sched()
{
  // Initialize scheduler
  resched = 0;
  dpc_queue = NULL;
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
