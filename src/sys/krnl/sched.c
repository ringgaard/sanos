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

#define DEFAULT_STACK_SIZE           (1 * 1024 * 1024)
#define DEFAULT_INITIAL_STACK_COMMIT (8 * 1024)

int preempt = 0;
int in_dpc = 0;
unsigned long dpc_time = 0;
unsigned long dpc_total = 0;
unsigned long dpc_lost = 0;
unsigned long thread_ready_summary = 0;

struct thread *idle_thread;
struct thread *ready_queue_head[THREAD_PRIORITY_LEVELS];
struct thread *ready_queue_tail[THREAD_PRIORITY_LEVELS];
struct thread *threadlist;

struct dpc *dpc_queue_head;
struct dpc *dpc_queue_tail;

struct task *idle_tasks_head;
struct task *idle_tasks_tail;

struct task_queue sys_task_queue;

static void tmr_alarm(void *arg);

__declspec(naked) struct thread *self() {
  __asm {
    mov eax, esp
    and eax, TCBMASK
    ret
  }
}

__declspec(naked) void switch_context(struct thread *t) {
  __asm {
    // Save registers on current kernel stack
    push    ebp
    push    ebx
    push    edi
    push    esi

    // Store kernel stack pointer in tcb
    mov     eax, esp
    and     eax, TCBMASK
    add     eax, TCBESP
    mov     [eax], esp

    // Get stack pointer for new thread and store in esp0
    mov     eax, 20[esp]
    add     eax, TCBESP
    mov     esp, [eax]
    mov     ebp, TSS_ESP0
    mov     [ebp], eax

    // Restore registers from new kernel stack
    pop     esi
    pop     edi
    pop     ebx
    pop     ebp

    ret
  }
}

static void insert_before(struct thread *t1, struct thread *t2) {
  t2->next = t1;
  t2->prev = t1->prev;
  t1->prev->next = t2;
  t1->prev = t2;
}

static void insert_after(struct thread *t1, struct thread *t2) {
  t2->next = t1->next;
  t2->prev = t1;
  t1->next->prev = t2;
  t1->next = t2;
}

static void remove(struct thread *t) {
  t->next->prev = t->prev;
  t->prev->next = t->next;
}

static void insert_ready_head(struct thread *t) {
  if (!ready_queue_head[t->priority]) {
    t->next_ready = t->prev_ready = NULL;
    ready_queue_head[t->priority] = ready_queue_tail[t->priority] = t;
    thread_ready_summary |= (1 << t->priority);
  } else {
    t->next_ready = ready_queue_head[t->priority];
    t->prev_ready = NULL;
    t->next_ready->prev_ready = t;
    ready_queue_head[t->priority] = t;
  }
}

static void insert_ready_tail(struct thread *t) {
  if (!ready_queue_tail[t->priority]) {
    t->next_ready = t->prev_ready = NULL;
    ready_queue_head[t->priority] = ready_queue_tail[t->priority] = t;
    thread_ready_summary |= (1 << t->priority);
  } else {
    t->next_ready = NULL;
    t->prev_ready = ready_queue_tail[t->priority];
    t->prev_ready->next_ready = t;
    ready_queue_tail[t->priority] = t;
  }
}

static void remove_from_ready_queue(struct thread *t) {
  if (t->next_ready) t->next_ready->prev_ready = t->prev_ready;
  if (t->prev_ready) t->prev_ready->next_ready = t->next_ready;
  if (t == ready_queue_head[t->priority]) ready_queue_head[t->priority] = t->next_ready;
  if (t == ready_queue_tail[t->priority]) ready_queue_tail[t->priority] = t->prev_ready;
  if (!ready_queue_tail[t->priority]) thread_ready_summary &= ~(1 << t->priority);
}

static void init_thread_stack(struct thread *t, void *startaddr, void *arg) {
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

void enter_wait(int reason) {
  struct thread *t = self();

  t->state = THREAD_STATE_WAITING;
  t->wait_reason = reason;
  dispatch();
}

int enter_alertable_wait(int reason) {
  struct thread *t = self();

  if (signals_ready(t)) return -EINTR;

  t->state = THREAD_STATE_WAITING;
  t->wait_reason = reason;
  t->flags &= ~THREAD_INTERRUPTED;

  t->flags |= THREAD_ALERTABLE;
  dispatch();
  t->flags &= ~THREAD_ALERTABLE;

  if (t->flags & THREAD_INTERRUPTED) {
    t->flags &= ~THREAD_INTERRUPTED;
    return -EINTR;
  }

  return 0;
}

int interrupt_thread(struct thread *t) {
  if (t->state != THREAD_STATE_WAITING) return -EBUSY;
  if ((t->flags & THREAD_ALERTABLE) == 0) return -EPERM;
  t->flags |= THREAD_INTERRUPTED;
  mark_thread_ready(t, 1, 1);
  return 0;
}

void mark_thread_ready(struct thread *t, int charge, int boost) {
  int prio = t->priority;
  int newprio;

  // Check for suspended thread that is now ready to run 
  if (t->suspend_count > 0) {
    t->state = THREAD_STATE_SUSPENDED;
    return;
  }

  // If thread has been interrupted it is already ready
  if ((t->flags & THREAD_INTERRUPTED) !=0 && t->state == THREAD_STATE_READY) return;

  // Charge quantums units each time a thread is restarted
  t->quantum -= charge;

  // Add boost to threads dynamic priority
  // Boosting is applied to the threads base priority
  // Boosting must never move threads to the realtime range
  // Boosting must never decrease the dynamic priority
  newprio = t->base_priority + boost;
  if (newprio > PRIORITY_TIME_CRITICAL) newprio = PRIORITY_TIME_CRITICAL;
  if (newprio > t->priority) t->priority = newprio;

  // Set thread state to ready
  if (t->state == THREAD_STATE_READY) panic("thread already ready");
  t->state = THREAD_STATE_READY;

  // Insert thread in ready queue
  if (t->quantum > 0) {
    // Thread has some quantum left. Insert it at the head of the
    // ready queue for its priority.
    insert_ready_head(t);
  } else {
    // The thread has exhausted its CPU quantum. Assign a new quantum 
    t->quantum = DEFAULT_QUANTUM;
    
    // Let priority decay towards base priority
    if (t->priority > t->base_priority) t->priority--;

    // Insert it at the end of the ready queue for its priority.
    insert_ready_tail(t);
  }

  // Signal preemption if new ready thread has priority over the running thread
  if (t->priority > self()->priority) preempt = 1;
}

void preempt_thread() {
  struct thread *t = self();

  // Enable interrupt in case we have been called in interupt context
  sti();

  // Count number of preempted context switches
  t->preempts++;

  // Assign a new quantum if quantum expired
  if (t->quantum <= 0)  {
    t->quantum = DEFAULT_QUANTUM;

    // Let priority decay towards base priority
    if (t->priority > t->base_priority) t->priority--;
  }

  // Thread is ready to run 
  t->state = THREAD_STATE_READY;

  // Insert thread at the end of the ready queue for its priority.
  insert_ready_tail(t);

  // Relinquish CPU
  dispatch();
}

void mark_thread_running() {
  struct thread *t;
  struct tib *tib;

  // Set thread state to running
  t = self();
  t->state = THREAD_STATE_RUNNING;
  t->context_switches++;

  // Set FS register to point to current TIB
  tib = t->tib;
  if (tib) {
    // Update TIB descriptor
#ifdef VMACH
    set_gdt_entry(GDT_TIB, (unsigned long) tib, PAGESIZE, D_DATA | D_DPL3 | D_WRITE | D_PRESENT, 0);
#else
    struct segment *seg;

    seg = &syspage->gdt[GDT_TIB];
    seg->base_low = (unsigned short)((unsigned long) tib & 0xFFFF);
    seg->base_med = (unsigned char)(((unsigned long) tib >> 16) & 0xFF);
    seg->base_high = (unsigned char)(((unsigned long) tib >> 24) & 0xFF);
#endif

    // Reload FS register
    __asm {
      mov ax, SEL_TIB + SEL_RPL3
      mov fs, ax
    }
  }
}

void kernel_thread_start(void *arg) {
  struct thread *t = self();

  // Mark thread as running
  mark_thread_running();

  // Call entry point
  ((void (*)(void *)) (t->entrypoint))(arg);
}

void user_thread_start(void *arg) {
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
  __asm {
    mov eax, stacktop
    mov ebx, entrypoint

    push SEL_UDATA + SEL_RPL3
    push eax
    pushfd
    push SEL_UTEXT + SEL_RPL3
    push ebx
    mov ax, SEL_UDATA + SEL_RPL3
    mov ds, ax
    mov es, ax
    IRETD
  }
}

static struct thread *create_thread(threadproc_t startaddr, void *arg, int priority) {
  // Allocate a new aligned thread control block
  struct thread *t = (struct thread *) alloc_pages_align(PAGES_PER_TCB, PAGES_PER_TCB, 'TCB');
  if (!t) return NULL;
  memset(t, 0, PAGES_PER_TCB * PAGESIZE);
  init_thread(t, priority);

  // Initialize the thread kernel stack to start executing the task function
  init_thread_stack(t, (void *) startaddr, arg);

  // Add thread to thread list
  insert_before(threadlist, t);

  // Signal preemption if new ready thread has priority over the running thread
  if (t->priority > self()->priority) preempt = 1;

  return t;
}

struct thread *create_kernel_thread(threadproc_t startaddr, void *arg, int priority, char *name) {
  struct thread *t;

  // Create new thread object
  t = create_thread(kernel_thread_start, arg, priority);
  if (!t) return NULL;
  if (name) strncpy(t->name, name, THREAD_NAME_LEN - 1);
  t->entrypoint = startaddr;

  // Mark thread as ready to run
  mark_thread_ready(t, 0, 0);

  // Notify debugger
  dbg_notify_create_thread(t, startaddr);

  return t;
}

int create_user_thread(void *entrypoint, unsigned long stacksize, char *name, struct thread **retval) {
  struct thread *creator = self();
  struct thread *t;
  int rc;

  // Determine stacksize
  if (stacksize == 0) {
    stacksize = DEFAULT_STACK_SIZE;
  } else {
    stacksize = PAGES(stacksize) * PAGESIZE;
  }

  // Create and initialize new TCB and suspend thread
  t = create_thread(user_thread_start, NULL, PRIORITY_NORMAL);
  if (!t) return -ENOMEM;
  if (name) strncpy(t->name, name, THREAD_NAME_LEN - 1);
  t->suspend_count++;

  // Inherit effective user and group from creator thread
  t->ruid = t->euid = creator->euid;
  t->rgid = t->egid = creator->egid;
  t->ngroups = creator->ngroups;
  memcpy(t->groups, creator->groups, creator->ngroups * sizeof(gid_t));
  strcpy(t->curdir, creator->curdir);

  // Create and initialize new TIB
  rc = init_user_thread(t, entrypoint);
  if (rc < 0) return rc;

  // Allocate user stack
  rc = allocate_user_stack(t, stacksize, DEFAULT_INITIAL_STACK_COMMIT);
  if (rc < 0) return rc;

  // Allocate self handle (it is also stored in the tib for fast access)
  t->hndl = t->tib->hndl = halloc(&t->object);

  // Protect handle from being closed
  hprotect(t->hndl);

  // Initialize signal alarm timer
  init_timer(&t->alarm, tmr_alarm, t);

  // Notify debugger
  dbg_notify_create_thread(t, entrypoint);

  *retval = t;
  return 0;
}

int init_user_thread(struct thread *t, void *entrypoint) {
  struct tib *tib;

  // Allocate and initialize thread information block for thread
  tib = vmalloc(NULL, sizeof(struct tib), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE, 'TIB', NULL);
  if (!tib) return -ENOMEM;

  t->entrypoint = entrypoint;
  t->tib = tib;
  tib->self = tib;
  tib->tlsbase = &tib->tls;
  tib->pid = 1;
  tib->tid = t->id;
  tib->peb = peb;
  tib->except = (struct xcptrec *) 0xFFFFFFFF;

  return 0;
}

int allocate_user_stack(struct thread *t, unsigned long stack_reserve, unsigned long stack_commit) {
  char *stack;
  struct tib *tib;

  stack = vmalloc(NULL, stack_reserve, MEM_RESERVE, PAGE_READWRITE, 'STK', NULL);
  if (!stack) return -ENOMEM;

  tib = t->tib;
  tib->stackbase = stack;
  tib->stacktop = stack + stack_reserve;
  tib->stacklimit = stack + (stack_reserve - stack_commit);

  if (!vmalloc(tib->stacklimit, stack_commit, MEM_COMMIT, PAGE_READWRITE, 'STK', NULL)) return -ENOMEM;
  if (vmprotect(tib->stacklimit, PAGESIZE, PAGE_READWRITE | PAGE_GUARD) < 0) return -ENOMEM;

  return 0;
}

static void destroy_tcb(void *arg) {
  // Deallocate TCB
  free_pages(arg, PAGES_PER_TCB);

  // Set the TASK_QUEUE_ACTIVE_TASK_INVALID flag, to inform the sys task queue that the
  // executing task is invalid. The destroy_tcb task is placed in the TCB, and we dont want
  // the flag to be updated after this task finish executing, because we have deallocated the
  // task.
  sys_task_queue.flags |= TASK_QUEUE_ACTIVE_TASK_INVALID;
}

int destroy_thread(struct thread *t) {
  struct task *task;

  // We can only remove terminated threads
  if (t->state != THREAD_STATE_TERMINATED) panic("thread terminated in invalid state");
 
  // Deallocate user context
  if (t->tib) {
    // Deallocate user stack
    if (t->tib->stackbase) {
      vmfree(t->tib->stackbase, (char *) (t->tib->stacktop) - (char *) (t->tib->stackbase), MEM_RELEASE);
      t->tib->stackbase = NULL;
    }

    // Deallocate TIB
    vmfree(t->tib, sizeof(struct tib), MEM_RELEASE);
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

struct thread *get_thread(tid_t tid) {
  struct thread *t = threadlist;

  while (1) {
    if (t->id == tid) return t;
    t = t->next;
    if (t == threadlist) return NULL;
  }
}

int suspend_thread(struct thread *t) {
  int prevcount = t->suspend_count++;

  if (prevcount == 0) {
    if (t->state == THREAD_STATE_READY) {
      remove_from_ready_queue(t);
      t->state = THREAD_STATE_SUSPENDED;
    } else if (t->state == THREAD_STATE_RUNNING) {
      t->state = THREAD_STATE_SUSPENDED;
      dispatch();
    }
  }

  return prevcount;
}

int resume_thread(struct thread *t) {
  int prevcount = t->suspend_count;

  if (t->suspend_count > 0) {
    if (--t->suspend_count == 0)  {
      if (t->state == THREAD_STATE_SUSPENDED || t->state == THREAD_STATE_INITIALIZED) mark_thread_ready(t, 0, 0);
    }
  }

  return prevcount;
}

void suspend_all_user_threads() {
  struct thread *t = threadlist;
  while (1) {
    if (t->tib && t != self()) suspend_thread(t);
    t = t->next;
    if (t == threadlist) break;
  }
}

static void tmr_alarm(void *arg) {
  struct thread *t = arg;
  send_user_signal(t, SIGALRM);
}

int schedule_alarm(unsigned int seconds) {
  struct thread *t = self();
  int rc = 0;

  if (t->alarm.active) rc = (t->alarm.expires - ticks) / HZ;
  if (seconds == 0) {
    del_timer(&t->alarm);
  } else {
    mod_timer(&t->alarm, ticks + seconds * HZ);
  }

  return rc;
}

void terminate_thread(int exitcode) {
  struct thread *t = self();
  t->state = THREAD_STATE_TERMINATED;
  t->exitcode = exitcode;
  del_timer(&t->alarm);
  exit_thread(t);
  hunprotect(t->hndl);
  hfree(t->hndl);
  dispatch();
}

int get_thread_priority(struct thread *t) {
  return t->priority;
}

int set_thread_priority(struct thread *t, int priority) {
  if (priority < 0 || priority >= THREAD_PRIORITY_LEVELS) return -EINVAL;
  if (t->base_priority == priority) return 0;

  if (t == self()) {
    // Thread changed priority for it self, reschedule if new priority lower
    if (priority < t->priority)  {
      t->base_priority = t->priority = priority;
      mark_thread_ready(t, 0, 0);
      dispatch();
    } else {
      t->base_priority = t->priority = priority;
    }
  } else {
    // If thread is ready to run, remove it from the current ready queue 
    // and insert the ready queue for the new priority
    if (t->state == THREAD_STATE_READY) {
      remove_from_ready_queue(t);
      t->base_priority = t->priority = priority;
      t->state = THREAD_STATE_TRANSITION;
      mark_thread_ready(t, 0, 0);
    } else {
      t->base_priority = t->priority = priority;
    }
  }

  return 0;
}

static void task_queue_task(void *tqarg) {
  struct task_queue *tq = tqarg;
  struct task *task;
  taskproc_t proc;
  void *arg;

  while (1) {
    // Wait until tasks arrive on the task queue
    while (tq->head == NULL) {
      enter_wait(THREAD_WAIT_TASK);
    }

    // Get next task from task queue
    task = tq->head;
    tq->head = task->next;
    if (tq->tail == task) tq->tail = NULL;
    tq->size--;

    // Execute task
    task->flags &= ~TASK_QUEUED;
    if ((task->flags & TASK_EXECUTING) == 0) {
      task->flags |= TASK_EXECUTING;
      proc = task->proc;
      arg = task->arg;
      tq->flags |= TASK_QUEUE_ACTIVE;
  
      proc(arg);

      if (!(tq->flags & TASK_QUEUE_ACTIVE_TASK_INVALID)) task->flags &= ~TASK_EXECUTING;
      tq->flags &= ~(TASK_QUEUE_ACTIVE | TASK_QUEUE_ACTIVE_TASK_INVALID);
    }
  }
}

int init_task_queue(struct task_queue *tq, int priority, int maxsize, char *name) {
  memset(tq, 0, sizeof(struct task_queue));
  tq->maxsize = maxsize;
  tq->thread = create_kernel_thread(task_queue_task, tq, priority, name);

  return 0;
}

void init_task(struct task *task) {
  task->proc = NULL;
  task->arg = NULL;
  task->next = NULL;
  task->flags = 0;
}

int queue_task(struct task_queue *tq, struct task *task, taskproc_t proc, void *arg) {
  if (!tq) tq = &sys_task_queue;
  if (task->flags & TASK_QUEUED) return -EBUSY;
  if (tq->maxsize != INFINITE && tq->size >= tq->maxsize) return -EAGAIN;

  task->proc = proc;
  task->arg = arg;
  task->next = NULL;
  task->flags |= TASK_QUEUED;

  if (tq->tail) {
    tq->tail->next = task;
    tq->tail = task;
  } else {
    tq->head = tq->tail = task;
  }

  tq->size++;

  if ((tq->flags & TASK_QUEUE_ACTIVE) == 0 && tq->thread->state == THREAD_STATE_WAITING) {
    mark_thread_ready(tq->thread, 0, 0);
  }

  return 0;
}

void init_dpc(struct dpc *dpc) {
  dpc->proc = NULL;
  dpc->arg = NULL;
  dpc->next = NULL;
  dpc->flags = 0;
}

void queue_irq_dpc(struct dpc *dpc, dpcproc_t proc, void *arg) {
  if (dpc->flags & DPC_QUEUED) {
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
}

void queue_dpc(struct dpc *dpc, dpcproc_t proc, void *arg) {
  cli();
  queue_irq_dpc(dpc, proc, arg);
  sti();
}

struct dpc *get_next_dpc() {
  struct dpc *dpc;

  cli();

  if (dpc_queue_head) {
    dpc = dpc_queue_head;
    dpc_queue_head = dpc->next;
    if (dpc_queue_tail == dpc) dpc_queue_tail = NULL;
  } else {
    dpc = NULL;
  }

  sti();

  return dpc;
}

void dispatch_dpc_queue() {
  struct dpc *dpc;
  dpcproc_t proc;
  void *arg;

  if (in_dpc) panic("sched: nested execution of dpc queue");
  in_dpc = 1;

  while (1) {
    // Get next deferred procedure call
    // As a side effect this will enable interrupts
    dpc = get_next_dpc();
    if (!dpc) break;

    // Execute DPC
    proc = dpc->proc;
    arg = dpc->arg;
    clear_bit(&dpc->flags, DPC_QUEUED_BIT);
 
    if ((dpc->flags & DPC_EXECUTING) == 0) {
      set_bit(&dpc->flags, DPC_EXECUTING_BIT);
      proc(arg);
      clear_bit(&dpc->flags, DPC_EXECUTING_BIT);
      dpc_total++;
    }
 
#ifdef RANDOMDEV
    if ((dpc->flags & DPC_NORAND) == 0) add_dpc_randomness(dpc);
#endif
  }

  in_dpc = 0;
}

static struct thread *find_ready_thread() {
  int prio;
  struct thread *t;

  // Find highest priority non-empty ready queue
  if (thread_ready_summary == 0) return NULL;
  prio = find_highest_bit(thread_ready_summary);

  // Remove thread from ready queue
  t = ready_queue_head[prio];
  if (!t->next_ready)  {
    ready_queue_head[prio] = ready_queue_tail[prio] = NULL;
    thread_ready_summary &= ~(1 << prio);
  } else {
    t->next_ready->prev_ready = NULL;
    ready_queue_head[prio] = t->next_ready;
  }

  t->next_ready = NULL;
  t->prev_ready = NULL;

  return t;
}

void dispatch() {
  struct thread *curthread = self();
  struct thread *t;

  // Clear preemption flag
  preempt = 0;

  // Execute all queued DPCs
  check_dpc_queue();

  // Find next thread to run
  t = find_ready_thread();
  if (!t) panic("No thread ready to run");

  // If current thread has been selected to run again then just return
  if (t == curthread)  {
    t->state = THREAD_STATE_RUNNING;
    return;
  }

  // Save fpu state if fpu has been used
  if (curthread->flags & THREAD_FPU_ENABLED) {
    fpu_disable(&curthread->fpustate);
    t->flags &= ~THREAD_FPU_ENABLED;
  }

  // Switch to new thread
  switch_context(t);

#ifdef VMACH
  switch_kernel_stack();
#endif

  // Mark new thread as running
  mark_thread_running();
}

void yield() {
  struct thread *t = self();

  // Give up remaining quantum
  t->quantum = 0;

  // Mark thread as ready to run
  mark_thread_ready(t, 0, 0);

  // Dispatch next thread
  dispatch();
}

int system_idle() {
  if (thread_ready_summary != 0) return 0;
  if (dpc_queue_head != NULL) return 0;
  return 1;
}

void add_idle_task(struct task *task, taskproc_t proc, void *arg) {
  task->proc = proc;
  task->arg = arg;
  task->next = NULL;
  task->flags |= TASK_QUEUED;

  if (idle_tasks_tail) {
    idle_tasks_tail->next = task;
    idle_tasks_tail = task;
  } else {
    idle_tasks_head = idle_tasks_tail = task;
  }
}

void idle_task() {
  struct thread *t = self();

  while (1) {
    struct task *task = idle_tasks_head;
    if (task) {
      while (task) {
        if (!system_idle()) break;
        task->flags |= TASK_EXECUTING;
        task->proc(task->arg);
        task->flags &= ~TASK_EXECUTING;
        task = task->next;
      }
    } else if (system_idle()) {
      halt();
    }

    mark_thread_ready(t, 0, 0);
    dispatch();

    //if ((eflags() & EFLAG_IF) == 0) panic("sched: interrupts disabled in idle loop");
  }
}

static int threads_proc(struct proc_file *pf, void *arg) {
  static char *threadstatename[] = {"init", "ready", "run", "wait", "term", "susp", "trans"};
  static char *waitreasonname[] = {"wait", "fileio", "taskq", "sockio", "sleep", "pipe", "devio"};
  struct thread *t = threadlist;
  char *state;
  unsigned long stksiz;

  pprintf(pf, "tid tcb      hndl state  prio s #h   user kernel ctxtsw stksiz name\n");
  pprintf(pf, "--- -------- ---- ------ ---- - -- ------ ------ ------ ------ --------------\n");
  while (1) {
    if (t->state == THREAD_STATE_WAITING) {
      state = waitreasonname[t->wait_reason];
    } else {
      state = threadstatename[t->state];
    }

    if (t->tib) {
      stksiz = (char *) (t->tib->stacktop) - (char *) (t->tib->stacklimit);
    } else {
      stksiz = 0;
    }

    pprintf(pf,"%3d %p %4d %-6s %2d%+2d %1d %2d%7d%7d%7d%6dK %s\n",
            t->id, t, t->hndl, state, t->base_priority, t->priority - t->base_priority, 
            t->suspend_count, t->object.handle_count, 
            t->utime, t->stime, t->context_switches,
            stksiz / 1024,
            t->name);

    t = t->next;
    if (t == threadlist) break;
  }

  return 0;
}

static int dpcs_proc(struct proc_file *pf, void *arg) {
  pprintf(pf, "dpc time   : %8d\n", dpc_time);
  pprintf(pf, "total dpcs : %8d\n", dpc_total);
  pprintf(pf, "lost dpcs  : %8d\n", dpc_lost);

  return 0;
}

void init_sched() {
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
  idle_thread->priority = PRIORITY_SYSIDLE;
  idle_thread->state = THREAD_STATE_RUNNING;
  idle_thread->next = idle_thread;
  idle_thread->prev = idle_thread;
  strcpy(idle_thread->name, "idle");
  thread_ready_summary = (1 << PRIORITY_SYSIDLE);

  // Initialize system task queue
  init_task_queue(&sys_task_queue, PRIORITY_NORMAL /*PRIORITY_SYSTEM*/, INFINITE, "systask");

  // Register /proc/threads and /proc/dpcs
  register_proc_inode("threads", threads_proc, NULL);
  register_proc_inode("dpcs", dpcs_proc, NULL);
}
