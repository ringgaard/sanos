#include <os.h>
#include <syscall.h>

__inline long atomic_add(long *dest, long add_value)
 {
  __asm 
  {
    mov edx, dest;
    mov eax, add_value;
    mov ecx, eax;
    lock xadd dword ptr [edx], eax;
    add eax, ecx;
  }
}

void init_critsect(critsect_t cs)
{
  cs->count = -1;
  cs->recursion = 0;
  cs->owner = NOHANDLE;
  cs->event = create_event(0, 0);
}

void delete_critsect(critsect_t cs)
{
  close_handle(cs->event);
}

void enter_critsect(critsect_t cs)
{
  handle_t self = get_current_thread();

  if (cs->owner == self)
  {
    cs->recursion++;
  }
  else 
  {    
    if (atomic_add(&cs->count, 1) > 0)  wait_for_object(cs->event, INFINITE);
    cs->owner = self;
  }
}

void leave_critsect(critsect_t cs)
{
  handle_t self = get_current_thread();

  if (cs->owner != self) return;
  if (cs->recursion > 0)
  {
    cs->recursion--;
  }
  else
  {
    cs->owner = NOHANDLE;
    if (atomic_add(&cs->count, -1) >= 0) set_event(cs->event);
  }
}
