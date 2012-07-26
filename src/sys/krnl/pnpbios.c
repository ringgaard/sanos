//
// pnpbios.c
//
// PnP BIOS
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

#pragma warning(disable: 4731)

struct pnp_bios_expansion_header pnpbios;
struct fullptr pnp_thunk_entrypoint;

struct {
  unsigned char type_code[3];
  char *name;
} pnp_typecodes[] = {
  {{0x01, 0x00, 0x00}, "SCSI controller"},
  {{0x01, 0x01, 0x00}, "IDE controller"},
  {{0x01, 0x02, 0x00}, "Floppy disk controller"},
  {{0x01, 0x03, 0x00}, "IPI controller"},
  {{0x01, 0x80, 0x00}, "Mass storage controller"},
  
  {{0x02, 0x00, 0x00}, "Ethernet controller"},
  {{0x02, 0x01, 0x00}, "Token Ring network controller"},
  {{0x02, 0x02, 0x00}, "FDDI network controller"},
  {{0x02, 0x80, 0x00}, "Network controller"},

  {{0x03, 0x00, 0x00}, "VGA controller"},
  {{0x03, 0x00, 0x01}, "VESA SVGA controller"},
  {{0x03, 0x01, 0x00}, "XGA controller"},
  {{0x03, 0x80, 0x00}, "Display controller"},
  
  {{0x04, 0x00, 0x00}, "Video controller"},
  {{0x04, 0x01, 0x00}, "Audio controller"},
  {{0x04, 0x80, 0x00}, "Multi-media controller"},
  
  {{0x05, 0x00, 0x00}, "RAM memory"},
  {{0x05, 0x01, 0x00}, "Flash memory"},
  {{0x05, 0x80, 0x00}, "Memory"},

  {{0x06, 0x00, 0x00}, "Host bridge"},
  {{0x06, 0x01, 0x00}, "ISA bridge"},
  {{0x06, 0x02, 0x00}, "EISA bridge"},
  {{0x06, 0x03, 0x00}, "MicroChannel bridge"},
  {{0x06, 0x04, 0x00}, "PCI bridge"},
  {{0x06, 0x05, 0x00}, "PCMCIA bridge"},
  {{0x06, 0x80, 0x00}, "Bridge"},
 
  {{0x07, 0x00, 0x00}, "RS-232 port"},
  {{0x07, 0x00, 0x01}, "RS-232 port (16450-compatible)"},
  {{0x07, 0x00, 0x02}, "RS-232 port (16550-compatible)"},
  {{0x07, 0x01, 0x00}, "Parallel port"},
  {{0x07, 0x01, 0x01}, "Bidirectional parallel port"},
  {{0x07, 0x01, 0x02}, "ECP parallel port"},
  {{0x07, 0x80, 0x00}, "Communication device"},
  
  {{0x08, 0x00, 0x00}, "8259 PIC"},
  {{0x08, 0x00, 0x01}, "ISA PIC"},
  {{0x08, 0x00, 0x02}, "EISA PIC"},
  {{0x08, 0x01, 0x00}, "DMA controller"},
  {{0x08, 0x01, 0x01}, "ISA DMA controller"},
  {{0x08, 0x01, 0x02}, "EISA DMA controller"},
  {{0x08, 0x02, 0x00}, "System timer"},
  {{0x08, 0x02, 0x01}, "ISA system timer"},
  {{0x08, 0x02, 0x02}, "EISA system timer"},
  {{0x08, 0x03, 0x00}, "Real time tlock"},
  {{0x08, 0x03, 0x01}, "ISA real time clock"},
  {{0x08, 0x80, 0x00}, "System peripheral"},
  {{0x08, 0x80, 0xFF}, "System board"},

  {{0x09, 0x00, 0x00}, "Keyboard controller"},
  {{0x09, 0x01, 0x00}, "Digitizer pen"},
  {{0x09, 0x02, 0x00}, "Mouse controller"},
  {{0x09, 0x80, 0x00}, "Input device"},

  {{0x0A, 0x00, 0x00}, "Docking station"},
  {{0x0A, 0x80, 0x00}, "Docking station"},
  
  {{0x0B, 0x00, 0x00}, "386-based processor"},
  {{0x0B, 0x01, 0x00}, "486-based processor"},
  {{0x0B, 0x02, 0x00}, "Pentium-based processor"},
  {{0x0B, 0x80, 0x00}, "Processsor"},

  {{0xFF, 0xFF, 0xFF}, NULL},
};

static unsigned char pnp_bios_thunk[] = {
  0x52,                // push edx
  0x51,                // push ecx
  0x53,                // push ebx
  0x50,                // push eax
  0x66, 0x9A, 0,0,0,0, // call word pnpseg:pnpofs
  0x83, 0xC4, 0x10,    // add  esp, 16
  0xCB                 // retf 0
};

static int pnp_bios_call(int func, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7) {
  int status;

  __asm {
    CLI
    push ebp
    push edi
    push esi
    push ds
    push es
    push fs
    push gs
    pushf

    mov eax, [arg1]
    shl eax, 16
    or eax, [func]

    mov ebx, [arg3]
    shl ebx, 16
    or ebx, [arg2]

    mov ecx, [arg5]
    shl ecx, 16
    or ecx, [arg4]

    mov edx, [arg7]
    shl edx, 16
    or edx, [arg6]

    call fword ptr [pnp_thunk_entrypoint]

    popf
    pop gs
    pop fs
    pop es
    pop ds
    pop esi
    pop edi
    pop ebp

    STI
    and eax, 0x0000FFFF
    mov [status], eax
  }

  return status;
}

//
// Call PnP BIOS with function 0x00, "Get number of system device nodes"
//

static int pnp_bios_dev_node_info(struct pnp_dev_node_info *data) {
  int status;

  set_gdt_entry(GDT_AUX1, (unsigned long) data, sizeof(struct pnp_dev_node_info), D_DATA | D_DPL0 | D_WRITE | D_PRESENT, 0);

  status = pnp_bios_call(PNP_GET_NUM_SYS_DEV_NODES, 0, SEL_AUX1, 2, SEL_AUX1, SEL_PNPDATA, 0, 0);
  data->no_nodes &= 0xFF;
  return status;
}

// 
// Call PnP BIOS with function 0x01, "Get system device node"
//
// Input:
//  *nodenum = desired node, 
//  boot = whether to get nonvolatile boot (!=0) or volatile current (0) config
//
// Output: 
//   *nodenum=next node or 0xff if no more nodes
//

static int pnp_bios_get_dev_node(unsigned char *nodenum, char boot, struct pnp_bios_node *data) {
  int status;

  set_gdt_entry(GDT_AUX1, (unsigned long) nodenum, sizeof(char), D_DATA | D_DPL0 | D_WRITE | D_PRESENT, 0);
  set_gdt_entry(GDT_AUX2, (unsigned long) data, 64 * 1024, D_DATA | D_DPL0 | D_WRITE | D_PRESENT, 0);

  status = pnp_bios_call(PNP_GET_SYS_DEV_NODE, 0, SEL_AUX1, 0, SEL_AUX2, boot ? 2 : 1, SEL_PNPDATA, 0);
  return status;
}

//
// Call PnP BIOS with function 0x40, "Get isa pnp configuration structure"
//

static int pnp_bios_isapnp_config(struct pnp_isa_config_struc *data) {
  int status;

  set_gdt_entry(GDT_AUX1, (unsigned long) data, sizeof(struct pnp_isa_config_struc), D_DATA | D_DPL0 | D_WRITE | D_PRESENT, 0);

  status = pnp_bios_call(PNP_GET_PNP_ISA_CONFIG_STRUC, 0, SEL_AUX1, SEL_PNPDATA, 0, 0, 0, 0);
  return status;
}

//
// Call PnP BIOS with function 0x41, "Get ESCD info"
//

static int pnp_bios_escd_info(struct escd_info_struc *data) {
  int status;

  set_gdt_entry(GDT_AUX1, (unsigned long) data, sizeof(struct escd_info_struc), D_DATA | D_DPL0 | D_WRITE | D_PRESENT, 0);

  status = pnp_bios_call(PNP_GET_ESCD_INFO, 0, SEL_AUX1, 2, SEL_AUX1, 4, SEL_AUX1, SEL_PNPDATA);
  return status;
}

//
// Call PnP BIOS function 0x42, "read ESCD"
// nvram_base is determined by calling escd_info
//

static int pnp_bios_read_escd(void *data, void *nvram_base) {
  int status;

  set_gdt_entry(GDT_AUX1, (unsigned long) data, 64 * 1024, D_DATA | D_DPL0 | D_WRITE | D_PRESENT, 0);
  set_gdt_entry(GDT_AUX2, (unsigned long) nvram_base, 64 * 1024, D_DATA | D_DPL0 | D_WRITE | D_PRESENT, 0);

  status = pnp_bios_call(PNP_READ_ESCD, 0, SEL_AUX1, SEL_AUX2, SEL_PNPDATA, 0, 0, 0);
  return status;
}

static void pnpid32_to_pnpid(unsigned long id, char *str) {
  const char *hex = "0123456789ABCDEF";

  id = ntohl(id);
  str[0] = '@' + (char) ((id >> 26) & 0x1F);
  str[1] = '@' + (char) ((id >> 21) & 0x1F);
  str[2] = '@' + (char) ((id >> 16) & 0x1F);
  str[3] = hex[(id >> 12) & 0x0F];
  str[4] = hex[(id >> 8) & 0x0F];
  str[5] = hex[(id >> 4) & 0x0F];
  str[6] = hex[(id >> 0) & 0x0F];
  str[7] = '\0';

  return;
}

static char *get_device_type_name(unsigned char type_code[3]) {
  int i = 0;

  while (pnp_typecodes[i].name != NULL) {
    if (pnp_typecodes[i].type_code[0] == type_code[0] &&
        pnp_typecodes[i].type_code[1] == type_code[1] &&
        pnp_typecodes[i].type_code[2] == type_code[2]) {
      return pnp_typecodes[i].name;
    }

    i++;
  }

  return "Unknown";
}

static void extract_node_resource_data(struct pnp_bios_node *node, struct unit *unit) {
  unsigned char *p = node->data;
  unsigned char *end = node->data + node->size;
  unsigned char *lastp = NULL;

  while (p < end)  {
    if(p == lastp) break;

    if(p[0] & 0x80)  {
      // Large item
      switch (p[0] & 0x7F) {
        case 0x01: { // Memory
          int addr = *(short *) &p[4];
          int len = *(short *) &p[10];
          add_resource(unit, RESOURCE_MEM, 0, addr, len);
          break;
        }
        
        case 0x02: { // Device name
          int len = *(short *) &p[1];
          unit->productname = (char *) kmalloc(len + 1);
          memcpy(unit->productname, p + 3, len);
          unit->productname[len] = 0;
          break;
        }

        case 0x05: { // 32-bit memory
          int addr = *(int *) &p[4];
          int len = *(int *) &p[16];
          add_resource(unit, RESOURCE_MEM, 0, addr, len);
          break;
        }

        case 0x06: { // Fixed location 32-bit memory
          int addr = *(int *) &p[4];
          int len = *(int *) &p[8];
          add_resource(unit, RESOURCE_MEM, 0, addr, len);
          break;
        }

        //default:
          //kprintf("tag:%x ", p[0]);
      }

      lastp = p + 3;
      p = p + p[1] + (p[2] << 8) + 3;
      continue;
    }

     // Test for end tag
    if ((p[0] >> 3) == 0x0F) break;
                  
    switch (p[0] >> 3) {
      case 0x04: { // IRQ
        int i, mask, irq = -1;
        mask = p[1] + (p[2] << 8);
        for (i = 0; i < 16; i++, mask = mask >> 1) if (mask & 0x01) irq = i;
        if (irq != -1) add_resource(unit, RESOURCE_IRQ, 0, irq, 1);
        break;
      }

      case 0x05: { // DMA
        int i, mask, dma = -1;
        mask = p[1];
        for (i = 0; i < 8;i++, mask = mask>>1) if (mask & 0x01) dma = i;
        if (dma != -1) add_resource(unit, RESOURCE_DMA, 0, dma, 1);
        break;
      }

      case 0x08: { // I/O
        int io = p[2] + (p[3] << 8);
        int len = p[7];
        if (len != 0) add_resource(unit, RESOURCE_IO, 0, io, len);
        break;
      }

      case 0x09: { // Fixed location io
        int io = p[1] + (p[2] << 8);
        int len = p[3];
        add_resource(unit, RESOURCE_IO, 0, io, len);
        break;
      }

      //default:
        //kprintf("tag:%x ", p[0]);
    }

    lastp = p + 1;
    p = p + (p[0] & 0x07) + 1;
  }
}

static void build_sys_devlist(struct bus *bus) {
  struct pnp_dev_node_info info;
  int status;
  int i;
  int nodenum = 0;
  int nodes_fetched = 0;
  struct pnp_bios_node *node;
  struct unit *unit;
  unsigned long classcode;
  unsigned long unitcode;

  status = pnp_bios_dev_node_info(&info);
  if (status != 0) return;

  node = kmalloc(info.max_node_size);
  if (!node) return;

  for (i = 0; i < 0xFF && nodenum != 0xFF; i++)  {
    int thisnodenum = nodenum;

    if (pnp_bios_get_dev_node((unsigned char *) &nodenum, (char) 0 /*1*/, node)) {
      kprintf(KERN_ERR  "pnpbios: PnP BIOS reported error on attempt to get dev node.\n");
      break;
    }

    // The BIOS returns with nodenum = the next node number
    if (nodenum < thisnodenum)  {
      kprintf("pnpbios: Node number is out of sequence.\n");
      break;
    }

    nodes_fetched++;

    classcode = (node->type_code[0] << 16) + (node->type_code[1] << 8) + node->type_code[0];
    unitcode = node->eisa_id;
    unit = add_unit(bus, classcode, unitcode, nodenum + 16);
    unit->classname = get_device_type_name(node->type_code);

    extract_node_resource_data(node, unit);
  }

  kfree(node);
}

int enum_pnp_mem(struct unit *unit, unsigned char *b) {       
  int i = 0;
  unsigned long int start, len, more = 1;

  //static char *datasizes[] = {"(byte)", "(word)", "(dword)", "(rsv)"};
  //static char *decodeszs[] = {"(20bit)", "(24bit)", "(32bit)", "(rsv)"};
  //static char *memtypes[] = {"(sys)", "(exp)", "(vir)", "(oth)"};

  while (more)  {
    unsigned int ram, cached, wt_wb_cache, shared, datasize, decodesz, memtyp;

    ram = b[i] & 0x01;
    cached = b[i] & 0x02;
    wt_wb_cache = b[i] & 0x04;
    memtyp = (b[i] >> 3) & 0x03;
    shared = b[i] & 0x10;
    // bit 6: reserved
    more = b[i] & 0x80;
    datasize = b[i + 1] & 0x03;
    decodesz =(b[i + 1] >> 2) & 0x03;
    // bit 4-7: reserved

    start = (b[i + 2] + b[i + 3] * 256 + b[i + 4] * 256 * 256) * 256;
    len = (b[i + 5] + b[i + 6] * 256) * 1024;
    if (len == 0) len = 64 * 1024 * 1024;

    add_resource(unit, RESOURCE_MEM, 0, start, len);
 
    i += 7;
  }

  return i;
}

int enum_pnp_irq(struct unit *unit, unsigned char *b) {
  int i = 0, more = 1;

  while (more) {
    int irq, shared, mode;

    more = b[i] & 0x80;
    irq = b[i] & 0x0f;
    shared = b[i] & 0x40;
    mode = b[i] & 0x20;
    // b[i] & 0x10 must be 0
    // b[i + 1] set to 0 (reserved)

    add_resource(unit, RESOURCE_IRQ, 0, irq, 1);

    i += 2;
  }

  return i;
}

int enum_pnp_dma(struct unit *unit, unsigned char *b) {
  int i = 0, more = 1;

  //static char *transfers[] = {"8bit","(16bit)","(32bit)","(16bit w/count)"};
  //static char *timings[] = {"ISA", "(Type A)", "(Type B)", "(Type C)"};

  while (more) {
    int dma, shared, timing, transfer;

    more = b[i] & 0x80;
    dma = b[i] & 0x07;
    shared = b[i] & 0x40;
    // b[i] & 0x38 must be 0
    transfer = (b[i + 1] >> 2) & 0x03;
    timing = (b[i + 1] >> 4) & 0x03;
    // b[i + 2] & 0xc3 set to 0 (reserved)
    
    add_resource(unit, RESOURCE_DMA, 0, dma, 1);

    i += 2;
  }

  return i;
}

int enum_pnp_io(struct unit *unit, unsigned char *b) {
  int i = 0,more = 1;

  while (more) {
    unsigned int io, len, shared;

    more = b[i] & 0x80;
    len = (b[i] & 0x1f) + 1;
    shared = b[i] & 0x40;
    // b[i] & 0x20 set 0 (reserved)
    io = b[i + 1] + b[i + 2] * 256;

    add_resource(unit, RESOURCE_IO, 0, io, len);

    i += 3;
  }

  return i;
}

static int enum_pnp_board(struct bus *bus, unsigned char *b) {
  int size, len, off, type, o, slot, unitcode;
  struct unit *unit;

  size = b[0] + 256 * b[1];
  if (size == 0) return 0;

  slot = b[2];
  // b[3] reserved
  unitcode = *(unsigned long *) &b[4];
  // b[8] and b[9] probably EISASLOTINFO: 0x60 0x40 or 0x40 0x40 mostly ?!
  // b[10] and b[11] are 0

  // We are only interested in PnP ISA devices (slot 1 to 15)
  if (slot < 1 || slot > 15) return size;

  // Allocate new unit
  unit = add_unit(bus, 0, unitcode, slot);
  unit->classname = "PNPISA";

  // Enumerate resouces
  off = 12;
  while (off + 4 < size) {
    len = b[off];
    if (len == 0) break;
    type = b[off + 4];

    //kprintf(" Function (len=%d,type=0x%02x)\n", len, type);
    //if (b[off + 1] != 0) kprintf("escd: b[0x%x]=0x%x, expected 0\n", off + 1, b[off + 1]);
    //if (b[off + 2] != 1) kprintf("escd: b[0x%x]=0x%x, expected 1\n", off + 2, b[off + 2]);
    //if (b[off + 3] != 0) kprintf("escd: b[0x%x]=0x%x, expected 0\n", off + 3, b[off + 3]);
    
    if (type == 0xc0) {
      //showfreeform(&b[off + 4]);
      break;
    } else {
      o = off + 5;
      //if (type & 0x80) kprintf(" Device/Function disabled.\n");
      if (type & 0x02) o += enum_pnp_mem(unit, &b[o]);
      if (type & 0x04) o += enum_pnp_irq(unit, &b[o]);
      if (type & 0x08) o += enum_pnp_dma(unit, &b[o]);
      if (type & 0x10) o += enum_pnp_io(unit, &b[o]);
    }

    off += len + 2;
  }

  return size;
}

static void parse_escd(struct bus *bus, unsigned char *b, int maxlen) {
  int off, len, size, major, minor, brdcnt, i;

  if (memcmp("ACFG", &b[2], 4) != 0)  {
    kprintf(KERN_WARNING "escd: signature does not match ACFG, is: %4s\n", &b[2]);
    return;
  }

  size = b[0] + b[1] * 256;
  minor = b[6];
  major = b[7];
  brdcnt = b[8];

  //kprintf("escd: size=%d, version=%d.%d, boardcount=%d\n", size, major, minor, brdcnt);

  off = 12;
  for (i = 0; i < brdcnt; i++) {
    len = enum_pnp_board(bus, &b[off]);
    off += len;
    if (off > size) {
      kprintf(KERN_WARNING "escd: offset exceeded size (%d)\n", off);
      break;
    }
  }
}

static int build_isa_devlist(struct bus *bus) {
  int rc;
  struct pnp_isa_config_struc isacfg;
  struct escd_info_struc escdinfo;
  unsigned char *escd;
  void *nvbase;

  rc = pnp_bios_isapnp_config(&isacfg);
  if (rc != 0) return rc;

  rc = pnp_bios_escd_info(&escdinfo);
  if (rc != 0) return rc;

  escd = kmalloc(escdinfo.escd_size);
  if (!escd) return -ENOMEM;

  nvbase = iomap(escdinfo.nv_storage_base, 64 * 1024);
  if (!nvbase) return -EINVAL;

  rc = pnp_bios_read_escd(escd, nvbase);
  if (rc != 0) return rc;

  parse_escd(bus, escd, escdinfo.escd_size);

  iounmap(nvbase, 64 * 1024);
  kfree(escd);

  return 0;
}

int enum_isapnp(struct bus *bus) {
  int ofs;
  struct pnp_bios_expansion_header *hdr;
  unsigned char checksum;
  int i, length;

  // Map first 1MB physical memory
  for (i = 0; i < 256; i++) map_page((void *) PTOB(i), i, PT_WRITABLE | PT_PRESENT);

  // Search the defined area (0xf0000-0xffff0) for a valid PnP BIOS
  // structure and, if one is found, sets up the selectors and
  // entry points
  for (ofs = 0xF0000; ofs < 0xFFFF0; ofs += 16) {
    hdr = (struct pnp_bios_expansion_header *) ofs;

    if (hdr->signature != PNP_SIGNATURE) continue;
    length = hdr->length;
    if (!length) continue;

    checksum = 0;
    for (i = 0; i < length; i++) checksum += ((unsigned char *) hdr)[i];
    if (checksum) continue;

    //kprintf("pnpbios: PnP BIOS version %d.%d\n", hdr->version >> 4, hdr->version & 0x0F);

    memcpy(&pnpbios, hdr, sizeof(struct pnp_bios_expansion_header));

    set_gdt_entry(GDT_PNPTEXT, pnpbios.pm16cseg, 64 * 1024, D_CODE | D_DPL0 | D_READ | D_PRESENT, 0);
    set_gdt_entry(GDT_PNPDATA, pnpbios.pm16dseg, 64 * 1024, D_DATA | D_DPL0 | D_WRITE | D_PRESENT, 0);
    set_gdt_entry(GDT_PNPTHUNK, (unsigned long) pnp_bios_thunk, 1, D_CODE | D_DPL0 | D_READ | D_PRESENT, D_BIG | D_BIG_LIM);

    *((unsigned short *)(pnp_bios_thunk + 6)) = pnpbios.pm16offset;
    *((unsigned short *)(pnp_bios_thunk + 8)) = SEL_PNPTEXT;

    pnp_thunk_entrypoint.segment = SEL_PNPTHUNK;
    pnp_thunk_entrypoint.offset = 0;

    build_sys_devlist(bus);
    build_isa_devlist(bus);

    set_gdt_entry(GDT_PNPTEXT, 0, 0, 0, 0);
    set_gdt_entry(GDT_PNPDATA, 0, 0, 0, 0);
    set_gdt_entry(GDT_PNPTHUNK, 0, 0, 0, 0);

    for (i = 0; i < 256; i++) unmap_page((void *) PTOB(i));
    return 1;
  }

  for (i = 0; i < 256; i++) unmap_page((void *) PTOB(i));
  return 0;
}

int __declspec(dllexport) isapnp(struct unit *unit) {
  struct bus *isabus;

  isabus = add_bus(unit, BUSTYPE_ISA, 0);
  return enum_isapnp(isabus);
}
