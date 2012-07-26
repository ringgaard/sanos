//
// ndis.h
//
// NDIS network miniport driver interface
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
// NDIS status codes
//

#define NDIS_STATUS_SUCCESS                0x00000000L
#define NDIS_STATUS_RESOURCES              0xC000009AL
#define NDIS_STATUS_FAILURE                0xC0000001L

//
// NDIS buffer alias Memory Descriptor List (MDL)
//
// An MDL describes pages in a virtual buffer in terms of physical pages.  The
// pages associated with the buffer are described in an array that is allocated
// just after the MDL header structure itself.  
//

struct ndis_buffer {
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

struct ndis_packet_pool {
  int todo;
};

struct ndis_packet;

struct ndis_packet_private {
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

struct ndis_packet {
  struct ndis_packet_private private;

  union {
    struct {                 // For Connection-less miniports
      unsigned char miniport_reserved[2 * sizeof(void *)];
      unsigned char wrapper_reserved[2 * sizeof(void *)];
    };

    struct {
      //
      // For de-serialized miniports. And by implication conn-oriented miniports.
      // This is for the send-path only. Packets indicated will use wrapper_reserved
      // instead of wrapper_reservedex
      //
      unsigned char miniport_reservedex[3 * sizeof(void *)];
      unsigned char wrapper_reservedex[sizeof(void *)];
    };

    struct {
      unsigned char mac_reserved[4 * sizeof(void *)];
    };
  };

  unsigned long *reserved[2];            // For compatibility with Win95
  unsigned char protocol_reserved[1];
};

//
// NDIS Configuration
//

enum ndis_parameter_type {
  NDIS_PARAMETER_INTEGER,
  NDIS_PARAMETER_HEXINTEGER,
  NDIS_PARAMETER_STRING,
  NDIS_PARAMETER_MULTISTRING,
  NDIS_PARAMETER_BINARY
};

struct ndis_string  {
  unsigned short length;
  unsigned short maxlength;
  wchar_t *buffer;
};

struct ndis_blob {
  unsigned short length;
  void *buffer;
};

struct ndis_configuration_parameter {
  enum ndis_parameter_type parameter_type;
  union {
    unsigned long integer_data;
    struct ndis_string string_data;
    struct ndis_blob binary_data;
  } parameter_data;
};

//
// NDIS Timers
//

typedef void (__stdcall *ndis_timer_func_t)(void *system_specific1, void *function_context, void *system_specific2, void *system_specific3);

struct ndis_miniport_timer {
  int todo;
};

//
// NDIS interface types
//

enum ndis_interface_type {
  NDIS_INTERFACE_INTERNAL,
  NDIS_INTERFACE_ISA,
  NDIS_INTERFACE_EISA,
  NDIS_INTERFACE_MCA,
  NDIS_INTERFACE_TURBOCHANNEL,
  NDIS_INTERFACE_PCI,
  NDIS_INTERFACE_VMEBUS,
  NDIS_INTERFACE_NUBUS,
  NDIS_INTERFACE_PCMCIA,
  NDIS_INTERFACE_CBUS,
  NDIS_INTERFACE_MPIBUS,
  NDIS_INTERFACE_MPSABUS,
  NDIS_INTERFACE_PROCESSORINTERNAL,
  NDIS_INTERFACE_INTERNALPOWERBUS,
  NDIS_INTERFACE_PNPISABUS,
  NDIS_INTERFACE_PNPBUS,
  NDIS_INTERFACE_MAXIMUM
};

//
// NDIS media types
//

enum ndis_medium {
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

enum ndis_power_profile {
  NDIS_POWER_PROFILE_BATTERY,
  NDIS_POWER_PROFILE_AC_ONLINE
};

enum ndis_device_pnp_event {
    NDIS_DEVICE_PNP_EVENT_QUERY_REMOVED,
    NDIS_DEVICE_PNP_EVENT_REMOVED,
    NDIS_DEVICE_PNP_EVENT_SURPRISE_REMOVED,
    NDIS_DEVICE_PNP_EVENT_QUERY_STOPPED,
    NDIS_DEVICE_PNP_EVENT_STOPPED,
    NDIS_DEVICE_PNP_EVENT_POWER_PROFILE_CHANGED,
    NDIS_DEVICE_PNP_EVENT_MAXIMUM
};

//
// NDIS shutdown handler
//

typedef void (__stdcall *adapter_shutdown_handler)(void *shutdown_context);

//
// NDIS Resource list
//

// Resource types

#define NDIS_RESOURCE_TYPE_NULL                 0
#define NDIS_RESOURCE_TYPE_PORT                 1
#define NDIS_RESOURCE_TYPE_INTERRUPT            2
#define NDIS_RESOURCE_TYPE_MEMORY               3
#define NDIS_RESOURCE_TYPE_DMA                  4
#define NDIS_RESOURCE_TYPE_DEVICESPECIFIC       5
#define NDIS_RESOURCE_TYPE_BUSNUMBER            6
#define NDIS_RESOURCE_TYPE_MAXIMUM              7

// Share disposition

#define NDIS_RESOURCE_SHARE_UNDETERMINED        0
#define NDIS_RESOURCE_SHARE_DEVICE_EXCLUSIVE    1
#define NDIS_RESOURCE_SHARE_DRIVER_EXCLUSIVE    2
#define NDIS_RESOURCE_SHARE_SHARED              3

// Interrupt resource flags

#define NDIS_RESOURCE_INTERRUPT_LEVEL_SENSITIVE 0
#define NDIS_RESOURCE_INTERRUPT_LATCHED         1

// Memory resource flags

#define NDIS_RESOURCE_MEMORY_READ_WRITE         0x0000
#define NDIS_RESOURCE_MEMORY_READ_ONLY          0x0001
#define NDIS_RESOURCE_MEMORY_WRITE_ONLY         0x0002
#define NDIS_RESOURCE_MEMORY_PREFETCHABLE       0x0004
#define NDIS_RESOURCE_MEMORY_COMBINEDWRITE      0x0008
#define NDIS_RESOURCE_MEMORY_24                 0x0010
#define NDIS_RESOURCE_MEMORY_CACHEABLE          0x0020

// I/O port resource flags

#define NDIS_RESOURCE_PORT_MEMORY               0x0000
#define NDIS_RESOURCE_PORT_IO                   0x0001
#define NDIS_RESOURCE_PORT_10_BIT_DECODE        0x0004
#define NDIS_RESOURCE_PORT_12_BIT_DECODE        0x0008
#define NDIS_RESOURCE_PORT_16_BIT_DECODE        0x0010
#define NDIS_RESOURCE_PORT_POSITIVE_DECODE      0x0020
#define NDIS_RESOURCE_PORT_PASSIVE_DECODE       0x0040
#define NDIS_RESOURCE_PORT_WINDOW_DECODE        0x0080

// DMA resource flags

#define NDIS_RESOURCE_DMA_8                     0x0000
#define NDIS_RESOURCE_DMA_16                    0x0001
#define NDIS_RESOURCE_DMA_32                    0x0002
#define NDIS_RESOURCE_DMA_8_AND_16              0x0004
#define NDIS_RESOURCE_DMA_BUS_MASTER            0x0008
#define NDIS_RESOURCE_DMA_TYPE_A                0x0010
#define NDIS_RESOURCE_DMA_TYPE_B                0x0020
#define NDIS_RESOURCE_DMA_TYPE_F                0x0040

#pragma pack(push, 4)

struct ndis_resource_descriptor {
  unsigned char type;
  unsigned char share_disposition;
  unsigned short flags;
  union {
    //
    // Range of resources, inclusive.  These are physical, bus relative.
    // It is known that Port and Memory below have the exact same layout
    // as Generic.
    //

    struct {
      ndis_physical_address_t start;
      unsigned long length;
    } generic;

    //
    // Range of port numbers, inclusive. These are physical, bus
    // relative.
    //

    struct {
      ndis_physical_address_t start;
      unsigned long length;
    } port;

    //
    // IRQL and vector.
    //

    struct {
      unsigned long level;
      unsigned long vector;
      unsigned long affinity;
    } interrupt;

    //
    // Range of memory addresses, inclusive. These are physical, bus
    // relative. 
    //

    struct {
      ndis_physical_address_t start;    // 64 bit physical addresses.
      unsigned long length;
    } memory;

    //
    // Physical DMA channel.
    //

    struct {
      unsigned long channel;
      unsigned long port;
      unsigned long reserved1;
    } dma;

    //
    // Device driver private data, usually used to help it figure
    // what the resource assignments decisions that were made.
    //

    struct {
      unsigned long data[3];
    } device_private;

    //
    // Bus Number information.
    //

    struct {
      unsigned long start;
      unsigned long length;
      unsigned long reserved;
    } busnumber;

    //
    // Device Specific information defined by the driver.
    // The DataSize field indicates the size of the data in bytes. The
    // data is located immediately after the device_specific_data field in
    // the structure.
    //

    struct {
      unsigned long data_size;
      unsigned long reserved1;
      unsigned long reserved2;
    } device_specific_data;
  } u;
};
#pragma pack(pop)

struct ndis_resource_list {
  unsigned short version;
  unsigned short revision;
  unsigned long count;
  struct ndis_resource_descriptor descriptors[0];
};

//
// NDIS miniport characteristics
//

typedef boolean (__stdcall *w_check_for_hang_handler)(
  ndis_handle_t miniport_adapter_context);

typedef void (__stdcall *w_disable_interrupt_handler)(
  ndis_handle_t miniport_adapter_context);

typedef void (__stdcall *w_enable_interrupt_handler)(
  ndis_handle_t miniport_adapter_context);

typedef void (__stdcall *w_halt_handler)(
  ndis_handle_t miniport_adapter_context);

typedef void (__stdcall *w_handle_interrupt_handler)(
  ndis_handle_t miniport_adapter_context);

typedef ndis_status (__stdcall *w_initialize_handler)(
  ndis_status *open_error_status,
  unsigned int *selected_medium_index,
  enum ndis_medium *medium_array,
  unsigned int medium_array_size,
  ndis_handle_t miniport_adapter_handle,
  ndis_handle_t wrapper_configuration_context);

typedef void (__stdcall *w_isr_handler)(
  boolean *interrupt_recognized, 
  boolean *queue_miniport_handle_interrupt, 
  ndis_handle_t miniport_adapter_context);

typedef ndis_status (__stdcall *w_query_information_handler)(
  ndis_handle_t miniport_adapter_context,
  ndis_oid_t oid,
  void *information_buffer,
  unsigned long information_buffer_length,
  unsigned long *bytes_written,
  unsigned long *bytes_needed);

typedef ndis_status (__stdcall *w_reconfigure_handler)(
  ndis_status *open_error_status,
  ndis_handle_t miniport_adapter_context,
  ndis_handle_t wrapper_configuration_context);

typedef ndis_status (__stdcall *w_reset_handler)(
  boolean *addressing_reset,
  ndis_handle_t miniport_adapter_context);

typedef ndis_status (__stdcall *w_send_handler)(
  ndis_handle_t miniport_adapter_context,
  struct ndis_packet *packet,
  unsigned int flags);

typedef ndis_status (__stdcall *w_set_information_handler)(
  ndis_handle_t miniport_adapter_context,
  ndis_oid_t oid,
  void *information_buffer,
  unsigned long information_buffer_length,
  unsigned long *bytes_read,
  unsigned long *bytes_needed);

typedef ndis_status (__stdcall *w_transfer_data_handler)(
  struct ndis_packet *packet,
  unsigned int *bytes_transferred,
  ndis_handle_t miniport_adapter_context,
  ndis_handle_t miniport_receive_context,
  unsigned int byte_offset,
  unsigned int bytes_to_transfer);

typedef void (__stdcall *w_return_packet_handler)(
  ndis_handle_t miniport_adapter_context,
  struct ndis_packet *packet);

typedef void (__stdcall *w_send_packets_handler)(
  ndis_handle_t miniport_adapter_context,
  struct ndis_packet **packet_array,
  unsigned int number_of_packets);

typedef void (__stdcall *w_allocate_complete_handler)(
  ndis_handle_t miniport_adapter_context,
  void *virtual_address,
  ndis_physical_address_t  *physical_address,
  unsigned long length,
  void *context);

typedef ndis_status (__stdcall *w_co_create_vc_handler)(
  ndis_handle_t miniport_adapter_context,
  ndis_handle_t ndis_vc_handle,
  ndis_handle_t *miniport_vc_context);

typedef ndis_status (__stdcall *w_co_delete_vc_handler)(
  ndis_handle_t miniport_vc_context);

typedef ndis_status (__stdcall *w_co_activate_vc_handler)(
  ndis_handle_t miniport_vc_context,
  void *call_parameters); // CO_CALL_PARAMETERS

typedef ndis_status (__stdcall *w_co_deactivate_vc_handler)(
  ndis_handle_t miniport_vc_context);

typedef void (__stdcall *w_co_send_packets_handler)(
  ndis_handle_t miniport_vc_context,
  struct ndis_packet **packet_array,
  unsigned int number_of_packets);

typedef ndis_status (__stdcall *w_co_request_handler)(
  ndis_handle_t miniport_adapter_context,
  ndis_handle_t ndis_vc_handle,
  void *ndis_request); // NDIS_REQUEST

typedef void (__stdcall *w_cancel_send_packets_handler)(
  ndis_handle_t miniport_adapter_context,
  void *cancel_id);

typedef void (__stdcall *w_pnp_event_notify_handler)(
  ndis_handle_t miniport_adapter_context,
  enum ndis_device_pnp_event device_pnp_event,
  void *information_buffer,
  unsigned long information_buffer_length);

typedef void (__stdcall *w_miniport_shutdown_handler) (
  ndis_handle_t miniport_adapter_context);

struct ndis_miniport_characteristics{
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

//
// NDIS DMA size
//

typedef unsigned char ndis_dma_size_t;

#define NDIS_DMA_24BITS             ((ndis_dma_size_t) 0)
#define NDIS_DMA_32BITS             ((ndis_dma_size_t) 1)
#define NDIS_DMA_64BITS             ((ndis_dma_size_t) 2)

//
// NDIS Interrupt
//

enum ndis_interrupt_mode {
  NDIS_INTR_LEVELSENSITIVE,
  NDIS_INTR_LATCHED
};

struct ndis_miniport_interrupt {
  //TODO PKINTERRUPT                 InterruptObject;
  //TODO KSPIN_LOCK                  DpcCountLock;
  void *reserved;
  w_isr_handler miniport_isr;
  w_handle_interrupt_handler miniport_dpc;
  //TODO KDPC                        InterruptDpc;
  struct ndis_miniport_block *miniport;

  unsigned char dpc_count;
  boolean filler1;

  //
  // This is used to tell when all the Dpcs for the adapter are completed.
  //

  //TODO KEVENT                      DpcsCompletedEvent;

  //TODO BOOLEAN                     SharedInterrupt;
  //TODO BOOLEAN                     IsrRequested;
};

//
// NDIS Spin Lock
//

struct ndis_spin_lock {
  int todo;
};

//
// NDIS mini-port block
//

typedef void (__stdcall *filter_packet_indication_handler)(
  ndis_handle_t miniport,
  struct ndis_packet **packet_array,
  unsigned int number_of_packets);

typedef void (__stdcall *ndis_m_send_complete_handler)(
  ndis_handle_t miniport_adapter_handle,
  struct ndis_packet *packet,
  ndis_status status);

typedef void (__stdcall *ndis_m_send_resources_handler)(ndis_handle_t miniport_adapter_handle);

typedef void (__stdcall *ndis_m_reset_complete_handler)(
  ndis_handle_t miniport_adapter_handle,
  ndis_status status,
  boolean addressing_reset);

typedef void (__stdcall *eth_rcv_indicate_handler)(
  void *filter,
  ndis_handle_t mac_receive_context,
  char *address,
  void *header_buffer,
  unsigned int header_buffer_size,
  void *lookahead_buffer,
  unsigned int lookahead_buffer_size,
  unsigned int packet_size);

typedef void (__stdcall *tr_rcv_indicate_handler)(
  void *filter,
  ndis_handle_t mac_receive_context,
  void *header_buffer,
  unsigned int header_buffer_size,
  void *lookahead_buffer,
  unsigned int lookahead_buffer_size,
  unsigned int packet_size);

typedef void (__stdcall *fddi_rcv_indicate_handler)(
  void *filter,
  ndis_handle_t mac_receive_context,
  char *address,
  unsigned int address_length,
  void *header_buffer,
  unsigned int header_buffer_size,
  void *lookahead_buffer,
  unsigned int lookahead_buffer_size,
  unsigned int packet_size);

typedef void (__stdcall *eth_rcv_complete_handler)(void *filter);
typedef void (__stdcall *tr_rcv_complete_handler)(void *filter);
typedef void (__stdcall *fddi_rcv_complete_handler)(void *filter);

typedef void (__stdcall *ndis_m_status_handler)(
  ndis_handle_t miniport_handle,
  ndis_status general_status,
  void *status_buffer,
  unsigned int status_buffer_size);

typedef void (__stdcall *ndis_m_sts_complete_handler)(ndis_handle_t miniport_adapter_handle);

typedef void (__stdcall *ndis_m_td_complete_handler)(
  ndis_handle_t miniport_adapter_handle,
  struct ndis_packet *packet,
  ndis_status status,
  unsigned int bytes_transferred);

typedef void (__stdcall *ndis_m_req_complete_handler)(
  ndis_handle_t miniport_adapter_handle,
  ndis_status status);

typedef void (__stdcall *ndis_wm_send_complete_handler)(
  ndis_handle_t miniport_adapter_handle,
  void *packet,
  ndis_status status);

typedef void (__stdcall *wan_rcv_handler)(
  ndis_status *status,
  ndis_handle_t miniport_adapter_handle,
  ndis_handle_t ndis_link_context,
  unsigned char *packet,
  unsigned long packet_size);

typedef void (__stdcall *wan_rcv_complete_handler)(
  ndis_handle_t miniport_adapter_handle,
  ndis_handle_t ndis_link_context);

struct ndis_filter_db {
  void *ethdb;
  void *trdb;
  void *fddidb;
  void *arcdb;
};

struct ndis_miniport_block {
  char reserved1[216];
  struct ndis_filter_db filterdb;

  filter_packet_indication_handler packet_indicate_handler;
  ndis_m_send_complete_handler send_complete_handler;
  ndis_m_send_resources_handler send_resources_handler;
  ndis_m_reset_complete_handler reset_complete_handler;

  char reserved2[108];

  eth_rcv_indicate_handler eth_rx_indicate_handler;
  tr_rcv_indicate_handler tr_rx_indicate_handler;
  fddi_rcv_indicate_handler fddi_rx_indicate_handler;

  eth_rcv_complete_handler eth_rx_complete_handler;
  tr_rcv_complete_handler tr_rx_complete_handler;
  fddi_rcv_complete_handler fddi_rx_complete_handler;

  ndis_m_status_handler status_handler;
  ndis_m_sts_complete_handler status_complete_handler;
  ndis_m_td_complete_handler td_complete_handler;
  ndis_m_req_complete_handler query_complete_handler;
  ndis_m_req_complete_handler set_complete_handler;

  ndis_wm_send_complete_handler wan_send_complete_handler;
  wan_rcv_handler wan_rcv_handler;
  wan_rcv_complete_handler wan_rcv_complete_handler;
};

//
// NDIS driver
//

struct ndis_adapter;

struct ndis_driver {
  struct ndis_driver *next;
  hmodule_t hmod;
  struct ndis_adapter *adapters;
  int numadapters;
  struct ndis_miniport_characteristics handlers;
};

struct ndis_adapter {
  struct ndis_miniport_block callbacks;
  struct ndis_adapter *next;
  struct unit *unit;
  struct ndis_driver *driver;
  ndis_handle_t context;
  dev_t devno;
  int adapterno;
};

struct ndis_property {
  char *name;
  struct ndis_property *next;
  struct ndis_configuration_parameter value;
};

struct ndis_config {
  struct section *sect;
  struct ndis_property *propcache;
  struct eth_addr hwaddr;
};

#endif
