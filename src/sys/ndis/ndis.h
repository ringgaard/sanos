//
// ndis.h
//
// NDIS network driver interface
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

#ifndef NDIS_H
#define NDIS_H

typedef unsigned char boolean;
typedef void *ndis_handle_t;
typedef unsigned short wchar_t;
typedef int ndis_status;
typedef unsigned long ndis_oid_t;
typedef __int64 ndis_physical_address_t;

//
// NDIS buffer alias Memory Descriptor List (MDL)
//
// An MDL describes pages in a virtual buffer in terms of physical pages.  The
// pages associated with the buffer are described in an array that is allocated
// just after the MDL header structure itself.  
//

struct ndis_buffer
{
  struct ndis_buffer *next;
  short size;
  short mdl_flags;
  void *process;
  void *mapped_system_va;
  void *start_va;
  unsigned long byte_count;
  unsigned long byte_offset;
  unsigned long pages[0];
};

#define MDL_MAPPED_TO_SYSTEM_VA     0x0001
#define MDL_PAGES_LOCKED            0x0002
#define MDL_SOURCE_IS_NONPAGED_POOL 0x0004
#define MDL_ALLOCATED_FIXED_SIZE    0x0008
#define MDL_PARTIAL                 0x0010
#define MDL_PARTIAL_HAS_BEEN_MAPPED 0x0020
#define MDL_IO_PAGE_READ            0x0040
#define MDL_WRITE_OPERATION         0x0080
#define MDL_PARENT_MAPPED_SYSTEM_VA 0x0100
#define MDL_FREE_EXTRA_PTES         0x0200
#define MDL_IO_SPACE                0x0800
#define MDL_NETWORK_HEADER          0x1000
#define MDL_MAPPING_CAN_FAIL        0x2000
#define MDL_ALLOCATED_MUST_SUCCEED  0x4000

//
// NDIS packet definition
//

struct ndis_packet_pool
{
  int todo;
};

struct ndis_packet;

struct ndis_packet_private
{
  unsigned int physical_count;             // Number of physical pages in packet.
  unsigned int total_length;               // Total amount of data in the packet.
  struct ndis_buffer *head;                // First buffer in the chain
  struct ndis_buffer *tail;                // Last buffer in the chain

  // If head is NULL the chain is empty; tail doesn't have to be NULL also

  struct ndis_packet_pool *pool;           // So we know where to free it back to
  unsigned int count;
  unsigned long flags;
  boolean valid_counts;
  unsigned char ndis_packet_flags;         // See PACKET_xxx bits below
  unsigned short ndis_packet_oob_offset;
};

struct ndis_packet
{
  struct ndis_packet_private private;

  union
  {
    struct                  // For Connection-less miniports
    {
      unsigned char miniport_reserved[2 * sizeof(void *)];
      unsigned char wrapper_reserved[2 * sizeof(void *)];
    };

    struct
    {
      //
      // For de-serialized miniports. And by implication conn-oriented miniports.
      // This is for the send-path only. Packets indicated will use wrapper_reserved
      // instead of wrapper_reservedex
      //
      unsigned char miniport_reservedex[3 * sizeof(void *)];
      unsigned char wrapper_reservedex[sizeof(void *)];
    };

    struct
    {
      unsigned char mac_reserved[4 * sizeof(void *)];
    };
  };

  unsigned long *reserved[2];            // For compatibility with Win95
  unsigned char protocol_reserved[1];
};

//
// NDIS Timers
//

typedef void (*ndis_timer_func_t)(void *system_specific1, void *function_context, void *system_specific2, void *system_specific3);

struct ndis_miniport_timer
{
  int todo;
};

//
// NDIS media types
//

enum ndis_medium
{
    NDIS_MEDIUM_802_3,
    NDIS_MEDIUM_802_5,
    NDIS_MEDIUM_FDDI,
    NDIS_MEDIUM_WAN,
    NDIS_MEDIUM_LOCALTALK,
    NDIS_MEDIUM_DIX,              // Defined for convenience, not a real medium
    NDIS_MEDIUM_ARCNETRAW,
    NDIS_MEDIUM_ARCNET878_2,
    NDIS_MEDIUM_ATM,
    NDIS_MEDIUM_WIRELESSWAN,
    NDIS_MEDIUM_IRDA,
    NDIS_MEDIUM_BPC,
    NDIS_MEDIUM_COWAN,
    NDIS_MEDIUM_1394,
    NDIS_MEDIUM_MAX               // Not a real medium, defined as an upper-bound
};

//
// NDIS power profile
//

enum ndis_power_profile
{
  NDIS_POWER_PROFILE_BATTERY,
  NDIS_POWER_PROFILE_AC_ONLINE
};

enum ndis_device_pnp_event
{
    NDIS_DEVICE_PNP_EVENT_QUERY_REMOVED,
    NDIS_DEVICE_PNP_EVENT_REMOVED,
    NDIS_DEVICE_PNP_EVENT_SURPRISE_REMOVED,
    NDIS_DEVICE_PNP_EVENT_QUERY_STOPPED,
    NDIS_DEVICE_PNP_EVENT_STOPPED,
    NDIS_DEVICE_PNP_EVENT_POWER_PROFILE_CHANGED,
    NDIS_DEVICE_PNP_EVENT_MAXIMUM
};

//
// NDIS miniport characteristics
//

typedef boolean (*w_check_for_hang_handler)
(
  ndis_handle_t miniport_adapter_context
);

typedef void (*w_disable_interrupt_handler)
(
  ndis_handle_t miniport_adapter_context
);

typedef void (*w_enable_interrupt_handler)
(
  ndis_handle_t miniport_adapter_context
);

typedef void (*w_halt_handler)
(
  ndis_handle_t miniport_adapter_context
);

typedef void (*w_handle_interrupt_handler)
(
  ndis_handle_t miniport_adapter_context
);

typedef ndis_status (*w_initialize_handler)
(
  ndis_status *open_error_status,
  unsigned int *selected_medium_index,
  enum ndis_medium *medium_array,
  unsigned int medium_array_size,
  ndis_handle_t miniport_adapter_context,
  ndis_handle_t wrapper_configuration_context
);

typedef void (*w_isr_handler)
(
  boolean *interrupt_recognized, 
  boolean *queue_miniport_handle_interrupt, 
  ndis_handle_t miniport_adapter_context
);

typedef ndis_status (*w_query_information_handler)
(
  ndis_handle_t miniport_adapter_context,
  ndis_oid_t oid,
  void *information_buffer,
  unsigned long information_buffer_length,
  unsigned long *bytes_written,
  unsigned long *bytes_needed
);

typedef ndis_status (*w_reconfigure_handler)
(
  ndis_status *open_error_status,
  ndis_handle_t miniport_adapter_context,
  ndis_handle_t wrapper_configuration_context
);

typedef ndis_status (*w_reset_handler)
(
  boolean *addressing_reset,
  ndis_handle_t miniport_adapter_context
);

typedef ndis_status (*w_send_handler)
(
  ndis_handle_t miniport_adapter_context,
  struct ndis_packet *packet,
  unsigned int flags
);

typedef ndis_status (*w_set_information_handler)
(
  ndis_handle_t miniport_adapter_context,
  ndis_oid_t oid,
  void *information_buffer,
  unsigned long information_buffer_length,
  unsigned long *bytes_read,
  unsigned long *bytes_needed
);

typedef ndis_status (*w_transfer_data_handler)
(
  struct ndis_packet *packet,
  unsigned int *bytes_transferred,
  ndis_handle_t miniport_adapter_context,
  ndis_handle_t miniport_receive_context,
  unsigned int byte_offset,
  unsigned int bytes_to_transfer
);

typedef void (*w_return_packet_handler)
(
  ndis_handle_t miniport_adapter_context,
  struct ndis_packet *packet
);

typedef void (*w_send_packets_handler)
(
  ndis_handle_t miniport_adapter_context,
  struct ndis_packet **packet_array,
  unsigned int number_of_packets
);

typedef void (*w_allocate_complete_handler)
(
  ndis_handle_t miniport_adapter_context,
  void *virtual_address,
  ndis_physical_address_t  *physical_address,
  unsigned long length,
  void *context
);

typedef ndis_status (*w_co_create_vc_handler)
(
  ndis_handle_t miniport_adapter_context,
  ndis_handle_t ndis_vc_handle,
  ndis_handle_t *miniport_vc_context
);

typedef ndis_status (*w_co_delete_vc_handler)
(
  ndis_handle_t miniport_vc_context
);

typedef ndis_status (*w_co_activate_vc_handler)
(
  ndis_handle_t miniport_vc_context,
  void *call_parameters // CO_CALL_PARAMETERS
);

typedef ndis_status (*w_co_deactivate_vc_handler)
(
  ndis_handle_t miniport_vc_context
);

typedef void (*w_co_send_packets_handler)
(
  ndis_handle_t miniport_vc_context,
  struct ndis_packet **packet_array,
  unsigned int number_of_packets
);

typedef ndis_status (*w_co_request_handler)
(
  ndis_handle_t miniport_adapter_context,
  ndis_handle_t ndis_vc_handle,
  void *ndis_request // NDIS_REQUEST
);

typedef void (*w_cancel_send_packets_handler)
(
  ndis_handle_t miniport_adapter_context,
  void *cancel_id
);

typedef void (*w_pnp_event_notify_handler)
(
  ndis_handle_t miniport_adapter_context,
  enum ndis_device_pnp_event device_pnp_event,
  void *information_buffer,
  unsigned long information_buffer_length
);

typedef void (*w_miniport_shutdown_handler) 
(
  ndis_handle_t miniport_adapter_context
);

typedef struct ndis_miniport_characteristics
{
  unsigned char major_ndis_version;
  unsigned char minor_ndis_version;
  unsigned short filler;
  unsigned int reserved;

  w_check_for_hang_handler      check_for_hang_handler;
  w_disable_interrupt_handler   disable_interrupt_handler;
  w_enable_interrupt_handler    enable_interrupt_handler;
  w_halt_handler                halt_handler;
  w_handle_interrupt_handler    handle_interrupt_handler;
  w_initialize_handler          initialize_handler;
  w_isr_handler                 isr_handler;
  w_query_information_handler   query_information_handler;
  w_reconfigure_handler         reconfigure_handler;
  w_reset_handler               reset_handler;
  w_send_handler                send_handler;
  w_set_information_handler     set_information_handler;
  w_transfer_data_handler       transfer_data_handler;

  // Extensions for NDIS 4.0
  w_return_packet_handler       return_packet_handler;
  w_send_packets_handler        send_packets_handler;
  w_allocate_complete_handler   allocate_complete_handler;

  // Extensions for NDIS 5.0
  w_co_create_vc_handler        co_create_vc_handler;
  w_co_delete_vc_handler        co_delete_vc_handler;
  w_co_activate_vc_handler      co_activate_vc_handler;
  w_co_deactivate_vc_handler    co_deactivate_vc_handler;
  w_co_send_packets_handler     co_send_packets_handler;
  w_co_request_handler          co_request_handler;

  // Extensions for NDIS 5.1
  w_cancel_send_packets_handler cancel_send_packets_handler;
  w_pnp_event_notify_handler    pnp_event_notify_handler;
  w_miniport_shutdown_handler   adapter_shutdown_handler;
  
  void *reserved1;
  void *reserved2;
  void *reserved3;
  void *reserved4;
};

#endif
