//
// pnpbios.h
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

#ifndef PNPBIOS_H
#define PNPBIOS_H

#define PNP_SIGNATURE   (('$' << 0) + ('P' << 8) + ('n' << 16) + ('P' << 24))

//
//
// PnP BIOS access functions
//
//

#define PNP_GET_NUM_SYS_DEV_NODES                  0x00
#define PNP_GET_SYS_DEV_NODE                       0x01
#define PNP_SET_SYS_DEV_NODE                       0x02
#define PNP_GET_EVENT                              0x03
#define PNP_SEND_MESSAGE                           0x04
#define PNP_GET_DOCKING_STATION_INFORMATION        0x05
#define PNP_SET_STATIC_ALLOCED_RES_INFO            0x09
#define PNP_GET_STATIC_ALLOCED_RES_INFO            0x0A
#define PNP_GET_APM_ID_TABLE                       0x0B
#define PNP_GET_PNP_ISA_CONFIG_STRUC               0x40
#define PNP_GET_ESCD_INFO                          0x41
#define PNP_READ_ESCD                              0x42
#define PNP_WRITE_ESCD                             0x43

//
// Status codes
//

#define PNP_SUCCESS                                0x00
#define PNP_NOT_SET_STATICALLY                     0x7F
#define PNP_UNKNOWN_FUNCTION                       0x81
#define PNP_FUNCTION_NOT_SUPPORTED                 0x82
#define PNP_INVALID_HANDLE                         0x83
#define PNP_BAD_PARAMETER                          0x84
#define PNP_SET_FAILED                             0x85
#define PNP_EVENTS_NOT_PENDING                     0x86
#define PNP_SYSTEM_NOT_DOCKED                      0x87
#define PNP_NO_ISA_PNP_CARDS                       0x88
#define PNP_UNABLE_TO_DETERMINE_DOCK_CAPABILITIES  0x89
#define PNP_CONFIG_CHANGE_FAILED_NO_BATTERY        0x8A
#define PNP_CONFIG_CHANGE_FAILED_RESOURCE_CONFLICT 0x8B
#define PNP_BUFFER_TOO_SMALL                       0x8C
#define PNP_USE_ESCD_SUPPORT                       0x8D
#define PNP_MESSAGE_NOT_SUPPORTED                  0x8E
#define PNP_HARDWARE_ERROR                         0x8F

#pragma pack(push, 1)

struct pnp_bios_expansion_header {
  unsigned long signature;      // "$PnP"
  unsigned char version;        // PnP BIOS version number in BCD
  unsigned char length;         // Length in bytes, currently 21h
  unsigned short control;       // System capabilities
  unsigned char checksum;       // Checksum, all bytes must add up to 0

  unsigned long eventflag;      // Physical address of the event flag
  unsigned short rmoffset;      // Real mode entry point 
  unsigned short rmcseg;
  unsigned short pm16offset;    // 16 bit protected mode entry
  unsigned long pm16cseg;
  unsigned long deviceid;       // EISA encoded system ID or 0
  unsigned short rmdseg;        // Real mode data segment
  unsigned long pm16dseg;       // 16 bit pm data segment base
};

struct pnp_dev_node_info {
  unsigned short no_nodes;
  unsigned short max_node_size;
};

struct pnp_bios_node {
  unsigned short size;
  unsigned char handle;
  unsigned long eisa_id;
  unsigned char type_code[3];
  unsigned short flags;
  unsigned char data[0];
};

struct pnp_isa_config_struc {
  unsigned char revision;
  unsigned char no_csns;
  unsigned short isa_rd_data_port;
  unsigned short reserved;
};

struct escd_info_struc {
  unsigned short min_escd_write_size;
  unsigned short escd_size;
  unsigned long nv_storage_base;
};

#pragma pack(pop)

int enum_isapnp(struct bus *bus);

#endif
