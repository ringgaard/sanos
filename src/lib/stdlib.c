//
// stdlib.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Standard library functions
//

#include <os.h>
#include <string.h>

#ifdef KERNEL
#include <os/krnl.h>
#endif

__declspec(naked) void _allmul()
{
  __asm
  {

    mov     eax,[esp+8]     ; HIWORD(A)
    mov     ecx,[esp+16]    ; HIWORD(B)
    or      ecx,eax         ; test for both hiwords zero.
    mov     ecx,[esp+12]    ; LOWORD(B)
    jnz     short hard      ; both are zero, just mult ALO and BLO

    mov     eax,[esp+4]     ; LOWORD(A)
    mul     ecx

    ret     16              ; callee restores the stack

  hard:
    push    ebx

    mul     ecx             ; eax has AHI, ecx has BLO, so AHI * BLO
    mov     ebx,eax         ; save result

    mov     eax,[esp+8]     ; LOWORD(A2)
    mul     dword ptr [esp+20] ; ALO * BHI
    add     ebx,eax         ; ebx = ((ALO * BHI) + (AHI * BLO))

    mov     eax,[esp+8]     ; ecx = BLO
    mul     ecx             ; so edx:eax = ALO*BLO
    add     edx,ebx         ; now edx has all the LO*HI stuff

    pop     ebx

    ret     16              ; callee restores the stack
  }
}

char *strdup(char *s)
{
  char *t;
  int len;

  if (!s) return NULL;
  len = strlen(s);
  t = (char *) malloc(len + 1);
  memcpy(t, s, len + 1);
  return t;
}

int parse_args(char *args, char **argv)
{
  char *p;
  int argc;
  char *start;
  char *end;
  char *buf;
  int delim;

  p = args;
  argc = 0;
  while (*p)
  {
    while (*p == ' ') p++;
    if (!*p) break;

    if (*p == '"' || *p == '\'')
    {
      delim = *p++;
      start = p;
      while (*p && *p != delim) p++;
      end = p;
      if (*p == delim) p++;
    }
    else
    {
      start = p;
      while (*p && *p != ' ') p++;
      end = p;
    }

    if (argv)
    {
      buf = (char *) malloc(end - start + 1);
      if (!buf) break;
      memcpy(buf, start, end - start);
      buf[end - start] = 0;
      argv[argc] = buf;
    }
    
    argc++;
  }

  return argc;
}

void free_args(int argc, char **argv)
{
  int i;

  for (i = 0; i < argc; i++) free(argv[i]);
  if (argv) free(argv);
}

#ifdef DEBUG

int abs(int number)
{
  return number >= 0 ? number : -number;
}

#endif