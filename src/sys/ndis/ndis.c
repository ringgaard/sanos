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

#define ndisapi __declspec(dllexport) __stdcall

//
// NTOSKRNL functions
//

boolean ndisapi RtlEqualUnicodeString(const wchar_t *string1, const wchar_t *string2, boolean case_insensitive)
{
  return 0;
}

unsigned long __cdecl DbgPrint(char *format, ...)
{
  return 0;
}

//
// Port I/O (from HAL.DLL)
//

unsigned char ndisapi READ_PORT_UCHAR(unsigned char *port)
{
  return inp((unsigned short) port);
}

void ndisapi WRITE_PORT_USHORT(unsigned short *port, unsigned short value)
{
  outpw((unsigned short) port, value);
}

unsigned short ndisapi READ_PORT_USHORT(unsigned short *port)
{
  return inpw((unsigned short) port);
}

void ndisapi KeStallExecutionProcessor(unsigned long microseconds)
{
  usleep(microseconds);
}

//
// Initialization and termination.
//

void ndisapi NdisInitializeWrapper(ndis_handle_t ndis_wrapper_handle, void *system_specific1, void *system_specific2, void *system_specific3)
{
}

void ndisapi NdisTerminateWrapper(ndis_handle_t ndis_wrapper_handle, void *system_specific)
{
}

ndis_status ndisapi NdisMRegisterMiniport(ndis_handle_t ndis_wrapper_handle, struct ndis_miniport_characteristics *miniport_characteristics, unsigned int characteristics_length)
{
  return 0;
}

//
// Timers
//

void ndisapi NdisMInitializeTimer(struct ndis_miniport_timer *timer, ndis_handle_t miniport_adapter_handle, ndis_timer_func_t  timer_function, void *function_context)
{
}

void ndisapi NdisMCancelTimer(struct ndis_miniport_timer *timer, boolean *timer_cancelled)
{
}

void ndisapi NdisMSetPeriodicTimer(struct ndis_miniport_timer *timer, unsigned int millisecond_period)
{
}

//
// Memory Allocation
//

ndis_status ndisapi NdisAllocateMemoryWithTag(void **virtual_address, unsigned int length, unsigned long tag)
{
  return 0;
}

void ndisapi NdisFreeMemory(void *virtual_address, unsigned int length, unsigned int memory_flags)
{
}

//
// Packets
//

void ndisapi NdisAllocatePacketPool(ndis_status *status, ndis_handle_t *pool_handle, unsigned int number_of_descriptors, unsigned int protocol_reserved_length)
{
}

void ndisapi NdisFreePacketPool(ndis_handle_t pool_handle)
{
}

void ndisapi NdisAllocatePacket(ndis_status *status, struct ndis_packet **packet, ndis_handle_t pool_handle)
{
}

void ndisapi NdisFreePacket(struct ndis_packet *packet)
{
}

//
// Buffers
//

void ndisapi NdisAllocateBufferPool()
{
}

void ndisapi NdisFreeBufferPool()
{
}

void ndisapi NdisAllocateBuffer()
{
}

void ndisapi NdisFreeBuffer()
{
}

void ndisapi NdisQueryBuffer()
{
}

void ndisapi NdisAdjustBufferLength()
{
}

void ndisapi NdisQueryBufferOffset()
{
}

void ndisapi NdisUnchainBufferAtFront()
{
}

void ndisapi NDIS_BUFFER_TO_SPAN_PAGES()
{
}

//
// Configuration
//

void ndisapi NdisOpenConfiguration()
{
}

void ndisapi NdisCloseConfiguration()
{
}

void ndisapi NdisReadConfiguration()
{
}

void ndisapi NdisReadNetworkAddress()
{
}

//
// Misc
//

void ndisapi NdisMAllocateSharedMemory()
{
}

void ndisapi NdisMFreeSharedMemory()
{
}

void ndisapi NdisMAllocateMapRegisters()
{
}

void ndisapi NdisMFreeMapRegisters()
{
}

void ndisapi NdisAllocateSpinLock()
{
}

void ndisapi NdisFreeSpinLock()
{
}

void ndisapi NdisAcquireSpinLock()
{
}

void ndisapi NdisReleaseSpinLock()
{
}

void ndisapi NdisReadPciSlotInformation()
{
}

void ndisapi NdisWritePciSlotInformation()
{
}

void ndisapi NdisMSynchronizeWithInterrupt()
{
}

void ndisapi NdisMDeregisterAdapterShutdownHandler()
{
}

void ndisapi NdisMDeregisterIoPortRange()
{
}

void ndisapi NdisMDeregisterInterrupt()
{
}

void ndisapi NdisMRegisterInterrupt()
{
}

void ndisapi NdisMSetAttributesEx()
{
}

void ndisapi NdisWriteErrorLogEntry()
{
}

void ndisapi NdisMRegisterIoPortRange()
{
}

void ndisapi NdisMRegisterAdapterShutdownHandler()
{
}

void ndisapi NdisMQueryAdapterResources()
{
}

int __stdcall start(hmodule_t hmod, int reason, void *reserved2)
{
  return 1;
}
