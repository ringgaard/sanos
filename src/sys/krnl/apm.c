//
// apm.c
//
// APM BIOS power management
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

//
// APM function codes
//

#define	APM_FUNC_INST_CHECK	0x5300
#define	APM_FUNC_REAL_CONN	0x5301
#define	APM_FUNC_16BIT_CONN	0x5302
#define	APM_FUNC_32BIT_CONN	0x5303
#define	APM_FUNC_DISCONN	0x5304
#define	APM_FUNC_IDLE		0x5305
#define	APM_FUNC_BUSY		0x5306
#define	APM_FUNC_SET_STATE	0x5307
#define	APM_FUNC_ENABLE_PM	0x5308
#define	APM_FUNC_RESTORE_BIOS	0x5309
#define	APM_FUNC_GET_STATUS	0x530A
#define	APM_FUNC_GET_EVENT	0x530B
#define	APM_FUNC_GET_STATE	0x530C
#define	APM_FUNC_ENABLE_DEV_PM	0x530D
#define	APM_FUNC_VERSION	0x530E
#define	APM_FUNC_ENGAGE_PM	0x530F
#define	APM_FUNC_GET_CAP	0x5310
#define	APM_FUNC_RESUME_TIMER	0x5311
#define	APM_FUNC_RESUME_ON_RING	0x5312
#define	APM_FUNC_TIMER		0x5313

static int apm_enabled = 0;
static struct fullptr apm_entrypoint;

static int apm_bios_call(unsigned long func, unsigned long ebx_in, unsigned long ecx_in,
	                 unsigned long *eax_out, unsigned long *ebx_out, unsigned long *ecx_out, 
			 unsigned long *edx_out, unsigned long *esi_out)
{
  __asm
  {
    cli

    push ebp
    push edi
    push esi

    push ds
    push es
    push fs
    push gs
    pushf

    mov eax, [func]
    mov ebx, [ebx_in]
    mov ecx, [ecx_in]

    call fword ptr [apm_entrypoint]
    setc al

    popf
    pop gs
    pop fs
    pop es
    pop ds

    mov [eax_out], eax
    mov [ebx_out], ebx
    mov [ecx_out], ecx
    mov [edx_out], edx
    mov [esi_out], esi

    pop esi
    pop edi
    pop ebp
  }

  return *eax_out & 0xFF;
}

static int apm_bios_call_simple(unsigned long func, unsigned long ebx_in, unsigned long ecx_in, 
				unsigned long *eax_out)
{
  __asm
  {
    cli

    push edi
    push esi

    push ds
    push es
    push fs
    push gs
    pushf

    mov eax, [func]
    mov ebx, [ebx_in]
    mov ecx, [ecx_in]

    call fword ptr [apm_entrypoint]
    setc al

    popf
    pop gs
    pop fs
    pop es
    pop ds

    mov [eax_out], eax

    pop esi
    pop edi

    sti
  }

  return *eax_out & 0xFF;
}

static int apm_driver_version(unsigned short *ver)
{
  unsigned long eax;

  if (apm_bios_call_simple(APM_FUNC_VERSION, 0, *ver, &eax)) return (eax >> 8) & 0xFF;
  *ver = (unsigned short) (eax & 0xFFFF);
  return 0;
}

void init_apm()
{
  struct apmparams *apm = &syspage->bootparams.apm;
  //unsigned short ver;
  //int rc;

  // Skip if no APM BIOS detected in ldrinit
  if (apm->version == 0) return;

  //kprintf("apm: BIOS version %d.%d Flags 0x%02x\n", ((apm->version >> 8) & 0xff), (apm->version & 0xff), apm->flags);
  //kprintf("apm: cseg32 0x%04x %d bytes\n", apm->cseg32, apm->cseg32len);
  //kprintf("apm: cseg16 0x%04x %d bytes\n", apm->cseg16, apm->cseg16len);
  //kprintf("apm: dseg 0x%04x %d bytes\n", apm->dseg, apm->dseglen);
  //kprintf("apm: entry 0x%04x\n", apm->entry);

  // TODO: This code does not yet work. The memory for the selectors needs to
  // be mapped before the APM BIOS can be used

  // Setup APM selectors in GDT
  seginit(&syspage->gdt[GDT_APMCS], apm->cseg32, apm->cseg32len, D_CODE | D_DPL0 | D_READ | D_PRESENT, D_BIG | D_BIG_LIM);
  seginit(&syspage->gdt[GDT_APMCS16], apm->cseg16, apm->cseg16len, D_CODE | D_DPL0 | D_READ | D_PRESENT, D_BIG | D_BIG_LIM);
  seginit(&syspage->gdt[GDT_APMDS], apm->dseg, apm->dseglen, D_DATA | D_DPL0 | D_WRITE | D_PRESENT, D_BIG | D_BIG_LIM);
  seginit(&syspage->gdt[GDT_APM40], 0x40, 4095 - 0x40 * 16, D_DATA | D_DPL0 | D_WRITE | D_PRESENT, D_BIG | D_BIG_LIM);

  // Setup APM entry point
  apm_entrypoint.segment = SEL_APMCS;
  apm_entrypoint.offset = apm->entry;

  apm_enabled = 1;

  //ver = apm->version;
  //rc = apm_driver_version(&ver);
  //if (rc != 0) 
  //  kprintf("apm: error %d in apm_driver_version()\n", rc);
  //else
  //  kprintf("apm: apm_driver_version() reports %04x\n", ver);
}
