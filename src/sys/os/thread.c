//
// thread.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Thread routines
//

#include <os.h>

#include <os/syscall.h>
#include <os/seg.h>
#include <os/tss.h>
#include <os/syspage.h>

handle_t mkthread(void (__stdcall *startaddr)(struct tib *), unsigned long stacksize, struct tib **ptib)
{
  return syscall(SYSCALL_MKTHREAD, &startaddr);
}

void __stdcall threadstart(struct tib *tib)
{
  // Set usermode segment selectors
  __asm
  {
    mov	ax, SEL_UDATA + SEL_RPL3
    mov	ds, ax
    mov	es, ax
  }

  // Call thread routine
  ((void (__stdcall *)(void *)) (tib->startaddr))(tib->startarg);
  endthread(0);
}

handle_t beginthread(void (__stdcall *startaddr)(void *), unsigned stacksize, void *arg, int suspended, tid_t *tid)
{
  struct tib *tib;
  handle_t h;

  h = mkthread(threadstart, stacksize, &tib);
  if (h < 0) return h;

  tib->startaddr = (void *) startaddr;
  tib->startarg = arg;

  if (!suspended) resume(h);

  if (tid) *tid = tib->tid;
  return h;
}

int suspend(handle_t thread)
{
  return syscall(SYSCALL_SUSPEND, &thread);
}

int resume(handle_t thread)
{
  return syscall(SYSCALL_RESUME, &thread);
}

void endthread(int retval)
{
  syscall(SYSCALL_ENDTHREAD, &retval);
}

tid_t gettid()
{
  return gettib()->tid;
}

int getcontext(handle_t thread, void *context)
{
  return syscall(SYSCALL_GETCONTEXT, &thread);
}

int getprio(handle_t thread)
{
  return syscall(SYSCALL_GETPRIO, &thread);
}

int setprio(handle_t thread, int priority)
{
  return syscall(SYSCALL_SETPRIO, &thread);
}

void sleep(int millisecs)
{
  syscall(SYSCALL_SLEEP, &millisecs);
}

struct tib *gettib()
{
  struct tib *tib;

  __asm
  {
    mov eax, fs:[TIB_SELF_OFFSET]
    mov [tib], eax
  }

  return tib;
}

