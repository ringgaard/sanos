//
// ndis.c
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

#include <os/krnl.h>
#include "ndis.h"

#define NDISTRACE(s) kprintf("ndis: %s called\n", s);

//#define ndisapi __declspec(dllexport) __stdcall
#define ndisapi

//
// NTOSKRNL functions
//

boolean ndisapi RtlEqualUnicodeString(const wchar_t *string1, const wchar_t *string2, boolean case_insensitive)
{
  NDISTRACE("RtlEqualUnicodeString");
  return 0;
}

unsigned long __cdecl DbgPrint(char *format, ...)
{
  va_list args;
  char buffer[1024];
  int len;

  va_start(args, format);
  len = vsprintf(buffer, format, args);
  va_end(args);
    
  kprintf("ndis: %s\n", buffer);
  return len;
}

//
// Port I/O (from HAL.DLL)
//

unsigned char ndisapi READ_PORT_UCHAR(unsigned char *port)
{
  NDISTRACE("READ_PORT_UCHAR");
  return _inp((unsigned short) port);
}

void ndisapi WRITE_PORT_USHORT(unsigned short *port, unsigned short value)
{
  NDISTRACE("WRITE_PORT_USHORT");
  _outpw((unsigned short) port, value);
}

unsigned short ndisapi READ_PORT_USHORT(unsigned short *port)
{
  NDISTRACE("READ_PORT_USHORT");
  return _inpw((unsigned short) port);
}

void ndisapi KeStallExecutionProcessor(unsigned long microseconds)
{
  NDISTRACE("KeStallExecutionProcessor");
  usleep(microseconds);
}

//
// NDIS Initialization and Registration Functions
//

void ndisapi NdisInitializeWrapper(ndis_handle_t *ndis_wrapper_handle, void *system_specific1, void *system_specific2, void *system_specific3)
{
  NDISTRACE("NdisInitializeWrapper");
}

void ndisapi NdisTerminateWrapper(ndis_handle_t ndis_wrapper_handle, void *system_specific)
{
  NDISTRACE("NdisTerminateWrapper");
}

ndis_status ndisapi NdisMRegisterMiniport(ndis_handle_t ndis_wrapper_handle, struct ndis_miniport_characteristics *miniport_characteristics, unsigned int characteristics_length)
{
  NDISTRACE("NdisMRegisterMiniport");
  return 0;
}

void ndisapi NdisMSetAttributesEx
(
  ndis_handle_t miniport_adapter_handle,
  ndis_handle_t miniport_adapter_context,
  unsigned int hang_check_interval,
  unsigned long attribute_flags,
  enum ndis_interface_type adapter_type
)
{
  NDISTRACE("NdisMSetAttributesEx");
}

void ndisapi NdisOpenConfiguration(ndis_status *status, ndis_handle_t *configuration_handle, ndis_handle_t wrapper_configuration_context)
{
  NDISTRACE("NdisOpenConfiguration");
}

void ndisapi NdisCloseConfiguration(ndis_handle_t configuration_handle)
{
  NDISTRACE("NdisCloseConfiguration");
}

void ndisapi NdisReadConfiguration
(
  ndis_status *status,
  struct ndis_configuration_parameter **parameter_value,
  ndis_handle_t configuration_handle,
  struct ndis_string *keyword,
  enum ndis_parameter_type parameter_type
)
{
  NDISTRACE("NdisReadConfiguration");
}

void ndisapi NdisReadNetworkAddress
(
  ndis_status *status,
  void **network_address,
  unsigned int *network_address_length,
  ndis_handle_t configuration_handle
)
{
  NDISTRACE("NdisReadNetworkAddress");
}

void ndisapi NdisMRegisterAdapterShutdownHandler
(
  ndis_handle_t miniport_handle,
  void *shutdown_context,
  adapter_shutdown_handler shutdown_handler
)
{
  NDISTRACE("NdisMRegisterAdapterShutdownHandler");
}

void ndisapi NdisMDeregisterAdapterShutdownHandler(ndis_handle_t miniport_handle)
{
  NDISTRACE("NdisMDeregisterAdapterShutdownHandler");
}

void ndisapi NdisMQueryAdapterResources
(
  ndis_status *status,
  ndis_handle_t wrapper_configuration_context,
  struct ndis_resource_list *resource_list,
  unsigned int *buffer_size
)
{
  NDISTRACE("NdisMQueryAdapterResources");
}

//
// NDIS Hardware Configuration Functions
//

unsigned long ndisapi NdisReadPciSlotInformation
(
  ndis_handle_t ndis_adapter_handle, 
  unsigned long slot_number, 
  unsigned long offset, 
  void *buffer, 
  unsigned long length
)
{
  NDISTRACE("NdisReadPciSlotInformation");
  return 0;
}

unsigned long ndisapi NdisWritePciSlotInformation
(
  ndis_handle_t ndis_adapter_handle, 
  unsigned long slot_number, 
  unsigned long offset, 
  void *buffer, 
  unsigned long length
)
{
  NDISTRACE("NdisWritePciSlotInformation");
  return 0;
}

//
// NDIS I/O Port Functions
//

ndis_status ndisapi NdisMRegisterIoPortRange
(
  void **port_offset,
  ndis_handle_t miniport_adapter_handle,
  unsigned int initial_port,
  unsigned int number_of_ports
)
{
  NDISTRACE("NdisMRegisterIoPortRange");
  return 0;
}

void ndisapi NdisMDeregisterIoPortRange
(
  ndis_handle_t miniport_adapter_handle,
  unsigned int initial_port,
  unsigned int number_of_ports,
  void *port_offset
)
{
  NDISTRACE("NdisMDeregisterIoPortRange");
}

//
// NDIS DMA-Related Functions
//

void ndisapi NdisMAllocateSharedMemory
(
  ndis_handle_t miniport_adapter_handle,
  unsigned long length,
  boolean cached,
  void **virtual_address,
  ndis_physical_address_t *physical_address
)
{
  NDISTRACE("NdisMAllocateSharedMemory");
}

void ndisapi NdisMFreeSharedMemory
(
  ndis_handle_t miniport_adapter_handle,
  unsigned long length,
  boolean cached,
  void *virtual_address,
  ndis_physical_address_t physical_address
)
{
  NDISTRACE("NdisMFreeSharedMemory");
}

void ndisapi NdisMAllocateMapRegisters
(
  ndis_handle_t miniport_adapter_handle,
  unsigned int dma_channel,
  ndis_dma_size_t dma_size,
  unsigned long base_map_registers_needed,
  unsigned long maximum_physical_mapping
)
{
  NDISTRACE("NdisMAllocateMapRegisters");
}

void ndisapi NdisMFreeMapRegisters(ndis_handle_t miniport_adapter_handle)
{
  NDISTRACE("NdisMFreeMapRegisters");
}

//
// NDIS Interrupt Handling Functions
//

ndis_status ndisapi NdisMRegisterInterrupt
(
  struct ndis_miniport_interrupt *interrupt,
  ndis_handle_t miniport_adapter_handle,
  unsigned int interrupt_vector,
  unsigned int interrupt_level,
  boolean request_isr,
  boolean shared_interrupt,
  enum ndis_interrupt_mode interrupt_mode
)
{
  NDISTRACE("NdisMRegisterInterrupt");
  return 0;
}

void ndisapi NdisMDeregisterInterrupt(struct ndis_miniport_interrupt *interrupt)
{
  NDISTRACE("NdisMDeregisterInterrupt");
}

boolean ndisapi NdisMSynchronizeWithInterrupt
(
  struct ndis_miniport_interrupt *interrupt,
  void *synchronize_function,
  void *synchronize_context
)
{
  NDISTRACE("NdisMSynchronizeWithInterrupt");
  return 0;
}

//
// NDIS Synchronization Functions
//

void ndisapi NdisMInitializeTimer(struct ndis_miniport_timer *timer, ndis_handle_t miniport_adapter_handle, ndis_timer_func_t  timer_function, void *function_context)
{
  NDISTRACE("NdisMInitializeTimer");
}

void ndisapi NdisMCancelTimer(struct ndis_miniport_timer *timer, boolean *timer_cancelled)
{
  NDISTRACE("NdisMCancelTimer");
}

void ndisapi NdisMSetPeriodicTimer(struct ndis_miniport_timer *timer, unsigned int millisecond_period)
{
  NDISTRACE("NdisMSetPeriodicTimer");
}

void ndisapi NdisAllocateSpinLock(struct ndis_spin_lock *spin_lock)
{
  NDISTRACE("NdisAllocateSpinLock");
}

void ndisapi NdisFreeSpinLock(struct ndis_spin_lock *spin_lock)
{
  NDISTRACE("NdisFreeSpinLock");
}

void ndisapi NdisAcquireSpinLock(struct ndis_spin_lock *spin_lock)
{
  NDISTRACE("NdisAcquireSpinLock");
}

void ndisapi NdisReleaseSpinLock(struct ndis_spin_lock *spin_lock)
{
  NDISTRACE("NdisReleaseSpinLock");
}

//
// NDIS Query and Set Completion Functions
//

//
// NDIS Status Indication Functions
//

//
// NDIS Send and Receive Functions for Connectionless Miniport Drivers
//

//
// NDIS Send and Receive Functions for Connection-Oriented Miniport Drivers
//

//
// NDIS Packet and Buffer Handling Functions
//

void ndisapi NdisAllocatePacketPool
(
  ndis_status *status, 
  ndis_handle_t *pool_handle, 
  unsigned int number_of_descriptors, 
  unsigned int protocol_reserved_length
)
{
  NDISTRACE("NdisAllocatePacketPool");
}

void ndisapi NdisFreePacketPool(ndis_handle_t pool_handle)
{
  NDISTRACE("NdisFreePacketPool");
}

void ndisapi NdisAllocatePacket(ndis_status *status, struct ndis_packet **packet, ndis_handle_t pool_handle)
{
  NDISTRACE("NdisAllocatePacket");
}

void ndisapi NdisFreePacket(struct ndis_packet *packet)
{
  NDISTRACE("NdisFreePacket");
}

void ndisapi NdisAllocateBufferPool
(
  ndis_status *status, 
  ndis_handle_t *pool_handle, 
  unsigned int number_of_descriptors
)
{
  NDISTRACE("NdisAllocateBufferPool");
}

void ndisapi NdisFreeBufferPool(ndis_handle_t pool_handle)
{
  NDISTRACE("NdisFreeBufferPool");
}

void ndisapi NdisAllocateBuffer
(
  ndis_status *status,
  struct ndis_buffer **Buffer,
  ndis_handle_t pool_handle,
  void *virtual_address,
  unsigned int length
)
{
  NDISTRACE("NdisAllocateBuffer");
}

void ndisapi NdisFreeBuffer(struct ndis_buffer *buffer)
{
  NDISTRACE("NdisFreeBuffer");
}

void ndisapi NdisQueryBuffer(struct ndis_buffer *buffer, void **virtual_address,  unsigned int *length)
{
  NDISTRACE("NdisQueryBuffer");
}

void ndisapi NdisAdjustBufferLength(struct ndis_buffer *buffer, unsigned int length)
{
  NDISTRACE("NdisAdjustBufferLength");
}

void ndisapi NdisQueryBufferOffset(struct ndis_buffer *buffer,  unsigned int *offset, unsigned int *length)
{
  NDISTRACE("NdisQueryBufferOffset");
}

void ndisapi NdisUnchainBufferAtFront(struct ndis_packet *packet, struct ndis_buffer *buffer)
{
  NDISTRACE("NdisUnchainBufferAtFront");
}

unsigned long ndisapi NDIS_BUFFER_TO_SPAN_PAGES(struct ndis_buffer *buffer)
{
  NDISTRACE("NDIS_BUFFER_TO_SPAN_PAGES");
  return 0;
}

//
// NDIS Memory Support Functions
//

ndis_status ndisapi NdisAllocateMemoryWithTag(void **virtual_address, unsigned int length, unsigned long tag)
{
  NDISTRACE("NdisAllocateMemoryWithTag");
  return 0;
}

void ndisapi NdisFreeMemory(void *virtual_address, unsigned int length, unsigned int memory_flags)
{
  NDISTRACE("NdisFreeMemory");
}

//
// NDIS Logging Support Functions
//

void ndisapi NdisWriteErrorLogEntry(ndis_handle_t ndis_adapter_handle, unsigned long error_code, unsigned long errvals, ...)
{
  NDISTRACE("NdisWriteErrorLogEntry");
}

//
// Module initialization
//

int __stdcall start(hmodule_t hmod, int reason, void *reserved2)
{
  return 1;
}
