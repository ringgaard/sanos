#include "msvcrt.h"

__declspec(naked) void _ftol()
{
  __asm 
  {
    push        ebp
    mov         ebp,esp
    add         esp,0F4h
    wait
    fnstcw      word ptr [ebp-2]
    wait
    mov         ax,word ptr [ebp-2]
    or          ah,0Ch
    mov         word ptr [ebp-4],ax
    fldcw       word ptr [ebp-4]
    fistp       qword ptr [ebp-0Ch]
    fldcw       word ptr [ebp-2]
    mov         eax,dword ptr [ebp-0Ch]
    mov         edx,dword ptr [ebp-8]
    leave
    ret
  }
}

__declspec(naked) void _isnan()
{
  __asm 
  {
    mov         eax,dword ptr [esp+0Ah]
    mov         ecx,7FF8h
    and         eax,ecx
    cmp         ax,7FF0h
    jne         00000022
    test        dword ptr [esp+8],7FFFFh

    jne         _isnan1
    cmp         dword ptr [esp+4],0
    jne         00000027
    cmp         ax,cx
    jne         _isnan2
    _isnan1:
    push        1
    pop         eax
    ret
    _isnan2:
    xor         eax,eax
    ret
  }
}

__declspec(naked) double _copysign(double a, double b)
{
  __asm 
  {
    push        ebp
    mov         ebp,esp
    push        ecx
    push        ecx
    mov         eax,dword ptr [ebp+8]
    mov         dword ptr [ebp-8],eax
    mov         eax,dword ptr [ebp+14h]
    xor         eax,dword ptr [ebp+0Ch]
    and         eax,7FFFFFFFh
    xor         eax,dword ptr [ebp+14h]
    mov         dword ptr [ebp-4],eax
    fld         qword ptr [ebp-8]
    leave
    ret
  }
}

__declspec(naked) void _finite()
{
  __asm 
  {
    mov         eax,dword ptr [esp+0Ah]
    xor         ecx,ecx
    and         ax,7FF0h
    cmp         ax,7FF0h
    setne       cl
    mov         eax,ecx
    ret
  }
}

void _CIfmod()
{
  panic("_CIfmod not implemented");
}

unsigned int _control87(unsigned int new, unsigned int mask)
{
  syslog(LOG_DEBUG, "warning: _control87 not implemented\n");
  return 0;
}

