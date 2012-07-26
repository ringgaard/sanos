//
// except.c
//
// MSVCRT exception handling
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
// Copyright (C) 2000 Jon Griffiths
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

#include "msvcrt.h"

__declspec(dllimport) VOID __stdcall RtlUnwind(PEXCEPTION_FRAME endframe, LPVOID eip, PEXCEPTION_RECORD rec, DWORD retval);

typedef struct _SCOPETABLE {
  int previousTryLevel;
  int (*lpfnFilter)(PEXCEPTION_POINTERS);
  int (*lpfnHandler)(void);
} SCOPETABLE, *PSCOPETABLE;

typedef struct _MSVCRT_EXCEPTION_FRAME {
  EXCEPTION_FRAME *prev;
  void (*handler)(PEXCEPTION_RECORD, PEXCEPTION_FRAME, PCONTEXT, PEXCEPTION_RECORD);
  PSCOPETABLE scopetable;
  int trylevel;
  int _ebp;
  PEXCEPTION_POINTERS xpointers;
} MSVCRT_EXCEPTION_FRAME;

#define TRYLEVEL_END (-1) // End of trylevel list

static __inline EXCEPTION_FRAME *push_frame(EXCEPTION_FRAME *frame) {
  struct tib *tib = gettib();
  frame->prev = (EXCEPTION_FRAME *) tib->except;
  tib->except = (struct xcptrec *) frame;
  return frame->prev;
}

static __inline EXCEPTION_FRAME *pop_frame(EXCEPTION_FRAME *frame) {
  struct tib *tib = gettib();
  tib->except = (struct xcptrec *) frame->prev;
  return frame->prev;
}

#pragma warning(disable: 4731) // C4731: frame pointer register 'ebp' modified by inline assembly code

static void call_finally_block(void *code_block, void *base_ptr) {
 __asm {
   mov eax, [code_block]
   mov ebp, [base_ptr]
   call eax
 }
}

static DWORD call_filter(void *func, void *arg, void *base_ptr) {
  DWORD rc;

  __asm {
    push ebp
    push [arg]
    mov eax, [func]
    mov ebp, [base_ptr]
    call eax
    pop ebp
    pop ebp
    mov [rc], eax
  }

  return rc;
}

static EXCEPTION_DISPOSITION msvcrt_nested_handler(EXCEPTION_RECORD *rec, EXCEPTION_FRAME *frame, CONTEXT *ctxt, EXCEPTION_FRAME **dispatcher) {
  if (rec->ExceptionFlags & (EH_UNWINDING | EH_EXIT_UNWIND)) return ExceptionContinueSearch;
  *dispatcher = frame;
  return ExceptionCollidedUnwind;
}

void _global_unwind2(PEXCEPTION_FRAME frame) {
  RtlUnwind(frame, 0, 0, 0);
}

void _local_unwind2(MSVCRT_EXCEPTION_FRAME* frame, int trylevel) {
  MSVCRT_EXCEPTION_FRAME *curframe = frame;
  EXCEPTION_FRAME reg;

  //syslog(LOG_DEBUG, "_local_unwind2(%p,%d,%d)",frame, frame->trylevel, trylevel);

  // Register a handler in case of a nested exception
  reg.handler = (PEXCEPTION_HANDLER) msvcrt_nested_handler;
  reg.prev = (PEXCEPTION_FRAME) gettib()->except;
  push_frame(&reg);

  while (frame->trylevel != TRYLEVEL_END && frame->trylevel != trylevel) {
    int curtrylevel = frame->scopetable[frame->trylevel].previousTryLevel;
    curframe = frame;
    curframe->trylevel = curtrylevel;
    if (!frame->scopetable[curtrylevel].lpfnFilter) {
      syslog(LOG_WARNING, "warning: __try block cleanup not implemented - expect crash!");
      // TODO: Remove current frame, set ebp, call frame->scopetable[curtrylevel].lpfnHandler()
    }
  }
  pop_frame(&reg);
}

int _except_handler3(PEXCEPTION_RECORD rec, MSVCRT_EXCEPTION_FRAME* frame, PCONTEXT context, void *dispatcher) {
  long retval;
  int trylevel;
  EXCEPTION_POINTERS exceptPtrs;
  PSCOPETABLE pScopeTable;

  //syslog(LOG_DEBUG, "msvcrt: exception %lx flags=%lx at %p handler=%p %p %p semi-stub",
  //       rec->ExceptionCode, rec->ExceptionFlags, rec->ExceptionAddress,
  //       frame->handler, context, dispatcher);

  __asm cld;

  if (rec->ExceptionFlags & (EH_UNWINDING | EH_EXIT_UNWIND)) {
    // Unwinding the current frame
     _local_unwind2(frame, TRYLEVEL_END);
    return ExceptionContinueSearch;
  } else {
    // Hunting for handler
    exceptPtrs.ExceptionRecord = rec;
    exceptPtrs.ContextRecord = context;
    *((DWORD *) frame - 1) = (DWORD) &exceptPtrs;
    trylevel = frame->trylevel;
    pScopeTable = frame->scopetable;

    while (trylevel != TRYLEVEL_END) {
      if (pScopeTable[trylevel].lpfnFilter) {
        //syslog(LOG_DEBUG, "filter = %p", pScopeTable[trylevel].lpfnFilter);

        retval = call_filter(pScopeTable[trylevel].lpfnFilter, &exceptPtrs, &frame->_ebp);

        //syslog(LOG_DEBUG, "filter returned %s", retval == EXCEPTION_CONTINUE_EXECUTION ?
        //      "CONTINUE_EXECUTION" : retval == EXCEPTION_EXECUTE_HANDLER ?
        //      "EXECUTE_HANDLER" : "CONTINUE_SEARCH");

        if (retval == EXCEPTION_CONTINUE_EXECUTION) return ExceptionContinueExecution;

        if (retval == EXCEPTION_EXECUTE_HANDLER) {
          // Unwind all higher frames, this one will handle the exception
          _global_unwind2((PEXCEPTION_FRAME) frame);
          _local_unwind2(frame, trylevel);

          // Set our trylevel to the enclosing block, and call the __finally code, which won't return
          frame->trylevel = pScopeTable->previousTryLevel;
          //syslog(LOG_DEBUG, "__finally block %p",pScopeTable[trylevel].lpfnHandler);
          call_finally_block(pScopeTable[trylevel].lpfnHandler, &frame->_ebp);
          panic("Returned from __finally block");
       }
      }
      trylevel = pScopeTable->previousTryLevel;
    }
  }

  return ExceptionContinueSearch;
}
