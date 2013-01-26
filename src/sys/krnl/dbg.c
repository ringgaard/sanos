//
// dbg.c
//
// Remote debugging support
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

#define DEBUGPORT         0x3F8 // COM1
#define MAX_DBG_CHUNKSIZE PAGESIZE

#define MAX_DBG_PACKETLEN (MAX_DBG_CHUNKSIZE + sizeof(union dbg_body))

extern char *trapnames[]; // Defined in trap.c

int debugging = 0;
int debugger_active = 0;
int debug_nointr = 0;

static char dbgdata[MAX_DBG_PACKETLEN];
struct dbg_evt_trap last_trap;
static char *krnlname = "krnl.dll";

static void init_debug_port() {
  // Turn off interrupts
  outp(DEBUGPORT + 1, 0);
  
  // Set 115200 baud, 8 bits, no parity, one stopbit
  outp(DEBUGPORT + 3, 0x80);
  outp(DEBUGPORT + 0, 0x01); // 0x0C = 9600, 0x01 = 115200
  outp(DEBUGPORT + 1, 0x00);
  outp(DEBUGPORT + 3, 0x03);
  outp(DEBUGPORT + 2, 0xC7);
  outp(DEBUGPORT + 4, 0x0B);
}

static void dbg_send(void *buffer, int count) {
  unsigned char *p = buffer;

  while (count-- > 0) {
    while ((inp(DEBUGPORT + 5) & 0x20) == 0) {
      if (!debug_nointr) check_dpc_queue();
    }
    outp(DEBUGPORT, *p++);
  }
}

static void dbg_recv(void *buffer, int count) {
  unsigned char *p = buffer;

  while (count-- > 0) {
    while ((inp(DEBUGPORT + 5) & 0x01) == 0) {
      if (!debug_nointr) check_dpc_queue();
    }
    *p++ = inp(DEBUGPORT) & 0xFF;
  }
}

static void dbg_send_rle(void *data, unsigned int len) {
  unsigned char *p = (unsigned char *) data;
  unsigned char *q = p;
  unsigned int left = len;

  while (left > 0) {
    if (p[0] == DBG_RLE_ESCAPE || (left > 3 && p[0] == p[1] && p[0] == p[2])) {
      unsigned char buf[3];

      if (p > q) {
        dbg_send(q, p - q);
        q = p;
      }

      while (left > 0 && q - p < 256 && q[0] == *p) {
        q++;
        left--;
      }

      buf[0] = DBG_RLE_ESCAPE;
      buf[1] = *p;
      buf[2] = (unsigned char) (q - p);
      dbg_send(buf, 3);
      p = q;
    } else {
      p++;
      left--;
    }
  }

  if (p > q) dbg_send(q, p - q);
}

static void dbg_recv_rle(void *data, unsigned int len) {
  unsigned char *p = (unsigned char *) data;
  unsigned int left = len;
  while (left > 0) {
    unsigned char ch;

    dbg_recv(&ch, 1);
    if (ch == DBG_RLE_ESCAPE) {
      unsigned char value;
      unsigned char count;
      int n;
  
      dbg_recv(&value, 1);
      dbg_recv(&count, 1);
      n = (count == 0) ? 256 : count;

      while (n > 0) {
        *p++ = value;
        left--;
        n--;
      }
    } else {
      *p++ = ch;
      left--;
    }
  }
}

static void dbg_send_packet(int cmd, unsigned char id, void *data, unsigned int len) {
  unsigned int n;
  struct dbg_hdr hdr;
  unsigned char checksum;
  unsigned char *p;

  hdr.signature = DBG_SIGNATURE;
  hdr.cmd = (unsigned char) cmd;
  hdr.len = len;
  hdr.id = id;
  hdr.checksum = 0;

  checksum = 0;
  p = (unsigned char *) &hdr;
  for (n = 0; n < sizeof(struct dbg_hdr); n++) checksum += *p++;
  p = (unsigned char *) data;
  for (n = 0; n < len; n++) checksum += *p++;
  hdr.checksum = -checksum;

  dbg_send(&hdr, sizeof(struct dbg_hdr));
  dbg_send_rle(data, len);
}

static void dbg_send_error(unsigned char errcode, unsigned char id) {
  dbg_send_packet(errcode, id, NULL, 0);
}

static int dbg_recv_packet(struct dbg_hdr *hdr, void *data) {
  unsigned int n;
  unsigned char checksum;
  unsigned char *p;

  while (1) {
    dbg_recv(&hdr->signature, 1);
    if (hdr->signature == DBG_SIGNATURE) break;
  }

  dbg_recv(&hdr->cmd, 1);
  dbg_recv(&hdr->id, 1);
  dbg_recv(&hdr->checksum, 1);
  dbg_recv((unsigned char *) &hdr->len, 4);
  if (hdr->len > MAX_DBG_PACKETLEN) return -EBUF;
  dbg_recv_rle(data, hdr->len);

  checksum = 0;
  p = (unsigned char *) hdr;
  for (n = 0; n < sizeof(struct dbg_hdr); n++) checksum += *p++;
  p = (unsigned char *) data;
  for (n = 0; n < hdr->len; n++) checksum += *p++;
  if (checksum != 0) return -EIO;

  return hdr->len;
}

static void dbg_connect(struct dbg_hdr *hdr, union dbg_body *body) {
  struct thread *t = self();

  if (body->conn.version != DRPC_VERSION) {
    dbg_send_error(DBGERR_VERSION, hdr->id);
  } else {
    body->conn.version = DRPC_VERSION;
    body->conn.trap = last_trap;
    body->conn.mod.hmod = (hmodule_t) OSBASE;
    body->conn.mod.name = &krnlname;
    body->conn.thr.tid = t->id;
    body->conn.thr.tib = t->tib;
    body->conn.thr.tcb = t;
    body->conn.thr.startaddr = t->entrypoint;
    body->conn.cpu = cpu;

    dbg_send_packet(hdr->cmd + DBGCMD_REPLY, hdr->id, body, sizeof(struct dbg_connect));
  }
}

static void dbg_read_memory(struct dbg_hdr *hdr, union dbg_body *body) {
  if (!mem_access(body->mem.addr, body->mem.size, PT_PRESENT)) {
    dbg_send_error(DBGERR_INVALIDADDR, hdr->id);
  } else {
    dbg_send_packet(hdr->cmd | DBGCMD_REPLY, hdr->id, body->mem.addr, body->mem.size);
  }
}

static void dbg_write_memory(struct dbg_hdr *hdr, union dbg_body *body) {
  if (!mem_access(body->mem.addr, body->mem.size, PT_PRESENT | PT_WRITABLE)) {
    dbg_send_error(DBGERR_INVALIDADDR, hdr->id);
  } else {
    memcpy(body->mem.addr, body->mem.data, body->mem.size);
    dbg_send_packet(hdr->cmd | DBGCMD_REPLY, hdr->id, NULL, 0);
  }
}

static void dbg_suspend_thread(struct dbg_hdr *hdr, union dbg_body *body) {
  int n;
  tid_t tid;
  struct thread *t;

  for (n = 0; n < body->thr.count; n++) {
    tid = body->thr.threadids[n];
    t = get_thread(tid);
    if (t == NULL) {
      body->thr.threadids[n] = -ENOENT;
    } else if (t != self()) {
      body->thr.threadids[n] = suspend_thread(t);
    }
  }

  dbg_send_packet(hdr->cmd | DBGCMD_REPLY, hdr->id, body, hdr->len);
}

static void dbg_resume_thread(struct dbg_hdr *hdr, union dbg_body *body) {
  int n;
  tid_t tid;
  struct thread *t;

  for (n = 0; n < body->thr.count; n++) {
    tid = body->thr.threadids[n];
    t = get_thread(tid);
    if (t == NULL) {
      body->thr.threadids[n] = -ENOENT;
    } else if (t != self()) {
      body->thr.threadids[n] = resume_thread(t);
    }
  }

  dbg_send_packet(hdr->cmd | DBGCMD_REPLY, hdr->id, body, hdr->len);
}

static void dbg_get_thread_context(struct dbg_hdr *hdr, union dbg_body *body) {
  struct thread *t;

  t = get_thread(body->ctx.tid);
  if (!t) {
    dbg_send_error(DBGERR_INVALIDTHREAD, hdr->id);
    return;
  }

  if (t->ctxt) {
    memcpy(&body->ctx.ctxt, t->ctxt, sizeof(struct context));
  
    // DS and ES are 16 bit register, clear upper bits
    body->ctx.ctxt.ds &= 0xFFFF;
    body->ctx.ctxt.es &= 0xFFFF;

    // Kernel mode contexts does not have ss:esp in context, fixup
    if (KERNELSPACE(body->ctx.ctxt.eip)) {
      body->ctx.ctxt.ess = SEL_KDATA | mach.kring;
      body->ctx.ctxt.esp = (unsigned long) t->ctxt + sizeof(struct context) - 8;
    }
  } else {
    // Build kernel mini context
    struct tcb *tcb= (struct tcb *) t;
    struct kernel_context *kctxt = (struct kernel_context *) tcb->esp;

    memset(&body->ctx.ctxt, 0, sizeof(struct context));
    body->ctx.ctxt.esi = kctxt->esi;
    body->ctx.ctxt.edi = kctxt->edi;
    body->ctx.ctxt.ebx = kctxt->ebx;
    body->ctx.ctxt.ebp = kctxt->ebp;
    body->ctx.ctxt.eip = kctxt->eip;
    body->ctx.ctxt.esp = (unsigned long) kctxt->stack;

    body->ctx.ctxt.ds = SEL_KDATA | mach.kring;
    body->ctx.ctxt.es = SEL_KDATA | mach.kring;
    body->ctx.ctxt.ess = SEL_KDATA | mach.kring;
    body->ctx.ctxt.ecs = SEL_KTEXT | mach.kring;
  }

  dbg_send_packet(hdr->cmd | DBGCMD_REPLY, hdr->id, body, sizeof(struct dbg_context));
}

static void dbg_set_thread_context(struct dbg_hdr *hdr, union dbg_body *body) {
  struct thread *t;

  t = get_thread(body->ctx.tid);
  if (!t) {
    dbg_send_error(DBGERR_INVALIDTHREAD, hdr->id);
    return;
  }

  if (!t->ctxt) {
    dbg_send_error(DBGERR_NOCONTEXT, hdr->id);
    return;
  }

  // Kernel mode contexts does not have ss:esp in context
  if (KERNELSPACE(body->ctx.ctxt.eip)) {
    memcpy(t->ctxt, &body->ctx.ctxt, sizeof(struct context) - 8);
  } else {
    memcpy(t->ctxt, &body->ctx.ctxt, sizeof(struct context));
  }

  dbg_send_packet(hdr->cmd | DBGCMD_REPLY, hdr->id, NULL, 0);
}

static void dbg_get_selector(struct dbg_hdr *hdr, union dbg_body *body) {
  int gdtidx = body->sel.sel >> 3;

  if (gdtidx < 0 || gdtidx >= MAXGDT) {
    dbg_send_error(DBGERR_INVALIDSEL, hdr->id);
    return;
  }

  memcpy(&body->sel.seg, &syspage->gdt[gdtidx], sizeof(struct segment));
  dbg_send_packet(hdr->cmd | DBGCMD_REPLY, hdr->id, body, sizeof(struct dbg_selector));
}

static void dbg_get_modules(struct dbg_hdr *hdr, union dbg_body *body) {
  struct peb *peb = (struct peb *) PEB_ADDRESS;
  struct module *mod;
  int n = 0;

  mod = kmods.modules;
  if (kmods.modules) {
    while (1) {
      body->modl.mods[n].hmod = mod->hmod;
      body->modl.mods[n].name = &mod->name;
      n++;

      mod = mod->next;
      if (mod == kmods.modules) break;
    }
  }

  if (page_mapped(peb) && peb->usermods) {
    mod = peb->usermods->modules;
    while (1) {
      body->modl.mods[n].hmod = mod->hmod;
      body->modl.mods[n].name = &mod->name;
      n++;

      mod = mod->next;
      if (mod == peb->usermods->modules) break;
    }
  }

  if (n == 0)  {
    body->modl.mods[n].hmod = (hmodule_t) OSBASE;
    body->modl.mods[n].name = &krnlname;
    n++;
  }

  body->modl.count = n;
  dbg_send_packet(hdr->cmd | DBGCMD_REPLY, hdr->id, body, sizeof(struct dbg_modulelist) + n * sizeof(struct dbg_moduleinfo));
}

static void dbg_get_threads(struct dbg_hdr *hdr, union dbg_body *body) {
  int n = 0;
  struct thread *t = threadlist;

  while (1) {
    body->thl.threads[n].tid = t->id;
    body->thl.threads[n].tib = t->tib;
    body->thl.threads[n].startaddr = t->entrypoint;
    body->thl.threads[n].tcb = t;
    n++;

    t = t->next;
    if (t == threadlist) break;
  }
  body->thl.count = n;
  dbg_send_packet(hdr->cmd | DBGCMD_REPLY, hdr->id, body, sizeof(struct dbg_threadlist) + n * sizeof(struct dbg_threadinfo));
}

static void dbg_main() {
  struct dbg_hdr hdr;
  union dbg_body *body = (union dbg_body *) dbgdata;

  int rc;

  if (!debugging) {
    init_debug_port();
    kprintf("dbg: waiting for remote debugger...\n");
    debugging = 1;
  }

  debugger_active = 1;

  while (1) {
    rc = dbg_recv_packet(&hdr, dbgdata);
    if (rc < 0) {
      kprintf("dbg: error %d receiving debugger command\n", rc);
      continue;
    }

    switch (hdr.cmd) {
      case DBGCMD_CONNECT:
        dbg_connect(&hdr, body);
        print_string("dbg: remote debugger connected\n");
        break;

      case DBGCMD_CONTINUE:
        dbg_send_packet(DBGCMD_CONTINUE | DBGCMD_REPLY, hdr.id, NULL, 0);
        debugger_active = 0;
        return;

      case DBGCMD_READ_MEMORY:
        dbg_read_memory(&hdr, body);
        break;

      case DBGCMD_WRITE_MEMORY:
        dbg_write_memory(&hdr, body);
        break;

      case DBGCMD_SUSPEND_THREAD:
        dbg_suspend_thread(&hdr, body);
        break;

      case DBGCMD_RESUME_THREAD:
        dbg_resume_thread(&hdr, body);
        break;

      case DBGCMD_GET_THREAD_CONTEXT:
        dbg_get_thread_context(&hdr, body);
        break;

      case DBGCMD_SET_THREAD_CONTEXT:
        dbg_set_thread_context(&hdr, body);
        break;

      case DBGCMD_GET_SELECTOR:
        dbg_get_selector(&hdr, body);
        break;

      case DBGCMD_GET_MODULES:
        dbg_get_modules(&hdr, body);
        break;

      case DBGCMD_GET_THREADS:
        dbg_get_threads(&hdr, body);
        break;

      default:
        kprintf("dbg: invalid command %d %d len=%d\n", hdr.id, hdr.cmd, hdr.len);
        dbg_send_error(DBGERR_INVALIDCMD, hdr.id);
    }
  }

  debugger_active = 0;
}

void dumpregs(struct context *ctxt) {
  kprintf("EAX  = %08X EBX  = %08X ECX  = %08X EDX  = %08X\n", ctxt->eax, ctxt->ebx, ctxt->ecx, ctxt->edx);
  kprintf("EDI  = %08X ESI  = %08X EBP  = %08X ESP  = %08X\n", ctxt->edi, ctxt->esi, ctxt->ebp, ctxt->esp);
  kprintf("CS   = %08X DS   = %08X ES   = %08X SS   = %08X\n", ctxt->ecs, ctxt->ds, ctxt->es, ctxt->ess);
  kprintf("EIP  = %08X EFLG = %08X TRAP = %08X ERR  = %08X\n", ctxt->eip, ctxt->eflags, ctxt->traptype, ctxt->errcode);
}

void dbg_enter(struct context *ctxt, void *addr) {
  if (!debugging) {
    kprintf("dbg: trap %d (%s) thread %d, addr %p\n", ctxt->traptype, trapnames[ctxt->traptype], self()->id, addr);
    dumpregs(ctxt);
  }

  last_trap.tid = self()->id;
  last_trap.traptype = ctxt->traptype;
  last_trap.errcode = ctxt->errcode;
  last_trap.eip = ctxt->eip;
  last_trap.addr = addr;

  if (!debug_nointr) sti();

  if (debugging) {
    if (debugger_active) {
      kprintf("dbg: trap %d thread %d, addr %p while debugger active, system halted.\n", ctxt->traptype, self()->id, addr);
      dumpregs(ctxt);
      cli();
      halt();
    }

    dbg_send_packet(DBGEVT_TRAP, 0, &last_trap, sizeof(struct dbg_evt_trap));
  }

  dbg_main();

  if (self()->suspend_count > 0) dispatch();
}

void dbg_notify_create_thread(struct thread *t, void *startaddr) {
  struct dbg_evt_create_thread create;

  if (debugging && !debugger_active) {
    create.tid = t->id;
    create.tib = t->tib;
    create.startaddr = startaddr;
    create.tcb = t;

    dbg_send_packet(DBGEVT_CREATE_THREAD, 0, &create, sizeof(struct dbg_evt_create_thread));
    dbg_main();
  }
}

void dbg_notify_exit_thread(struct thread *t) {
  struct dbg_evt_exit_thread exit;

  if (debugging && !debugger_active) {
    exit.tid = t->id;
    exit.exitcode = t->exitcode;

    dbg_send_packet(DBGEVT_EXIT_THREAD, 0, &exit, sizeof(struct dbg_evt_exit_thread));
    dbg_main();
  }
}

void dbg_notify_load_module(hmodule_t hmod, char **name) {
  struct dbg_evt_load_module load;

  if (debugging && !debugger_active) {
    load.hmod = hmod;
    load.name = name;

    dbg_send_packet(DBGEVT_LOAD_MODULE, 0, &load, sizeof(struct dbg_evt_load_module));
    dbg_main();
  }
}

void dbg_notify_unload_module(hmodule_t hmod) {
  struct dbg_evt_unload_module unload;

  if (debugging && !debugger_active) {
    unload.hmod = hmod;

    dbg_send_packet(DBGEVT_UNLOAD_MODULE, 0, &unload, sizeof(struct dbg_evt_unload_module));
    dbg_main();
  }
}

void dbg_output(char *msg) {
  struct dbg_evt_output output;

  if (debugging && !debugger_active) {
    output.msgptr = msg;
    output.msglen = strlen(msg) + 1;

    dbg_send_packet(DBGEVT_OUTPUT, 0, &output, sizeof(struct dbg_evt_output));
    dbg_main();
  }
}
