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

#define APM_FUNC_INST_CHECK     0x5300
#define APM_FUNC_REAL_CONN      0x5301
#define APM_FUNC_16BIT_CONN     0x5302
#define APM_FUNC_32BIT_CONN     0x5303
#define APM_FUNC_DISCONN        0x5304
#define APM_FUNC_IDLE           0x5305
#define APM_FUNC_BUSY           0x5306
#define APM_FUNC_SET_STATE      0x5307
#define APM_FUNC_ENABLE_PM      0x5308
#define APM_FUNC_RESTORE_BIOS   0x5309
#define APM_FUNC_GET_STATUS     0x530A
#define APM_FUNC_GET_EVENT      0x530B
#define APM_FUNC_GET_STATE      0x530C
#define APM_FUNC_ENABLE_DEV_PM  0x530D
#define APM_FUNC_VERSION        0x530E
#define APM_FUNC_ENGAGE_PM      0x530F
#define APM_FUNC_GET_CAP        0x5310
#define APM_FUNC_RESUME_TIMER   0x5311
#define APM_FUNC_RESUME_ON_RING 0x5312
#define APM_FUNC_TIMER          0x5313

//
// Power states
//

#define APM_STATE_READY         0x0000
#define APM_STATE_STANDBY       0x0001
#define APM_STATE_SUSPEND       0x0002
#define APM_STATE_OFF           0x0003
#define APM_STATE_BUSY          0x0004
#define APM_STATE_REJECT        0x0005
#define APM_STATE_OEM_SYS       0x0020
#define APM_STATE_OEM_DEV       0x0040

#define APM_STATE_DISABLE       0x0000
#define APM_STATE_ENABLE        0x0001

#define APM_STATE_DISENGAGE     0x0000
#define APM_STATE_ENGAGE        0x0001


//
// APM Device IDs
//

#define APM_DEVICE_BIOS         0x0000
#define APM_DEVICE_ALL          0x0001
#define APM_DEVICE_DISPLAY      0x0100
#define APM_DEVICE_STORAGE      0x0200
#define APM_DEVICE_PARALLEL     0x0300
#define APM_DEVICE_SERIAL       0x0400
#define APM_DEVICE_NETWORK      0x0500
#define APM_DEVICE_PCMCIA       0x0600
#define APM_DEVICE_BATTERY      0x8000
#define APM_DEVICE_OEM          0xE000
#define APM_DEVICE_OLD_ALL      0xFFFF
#define APM_DEVICE_CLASS        0x00FF
#define APM_DEVICE_MASK         0xFF00

//
// APM BIOS Installation Check Flags
//

#define APM_16_BIT_SUPPORT      0x0001
#define APM_32_BIT_SUPPORT      0x0002
#define APM_IDLE_SLOWS_CLOCK    0x0004
#define APM_BIOS_DISABLED       0x0008
#define APM_BIOS_DISENGAGED     0x0010

int apm_enabled = 0;
static unsigned long apm_conn_ver = 0;
static struct fullptr apm_entrypoint;

#pragma warning(disable: 4731) // C4731: frame pointer register 'ebp' modified by inline assembly code

static int apm_bios_call(unsigned long func, unsigned long ebx_in, unsigned long ecx_in,
                         unsigned long *eax_out, unsigned long *ebx_out, unsigned long *ecx_out, 
                         unsigned long *edx_out, unsigned long *esi_out) {
  __asm {
    push edi
    push esi

    push ebx
    push ecx

    push ebp
    push ds
    push es
    push fs
    push gs
    pushf

    mov eax, [func]
    mov ebx, [ebx_in]
    mov ecx, [ecx_in]
    
    CLI
    call fword ptr [apm_entrypoint]
    setc al
    STI

    popf
    pop gs
    pop fs
    pop es
    pop ds
    pop ebp

    mov edi, [eax_out]
    mov [edi], eax

    mov edi, [ebx_out]
    mov [edi], ebx
    
    mov edi, [ecx_out]
    mov [edi], ecx

    mov edi, [edx_out]
    mov [edi], edx

    mov edi, [esi_out]
    mov [edi], esi

    pop ecx
    pop ebx

    pop esi
    pop edi
  }

  return *eax_out & 0xFF;
}

static int apm_bios_call_simple(unsigned long func, unsigned long ebx_in, unsigned long ecx_in, 
                                unsigned long *eax_out) {
  __asm {
    push edi
    push esi

    push ebp
    push ds
    push es
    push fs
    push gs
    pushf

    mov eax, [func]
    mov ebx, [ebx_in]
    mov ecx, [ecx_in]
    xor edx, edx

    CLI
    call fword ptr [apm_entrypoint]
    setc al
    STI

    popf
    pop gs
    pop fs
    pop es
    pop ds
    pop ebp

    mov edi, [eax_out]
    mov [edi], eax

    pop esi
    pop edi
  }

  return *eax_out & 0xFF;
}

int apm_driver_version(unsigned long *ver) {
  unsigned long eax;
  int rc;

  rc = apm_bios_call_simple(APM_FUNC_VERSION, 0, *ver, &eax);
  if (rc != 0) return (eax >> 8) & 0xFF;
  *ver = (eax & 0xFFFF);
  return 0;
}

int apm_set_power_state(unsigned long device, unsigned long state) {
  unsigned long eax;
  int rc;

  rc = apm_bios_call_simple(APM_FUNC_SET_STATE, device, state, &eax);
  if (rc != 0) return (eax >> 8) & 0xFF;
  return 0;
}

int apm_set_system_power_state(unsigned long state) {
  return apm_set_power_state(APM_DEVICE_ALL, state);
}

int apm_get_power_status(unsigned long *status, unsigned long *battery, unsigned long *life) {
  unsigned long eax;
  unsigned long ebx;
  unsigned long ecx;
  unsigned long edx;
  unsigned long dummy;
  int rc;

  rc = apm_bios_call(APM_FUNC_GET_STATUS, APM_DEVICE_ALL, 0, &eax, &ebx, &ecx, &edx, &dummy);
  if (rc != 0) return (eax >> 8) & 0xFF;
  *status = ebx;
  *battery = ecx;
  *life = edx;
  
  return 0;
}

int apm_enable_power_management(unsigned long enable) {
  unsigned long eax;
  int rc;
  
  rc = apm_bios_call_simple(APM_FUNC_ENABLE_PM, apm_conn_ver > 0x0100 ? APM_DEVICE_ALL : APM_DEVICE_OLD_ALL, enable, &eax);
  if (rc != 0) return (eax >> 8) & 0xFF;
  return 0;
}

int apm_engage_power_management(unsigned long device, unsigned long enable) {
  unsigned long eax;
  int rc;

  rc = apm_bios_call_simple(APM_FUNC_ENGAGE_PM, device, enable, &eax);
  if (rc != 0) return (eax >> 8) & 0xFF;
  return 0;
}

void apm_power_off() {
  int rc;

  if (apm_enabled)  {
    rc = apm_set_system_power_state(APM_STATE_OFF);
    if (rc != 0) {
      // If shutdown fails, try to engage power management and try again.
      apm_engage_power_management(APM_DEVICE_ALL, 1);
      rc = apm_set_system_power_state(APM_STATE_OFF);
    }

    if (rc != 0) {
      kprintf(KERN_ERR "apm: error %d in apm_set_system_power_state()\n", rc);
    }
  }
}

int apm_proc(struct proc_file *pf, void *arg) {
  int rc;
  unsigned long status;
  unsigned long battery;
  unsigned long life;
  char *power_stat;
  char *bat_stat;

  rc = apm_get_power_status(&status, &battery, &life);
  if (rc != 0) {
    pprintf(pf, "power status not available");
  } else {
    switch ((status >> 8) & 0xff)  {
      case 0: power_stat = "off line"; break;
      case 1: power_stat = "on line"; break;
      case 2: power_stat = "on backup power"; break;
      default: power_stat = "unknown";
    }

    switch (status & 0xff) {
      case 0: bat_stat = "high"; break;
      case 1: bat_stat = "low"; break;
      case 2: bat_stat = "critical"; break;
      case 3: bat_stat = "charging"; break;
      default: bat_stat = "unknown";
    }

    pprintf(pf, "AC %s, battery status %s, battery life ", power_stat, bat_stat);
    if ((battery & 0xff) == 0xff) {
      pprintf(pf, "unknown");
    } else {
      pprintf(pf, "%d%%", battery & 0xff);
    }

    if ((life & 0xffff) != 0xffff) pprintf(pf, " %d %s\n", life & 0x7fff, (life & 0x8000) ? "minutes" : "seconds");
    pprintf(pf, "\n");
  }

  return 0;
}

int __declspec(dllexport) apm(struct unit *unit, char *opts) {
  struct apmparams *apm = &syspage->bootparams.apm;
  int rc;
  unsigned long vaddr;
  unsigned long cseg16len;
  unsigned long cseg32len;
  unsigned long dseglen;

  int engage = get_num_option(opts, "engage", 2);
  int enable = get_num_option(opts, "enable", 2);

  // Skip if no APM BIOS detected in ldrinit
  if (apm->version == 0) return 0;

  cseg16len = apm->cseg16len;
  cseg32len = apm->cseg32len;
  dseglen = apm->dseglen;

  switch (apm->version) {
    case 0x0100:
      cseg16len = 0x10000;
      cseg32len = 0x10000;
      dseglen = 0x10000;
      break;

    case 0x0101:
      cseg16len = cseg32len;
      break;
    
    case 0x0102:
    default:
      if (cseg16len == 0) cseg16len = 0x10000;
      if (cseg32len == 0) cseg32len = 0x10000;
      if (dseglen == 0) dseglen = 0x10000;
  }

  //kprintf("apm: version 0x%x flags 0x%x entry %x\n", apm->version, apm->flags, apm->entry);
  //kprintf("apm: cseg16 %x (%d bytes) cseg32 %x (%d bytes) dseg %x (%d bytes)\n", apm->cseg16, cseg16len, apm->cseg32, cseg32len, apm->dseg, dseglen);

  // Setup APM selectors in GDT
  vaddr = (unsigned long) iomap(apm->cseg32 << 4, cseg32len);
  set_gdt_entry(GDT_APMCS,  vaddr, cseg32len, D_CODE | D_DPL0 | D_READ | D_PRESENT, D_BIG);

  vaddr = (unsigned long) iomap(apm->cseg16 << 4, cseg16len);
  set_gdt_entry(GDT_APMCS16, vaddr, cseg16len, D_CODE | D_DPL0 | D_READ | D_PRESENT, 0);

  vaddr = (unsigned long) iomap(apm->dseg << 4, dseglen);
  set_gdt_entry(GDT_APMDS, vaddr, dseglen, D_DATA | D_DPL0 | D_WRITE | D_PRESENT, D_BIG);
  
  set_gdt_entry(GDT_APM40, (unsigned long) iomap(0x400, 4096), 4096 - 0x40 * 16, D_DATA | D_DPL0 | D_WRITE | D_PRESENT, D_BIG);

  // Setup APM entry point
  apm_entrypoint.segment = SEL_APMCS;
  apm_entrypoint.offset = apm->entry;

  // Initialize APM
  apm_conn_ver = apm->version;
  if (apm_conn_ver > 0x0100) {
    if (apm_conn_ver > 0x0102) apm_conn_ver = 0x0102;
    rc = apm_driver_version(&apm_conn_ver);
    if (rc != 0) {
      //kprintf(KERN_DEBUG "apm: error %d in apm_driver_version()\n", rc);
      apm_conn_ver = 0x0100;
    }
  }

  //kprintf(KERN_DEBUG "apm: connection version %04x\n", apm_conn_ver);

  if (enable == 1 || enable == 2 && (apm->flags & APM_BIOS_DISABLED) != 0) {
    rc = apm_enable_power_management(1);
    if (rc != 0)  {
      kprintf(KERN_ERR "apm: error %d in apm_enable_power_management()\n", rc);
      return -EIO;
    }
  }

  if (engage == 1 || engage == 2 && (apm->flags & APM_BIOS_DISENGAGED) != 0) {
    rc = apm_engage_power_management(APM_DEVICE_ALL, 1);
    if (rc != 0) {
      kprintf(KERN_ERR "apm: error %d in apm_engage_power_management()\n", rc);
      return -EIO;
    }
  }

  apm_enabled = 1;
  register_proc_inode("apm", apm_proc, NULL);
  return 0;
}
