//
// setjmp.s
//
// TCC non-local goto
//

OFS_EBP=0
OFS_EBX=4
OFS_EDI=8
OFS_ESI=12
OFS_ESP=16
OFS_EIP=20

.text

.globl  _setjmp
.globl  setjmp
.globl  _longjmp
.globl  longjmp

_setjmp:
setjmp:
    movl  4(%esp),%edx         /* Get jmp_buf pointer */
    movl  (%esp),%eax          /* Save EIP */
    movl  %eax, OFS_EIP(%edx)
    movl  %ebp, OFS_EBP(%edx)  /* Save EBP, EBX, EDI, ESI, and ESP */
    movl  %ebx, OFS_EBX(%edx)
    movl  %edi, OFS_EDI(%edx)
    movl  %esi, OFS_ESI(%edx)
    movl  %esp, OFS_ESP(%edx)
    xorl  %eax, %eax           /* Return 0 */
    ret

_longjmp:
longjmp:
    movl  4(%esp), %edx          /* Get jmp_buf pointer */
    movl  8(%esp), %eax          /* Get return value (eax) */

    movl  OFS_ESP(%edx), %esp    /* Switch to new stack position */
    movl  OFS_EIP(%edx), %ebx    /* Get new EIP value and set as return address */
    movl  %ebx, (%esp)
    
    movl  OFS_EBP(%edx), %ebp    /* Restore EBP, EBX, EDI, and ESI */
    movl  OFS_EBX(%edx), %ebx
    movl  OFS_EDI(%edx), %edi
    movl  OFS_ESI(%edx), %esi

    ret
