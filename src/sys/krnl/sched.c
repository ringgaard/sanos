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
struct thread *idle_thread;
struct thread *ready_queue_head[THREAD_PRIORITY_LEVELS];
struct thread *ready_queue_tail[THREAD_PRIORITY_LEVELS];
struct dpc *dpc_queue;
struct thread *dead_tcb_queue;

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

__declspec(naked) struct thread *current_thread()
{
  __asm
  {
    mov eax, esp;
    and eax, TCBMASK
    ret
  }
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
  struct thread *self = current_thread();
  struct tib *tib = self->tib;
  struct segment *seg;

  // Set thread state to running
  self->state = THREAD_STATE_RUNNING;

  // Set FS register to point to current TIB
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
  struct thread *t = current_thread();
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

struct thread *create_task(taskproc_t task, void *arg, int priority)
{
  // Allocate a new aligned thread control block
  struct thread *t = (struct thread *) alloc_pages_align(PAGES_PER_TCB, PAGES_PER_TCB);
  if (!t) return NULL;
  memset(t, 0, PAGES_PER_TCB * PAGESIZE);
  init_thread(t, priority);

  // Initialize the thread kernel stack to start executing the task function
  init_thread_stack(t, (void *) task, arg);

  return t;
}

int create_thread(void *entrypoint, unsigned long stacksize, struct thread **retval)
{
  struct thread *t;
  int rc;

  // Determine stacksize
  if (stacksize == 0)
    stacksize = DEFAULT_STACK_SIZE;
  else
    stacksize = PAGES(stacksize) * PAGESIZE;

  // Create and initialize new TCB and suspend thread
  t = create_task(threadstart, NULL, PRIORITY_NORMAL);
  if (!t) return -ENOMEM;
  t->suspend_count++;

  // Create and initialize new TIB
  rc = init_user_thread(t, entrypoint);
  if (rc < 0) return rc;

  // Allocate user stack with one committed page
  rc = allocate_user_stack(t, stacksize, PAGESIZE);
  if (rc < 0) return rc;

  // Allocate self handle
  t->self = halloc(&t->object);

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

int destroy_thread(struct thread *t)
{
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

  // Insert the TCB in the dead tcb queue to be destroyed later by the idle thread
  t->next_ready = dead_tcb_queue;
  dead_tcb_queue = t;

  return 0;
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
      if (t->state != THREAD_STATE_TERMINATED) mark_thread_ready(t);
    }
  }

  return prevcount;
}

void terminate_thread(int exitcode)
{
  struct thread *t = current_thread();
  t->state = THREAD_STATE_TERMINATED;
  t->exitcode = exitcode;
  hfree(t->self);
  dispatch();
}

void init_dpc(struct dpc *dpc)
{
  dpc->proc = NULL;
  dpc->arg = NULL;
  dpc->next = NULL;
  dpc->active = 0;
}

void queue_dpc(struct dpc *dpc, dpcproc_t proc, void *arg)
{
  if (dpc->active) return;

  __asm { cli };
  dpc->proc = proc;
  dpc->arg = arg;
  dpc->next = dpc_queue;
  dpc->active = 1;
  dpc_queue = dpc;
  __asm { sti };
  resched = 1;
}

void queue_irq_dpc(struct dpc *dpc, dpcproc_t proc, void *arg)
{
  if (dpc->active) return;

  dpc->proc = proc;
  dpc->arg = arg;
  dpc->next = dpc_queue;
  dpc_queue = dpc;
  dpc->active = 1;
  resched = 1;
}

void dispatch_dpc_queue()
{
  struct dpc *queue;
  struct dpc *next;
  dpcproc_t proc;

  // Get list of queued deferred procedure calls
  __asm { cli };
  queue = dpc_queue;
  dpc_queue = NULL;
  __asm { sti };

  // Execute each DPC
  while (queue)
  {
    next = queue->next;
    proc = queue->proc;

    queue->active = 0;
    
    proc(queue->arg);
    queue = next;
  }
}

void dispatch()
{
  int prio;
  struct thread *t;
  struct thread *self = current_thread();

  // Clear rescheduling flag
  resched = 0;

  // Execute all queued DPC's
  dispatch_dpc_queue();

  // Find next thread to run
  prio = THREAD_PRIORITY_LEVELS - 1;
  while (ready_queue_head[prio] == 0 && prio > 0) prio--;
  t = ready_queue_head[prio];
  if (t == NULL) panic("No thread ready to run");

  // Remove thread from ready queue
  ready_queue_head[prio] = t->next_ready;
  if (t->next_ready == NULL) ready_queue_tail[prio] = NULL;
  t->next_ready = NULL;

  // If current thread has been selected to run again then just return
  if (t == self) return;

  // Save fpu state if fpu has been used
  if (self->flags & THREAD_FPU_ENABLED)
  {
    fpu_disable(self->fpustate);
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
  mark_thread_ready(current_thread());

  // Dispatch next thread
  dispatch();
}

static void collect_dead_tcbs()
{
  struct thread *t;

  while (dead_tcb_queue)
  {
    t = dead_tcb_queue;
    dead_tcb_queue = t->next_ready;
    free_pages(t, PAGES_PER_TCB);
  }
}

void idle_task()
{
  while (1) 
  {
    idle = 1;
    __asm { hlt };
    idle = 0;

    if (dead_tcb_queue) collect_dead_tcbs();

    if (resched)
    {
      mark_thread_ready(current_thread());
      dispatch();
    }
  }
}

void debug_intr_handler(struct context *ctxt, void *arg)
{
  //kprintf("debug intr\n");
}

void init_sched()
{
  // Initialize scheduler
  resched = 0;
  dpc_queue = NULL;
  dead_tcb_queue = 0;
  memset(ready_queue_head, 0, sizeof(ready_queue_head));
  memset(ready_queue_tail, 0, sizeof(ready_queue_tail));

  // The initial kernel thread will later become the idle thread
  idle_thread = current_thread();

  // The idle thread is always ready to run
  memset(idle_thread, 0, sizeof(struct thread));
  idle_thread->object.type = OBJECT_THREAD;
  idle_thread->priority = PRIORITY_IDLE;
  idle_thread->state = THREAD_STATE_RUNNING;

  set_interrupt_handler(INTR_DEBUG, debug_intr_handler, NULL);
}
