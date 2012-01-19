//
// chkstk.s
//
// TCC stack check
//

.text
        
.globl ___chkstk
.globl __chkstk
.globl _alloca
.globl __alloca

___chkstk:
__chkstk:
        xchg    (%esp), %ebp   /* store ebp, get ret.addr */
        push    %ebp           /* push ret.addr */
        lea     4(%esp), %ebp  /* setup frame ptr */
        push    %ecx           /* save ecx */
        mov     %ebp, %ecx
L1:
        sub     $4096,%ecx
        test    %eax,(%ecx)
        sub     $4096,%eax
        cmp     $4096,%eax
        jge     L1

        sub     %eax,%ecx
        mov     %esp,%eax
        test    %eax,(%ecx)
        mov     %ecx,%esp

        mov     (%eax),%ecx     /* restore ecx */
        mov     4(%eax),%eax
        push    %eax
        ret

__alloca:
_alloca:
        popl    %edx            /* pop return addr */
        popl    %eax            /* pop amount to allocate */
        movl    %esp,%ecx
        addl    $3,%eax         /* round up to next word */
        andl    $0xfffffffc,%eax
        subl    %eax,%esp
        movl    %esp,%eax       /* base of newly allocated space */
        pushl   8(%ecx)         /* copy possible saved registers */
        pushl   4(%ecx)
        pushl   0(%ecx)
        pushl   %eax            /* dummy to pop at callsite */
        jmp     *%edx           /* "return" */
