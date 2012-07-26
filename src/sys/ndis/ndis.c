//
// ndis.c
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

#include <os/krnl.h>
#include "ndis.h"

#define NDISTRACE(s) kprintf("ndis: %s called\n", s);
#define ndisapi __declspec(dllexport) __stdcall

struct ndis_driver *drivers = NULL;

//
// Configuration functions
//

static struct ndis_config *open_config(struct section *sect) {
  struct ndis_config *cfg;

  cfg = kmalloc(sizeof(struct ndis_config));
  if (!cfg) return NULL;
  cfg->sect = sect;
  cfg->propcache = NULL;

  return cfg;
}

static void free_config(struct ndis_config *cfg) {
  struct ndis_property *prop;
  struct ndis_property *next;

  if (!cfg) return;
  prop = cfg->propcache;
  while (prop) {
    next = prop->next;

    switch (prop->value.parameter_type) {
      case NDIS_PARAMETER_INTEGER:
      case NDIS_PARAMETER_HEXINTEGER:
        break;

      case NDIS_PARAMETER_STRING:
        kfree(prop->value.parameter_data.string_data.buffer);
        break;

      case NDIS_PARAMETER_MULTISTRING:
        panic("unsupported ndis property type");
        break;

      case NDIS_PARAMETER_BINARY:
        kfree(prop->value.parameter_data.binary_data.buffer);
        break;
    }

    kfree(prop);
    prop = next;
  }

  kfree(cfg);
}

static int hexdigit(int digit) {
  if (digit >= '0' && digit <= '9') return digit - '0';
  if (digit >= 'A' && digit <= 'F') return digit - 'A';
  if (digit >= 'a' && digit <= 'f') return digit - 'a';
  return 0;
}

static struct ndis_configuration_parameter *read_config(struct ndis_config *cfg, char *name, enum ndis_parameter_type type) {
  struct ndis_property *np;
  struct property *prop;
  char *value;
  wchar_t *buf;
  unsigned char *data;
  int len;
  int i;

  // Fail if no config or no config section
  if (!cfg) return NULL;
  if (!cfg->sect) return NULL;

  // Try to find property in the property cache
  np = cfg->propcache;
  while (np) {
    if (stricmp(np->name, name) == 0) {
      if (np->value.parameter_type != type) {
        kprintf("ndis: wrong type for parameter %s\n", name);
        return NULL;
      }

      kprintf("ndis: cached parameter %s\n", name);
      return &np->value;
    }
    np = np->next;
  }

  // Try to find the property in the configuration section
  prop = cfg->sect->properties;
  while (prop) {
    if (stricmp(prop->name, name) == 0) break;
    prop = prop->next;
  }

  if (!prop) {
    kprintf("ndis: parameter %s not found\n", name);
    return NULL;
  }

  if (prop->value) {
    value = prop->value;
  } else {
    value = "";
  }

  // Add property to property cache
  np = kmalloc(sizeof(struct ndis_property));
  if (!np) return NULL;
  memset(np, 0, sizeof(struct ndis_property));

  np->name = prop->name;
  np->value.parameter_type = type;

  switch (np->value.parameter_type) {
    case NDIS_PARAMETER_INTEGER:
      np->value.parameter_data.integer_data = strtoul(value, NULL, 0);
      break;

    case NDIS_PARAMETER_HEXINTEGER:
      np->value.parameter_data.integer_data = strtoul(value, NULL, 16);
      break;

    case NDIS_PARAMETER_STRING:
      len = strlen(value);
      buf = kmalloc(sizeof(wchar_t) * (len + 1));
      if (!buf) return NULL;
      for (i = 0; i < len; i++) buf[i] = value[i];
      buf[len] = 0;
      np->value.parameter_data.string_data.buffer = buf;
      np->value.parameter_data.string_data.length = len * sizeof(wchar_t);
      np->value.parameter_data.string_data.maxlength = (len + 1) * sizeof(wchar_t);
      break;

    case NDIS_PARAMETER_MULTISTRING:
      kprintf("ndis: '%s', multisz properties not supported\n", name);
      break;

    case NDIS_PARAMETER_BINARY:
      len = strlen(value) / 2;
      data = kmalloc(len);
      if (!data) return NULL;
      for (i = 0; i < len; i++) data[i] = hexdigit(value[i * 2]) + (hexdigit(value[i * 2 + 1]) << 4);
      np->value.parameter_data.binary_data.buffer = data;
      np->value.parameter_data.binary_data.length = len;
      break;
  }

  np->next = cfg->propcache;
  cfg->propcache = np;

  return &np->value;
}

static struct section *get_config_section(struct ndis_adapter *adapter) {
  char buf[MAXPATH];
  char *cfgname;
  char *p;
  char *lastdot;
  struct section *sect;

  if (getmodpath(adapter->driver->hmod, buf, sizeof buf) < 0) return NULL;
  cfgname = buf;
  lastdot = NULL;
  
  p = buf;
  while (*p) {
    if (*p == PS1 || *p == PS2) cfgname = p + 1;
    if (*p == '.') lastdot = p;
    p++;
  }
  if (lastdot > cfgname) *lastdot = 0;
  
  p = cfgname + strlen(cfgname);
  *p++ = '-';
  *p++ = '0' + adapter->adapterno;
  *p = 0;

  sect = krnlcfg;
  while (sect) {
    if (stricmp(sect->name, cfgname) == 0) return sect;
    sect = sect->next;
  }

  return NULL;
}

//
// NTOSKRNL functions
//

boolean ndisapi RtlEqualUnicodeString(const struct ndis_string *string1, const struct ndis_string *string2, boolean case_insensitive) {
  wchar_t *s1, *s2;
  wchar_t f, l;
  unsigned int n;

  NDISTRACE("RtlEqualUnicodeString");
  
  {
    char buf[256];
    wchar_t *p;
    char *q;

    p = string1->buffer;
    q = buf;
    while (*p) *q++ = (char) *p++;
    *q = 0;

    kprintf("string1='%s'\n", buf);

    p = string2->buffer;
    q = buf;
    while (*p) *q++ = (char) *p++;
    *q = 0;

    kprintf("string2='%s'\n", buf);
  }

  kprintf("len=%d %d\n", string1->length, string2->length);
  if (string1->length != string2->length) return 0;

  s1 = string1->buffer;
  s2 = string2->buffer;
  for (n = 0; n < string1->length / sizeof(wchar_t); n++) {
    if (case_insensitive) {
      f = ((*s1 <= 'Z') && (*s1 >= 'A')) ? *s1 + 'a' - 'A' : *s1;
      l = ((*s2 <= 'Z') && (*s2 >= 'A')) ? *s2 + 'a' - 'A' : *s2;
    } else {
      f = *s1;
      l = *s2;
    }

    if (f != l) return 0;
    s1++;
    s2++;
  }

  kprintf("match\n");
  return 1;
}

unsigned long __cdecl DbgPrint(char *format, ...) {
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

unsigned char ndisapi READ_PORT_UCHAR(unsigned char *port) {
  //NDISTRACE("READ_PORT_UCHAR");
  kprintf("ndis: inp(0x%x)\n", port);
  return inp((unsigned short) port);
}

void ndisapi WRITE_PORT_USHORT(unsigned short *port, unsigned short value) {
  //NDISTRACE("WRITE_PORT_USHORT");
  kprintf("ndis: outpw(0x%x,0x%x)\n", port, value);
  outpw((unsigned short) port, value);
}

unsigned short ndisapi READ_PORT_USHORT(unsigned short *port) {
  //NDISTRACE("READ_PORT_USHORT");
  kprintf("ndis: inp(0x%x)\n", port);
  return inpw((unsigned short) port);
}

void ndisapi KeStallExecutionProcessor(unsigned long microseconds) {
  NDISTRACE("KeStallExecutionProcessor");
  udelay(microseconds);
}

//
// NDIS Initialization and Registration Functions
//

void ndisapi NdisInitializeWrapper(ndis_handle_t *ndis_wrapper_handle, void *system_specific1, void *system_specific2, void *system_specific3) {
  struct ndis_driver *driver = (struct ndis_driver *) system_specific1;
  NDISTRACE("NdisInitializeWrapper");
  *ndis_wrapper_handle = driver;
}

void ndisapi NdisTerminateWrapper(ndis_handle_t ndis_wrapper_handle, void *system_specific) {
  NDISTRACE("NdisTerminateWrapper");
}

ndis_status ndisapi NdisMRegisterMiniport(ndis_handle_t ndis_wrapper_handle, struct ndis_miniport_characteristics *miniport_characteristics, unsigned int characteristics_length) {
  struct ndis_driver *driver = (struct ndis_driver *) ndis_wrapper_handle;
  int len;

  NDISTRACE("NdisMRegisterMiniport");
  len = characteristics_length;
  if (len > sizeof(struct ndis_miniport_characteristics)) len = sizeof(struct ndis_miniport_characteristics);
  memcpy(&driver->handlers, miniport_characteristics, len);

  return 0;
}

void ndisapi NdisMSetAttributesEx(
    ndis_handle_t miniport_adapter_handle,
    ndis_handle_t miniport_adapter_context,
    unsigned int hang_check_interval,
    unsigned long attribute_flags,
    enum ndis_interface_type adapter_type) {
  NDISTRACE("NdisMSetAttributesEx");
}

void ndisapi NdisOpenConfiguration(ndis_status *status, ndis_handle_t *configuration_handle, ndis_handle_t wrapper_configuration_context) {
  struct ndis_adapter *adapter = (struct ndis_adapter *) wrapper_configuration_context;
  struct section *sect;
  struct ndis_config *cfg;

  NDISTRACE("NdisOpenConfiguration");

  sect = get_config_section(adapter);
  cfg = open_config(sect);
  if (!cfg) {
    *status = NDIS_STATUS_FAILURE;
    return;
  }

  *configuration_handle = cfg;
  *status = 0;
}

void ndisapi NdisCloseConfiguration(ndis_handle_t configuration_handle) {
  struct ndis_config *cfg = (struct ndis_config *) configuration_handle;

  NDISTRACE("NdisCloseConfiguration");
  free_config(cfg);
}

void ndisapi NdisReadConfiguration(
    ndis_status *status,
    struct ndis_configuration_parameter **parameter_value,
    ndis_handle_t configuration_handle,
    struct ndis_string *keyword,
    enum ndis_parameter_type parameter_type) {
  struct ndis_config *cfg = (struct ndis_config *) configuration_handle;
  char buf[256];
  int len;
  int i;

  //NDISTRACE("NdisReadConfiguration");

  // Parameter names are restricted to max 255 chars
  len = keyword->length / sizeof(wchar_t);
  if (len > sizeof(buf) - 1) {
    kprintf("ndis: parameter name too long\n");
    *status = NDIS_STATUS_RESOURCES;
    return;
  }

  // Convert parameters name from unicode to ansi
  for (i = 0; i < len; i++) buf[i] = (char) keyword->buffer[i];
  buf[len] = 0;

  kprintf("ndis: read config %s len %d type %d\n", buf, len, parameter_type);

  // Read property from configuration
  *parameter_value = read_config(cfg, buf, parameter_type);
  if (!*parameter_value) {
    *status = NDIS_STATUS_FAILURE;
    return;
  }

  kprintf("ndis: value: %d\n", (*parameter_value)->parameter_data.integer_data);

  *status = 0;
}

void ndisapi NdisReadNetworkAddress(
    ndis_status *status,
    void **network_address,
    unsigned int *network_address_length,
    ndis_handle_t configuration_handle) {
  struct ndis_config *cfg = (struct ndis_config *) configuration_handle;
  struct ndis_configuration_parameter *param;
  wchar_t *p;
  unsigned char *q;
  int addrlen;

  NDISTRACE("NdisReadNetworkAddress");

  param = read_config(cfg, "NetworkAddress", NDIS_PARAMETER_STRING);
  if (!param) {
    *status = NDIS_STATUS_FAILURE;
    return;
  }

  p = param->parameter_data.string_data.buffer;
  q = cfg->hwaddr.addr;
  addrlen = 0;
  while (*p) {
    if (addrlen > ETHER_ADDR_LEN) break;
    if (*p == '-') *p++;
    if (*p)  {
      *q = hexdigit(*p++);
      if (*p) *q = (*q << 4) | hexdigit(*p++);
      q++;
      addrlen++;
    }
  }

 *network_address = &cfg->hwaddr;
 *network_address_length = addrlen;
 *status = 0;
}

void ndisapi NdisMRegisterAdapterShutdownHandler(
    ndis_handle_t miniport_handle,
    void *shutdown_context,
    adapter_shutdown_handler shutdown_handler) {
  NDISTRACE("NdisMRegisterAdapterShutdownHandler");
}

void ndisapi NdisMDeregisterAdapterShutdownHandler(ndis_handle_t miniport_handle) {
  NDISTRACE("NdisMDeregisterAdapterShutdownHandler");
}

void ndisapi NdisMQueryAdapterResources(
    ndis_status *status,
    ndis_handle_t wrapper_configuration_context,
    struct ndis_resource_list *resource_list,
    unsigned int *buffer_size) {
  struct ndis_adapter *adapter = (struct ndis_adapter *) wrapper_configuration_context;
  struct resource *res;
  struct ndis_resource_descriptor *desc;
  unsigned int buflen;
  int resno;

  NDISTRACE("NdisMQueryAdapterResources");

  buflen = sizeof(struct ndis_resource_list);
  resno = 0;

  if (*buffer_size < buflen) {
    *status = NDIS_STATUS_RESOURCES;
    return;
  }

  resource_list->version = 1;
  resource_list->revision = 1;
  
  res = adapter->unit->resources;
  desc = resource_list->descriptors;
  while (res) {
    buflen += sizeof(struct ndis_resource_descriptor);
    if (*buffer_size < buflen) {
      *status = NDIS_STATUS_RESOURCES;
      return;
    }

    switch (res->type) {
      case RESOURCE_IO:
        desc->type = NDIS_RESOURCE_TYPE_PORT;
        desc->flags = NDIS_RESOURCE_PORT_IO;
        desc->u.port.start = res->start;
        desc->u.port.length = res->len;
        break;

      case RESOURCE_MEM:
        desc->type = NDIS_RESOURCE_TYPE_MEMORY;
        desc->flags = 0;
        desc->u.memory.start = res->start;
        desc->u.memory.length = res->len;
        break;

      case RESOURCE_IRQ:
        desc->type = NDIS_RESOURCE_TYPE_INTERRUPT;
        desc->flags = 0;
        desc->u.interrupt.level = res->start;
        desc->u.interrupt.vector = res->start;
        desc->u.interrupt.affinity = -1;
        break;

      case RESOURCE_DMA:
        desc->type = NDIS_RESOURCE_TYPE_DMA;
        desc->flags = 0;
        desc->u.dma.channel = res->start;
        desc->u.dma.port = 0;
        desc->u.dma.reserved1 = 0;
        break;
    }

    resource_list->descriptors[resno].share_disposition = NDIS_RESOURCE_SHARE_UNDETERMINED;

    resno++;
    desc++;
    res = res->next;
  }
  
  // TEST
  //buflen += sizeof(struct ndis_resource_descriptor);
  //desc->type = RESOURCE_DMA;
  //desc->u.dma.channel = 0x0F;
  //resno++;

  kprintf("ndis: returned %d resource descriptors\n", resno);

  resource_list->count = resno;
  *buffer_size = buflen;
  *status = 0;
}

//
// NDIS Hardware Configuration Functions
//

unsigned long ndisapi NdisReadPciSlotInformation(
    ndis_handle_t ndis_adapter_handle, 
    unsigned long slot_number, 
    unsigned long offset, 
    void *buffer, 
    unsigned long length) {
  struct ndis_adapter *adapter = (struct ndis_adapter *) ndis_adapter_handle;

  NDISTRACE("NdisReadPciSlotInformation");
  kprintf("ndis: read pci slot %d offset %d length %d\n", slot_number, offset, length);

  pci_read_buffer(adapter->unit, offset, buffer, length);

  return 0;
}

unsigned long ndisapi NdisWritePciSlotInformation(
    ndis_handle_t ndis_adapter_handle, 
    unsigned long slot_number, 
    unsigned long offset, 
    void *buffer, 
    unsigned long length) {
  struct ndis_adapter *adapter = (struct ndis_adapter *) ndis_adapter_handle;

  NDISTRACE("NdisWritePciSlotInformation");
  kprintf("ndis: write pci slot %d offset %d length %d\n", slot_number, offset, length);

  pci_write_buffer(adapter->unit, offset, buffer, length);

  return 0;
}

//
// NDIS I/O Port Functions
//

ndis_status ndisapi NdisMRegisterIoPortRange(
    void **port_offset,
    ndis_handle_t miniport_adapter_handle,
    unsigned int initial_port,
    unsigned int number_of_ports) {
  NDISTRACE("NdisMRegisterIoPortRange");
  kprintf("ndis: register io ports start=0x%x len=%d\n", initial_port, number_of_ports);
  *port_offset = (void *) initial_port;
  return 0;
}

void ndisapi NdisMDeregisterIoPortRange(
    ndis_handle_t miniport_adapter_handle,
    unsigned int initial_port,
    unsigned int number_of_ports,
    void *port_offset) {
  NDISTRACE("NdisMDeregisterIoPortRange");
}

//
// NDIS DMA-Related Functions
//

void ndisapi NdisMAllocateSharedMemory(
    ndis_handle_t miniport_adapter_handle,
    unsigned long length,
    boolean cached,
    void **virtual_address,
    ndis_physical_address_t *physical_address) {
  NDISTRACE("NdisMAllocateSharedMemory");
}

void ndisapi NdisMFreeSharedMemory(
    ndis_handle_t miniport_adapter_handle,
    unsigned long length,
    boolean cached,
    void *virtual_address,
    ndis_physical_address_t physical_address) {
  NDISTRACE("NdisMFreeSharedMemory");
}

void ndisapi NdisMAllocateMapRegisters(
    ndis_handle_t miniport_adapter_handle,
    unsigned int dma_channel,
    ndis_dma_size_t dma_size,
    unsigned long base_map_registers_needed,
    unsigned long maximum_physical_mapping) {
  NDISTRACE("NdisMAllocateMapRegisters");
}

void ndisapi NdisMFreeMapRegisters(ndis_handle_t miniport_adapter_handle) {
  NDISTRACE("NdisMFreeMapRegisters");
}

//
// NDIS Interrupt Handling Functions
//

ndis_status ndisapi NdisMRegisterInterrupt(
    struct ndis_miniport_interrupt *interrupt,
    ndis_handle_t miniport_adapter_handle,
    unsigned int interrupt_vector,
    unsigned int interrupt_level,
    boolean request_isr,
    boolean shared_interrupt,
    enum ndis_interrupt_mode interrupt_mode) {
  NDISTRACE("NdisMRegisterInterrupt");
  return 0;
}

void ndisapi NdisMDeregisterInterrupt(struct ndis_miniport_interrupt *interrupt) {
  NDISTRACE("NdisMDeregisterInterrupt");
}

boolean ndisapi NdisMSynchronizeWithInterrupt(
    struct ndis_miniport_interrupt *interrupt,
    void *synchronize_function,
    void *synchronize_context) {
  NDISTRACE("NdisMSynchronizeWithInterrupt");
  return 0;
}

//
// NDIS Synchronization Functions
//

void ndisapi NdisMInitializeTimer(struct ndis_miniport_timer *timer, ndis_handle_t miniport_adapter_handle, ndis_timer_func_t  timer_function, void *function_context) {
  NDISTRACE("NdisMInitializeTimer");
}

void ndisapi NdisMCancelTimer(struct ndis_miniport_timer *timer, boolean *timer_cancelled) {
  NDISTRACE("NdisMCancelTimer");
}

void ndisapi NdisMSetPeriodicTimer(struct ndis_miniport_timer *timer, unsigned int millisecond_period) {
  NDISTRACE("NdisMSetPeriodicTimer");
}

void ndisapi NdisAllocateSpinLock(struct ndis_spin_lock *spin_lock) {
  NDISTRACE("NdisAllocateSpinLock");
}

void ndisapi NdisFreeSpinLock(struct ndis_spin_lock *spin_lock) {
  NDISTRACE("NdisFreeSpinLock");
}

void ndisapi NdisAcquireSpinLock(struct ndis_spin_lock *spin_lock) {
  NDISTRACE("NdisAcquireSpinLock");
}

void ndisapi NdisReleaseSpinLock(struct ndis_spin_lock *spin_lock) {
  NDISTRACE("NdisReleaseSpinLock");
}

//
// NDIS Query and Set Completion Functions
//

void __stdcall NdisMQueryInformationComplete(
    ndis_handle_t miniport_adapter_handle,
    ndis_status status) {
  NDISTRACE("NdisMQueryInformationComplete");
}

void __stdcall NdisMSetInformationComplete(
    ndis_handle_t miniport_adapter_handle,
    ndis_status status) {
  NDISTRACE("NdisMSetInformationComplete");
}

//
// NDIS Status and Reset Indication Functions
//

void __stdcall NdisMIndicateStatus(
    ndis_handle_t miniport_handle,
    ndis_status general_status,
    void *status_buffer,
    unsigned int status_buffer_size) {
  NDISTRACE("NdisMIndicateStatus");
}


void __stdcall NdisMIndicateStatusComplete(ndis_handle_t miniport_adapter_handle) {
  NDISTRACE("NdisMIndicateStatusComplete");
}

void __stdcall NdisMResetComplete(ndis_handle_t miniport_adapter_handle, ndis_status status, boolean addressing_reset) {
  NDISTRACE("NdisMResetComplete");
}

//
// NDIS Send and Receive Functions for Connectionless Miniport Drivers
//

void __stdcall NdisMIndicateReceivePacket(
    ndis_handle_t miniport,
    struct ndis_packet **packet_array,
    unsigned int number_of_packets) {
  NDISTRACE("NdisMIndicateReceivePacket");
}

void __stdcall NdisMEthIndicateReceive(
    void *filter,
    ndis_handle_t mac_receive_context,
    char *address,
    void *header_buffer,
    unsigned int header_buffer_size,
    void *lookahead_buffer,
    unsigned int lookahead_buffer_size,
    unsigned int packet_size) {
  NDISTRACE("NdisMEthIndicateReceive");
}

void __stdcall NdisMTrIndicateReceive(
    void *filter,
    ndis_handle_t mac_receive_context,
    void *header_buffer,
    unsigned int header_buffer_size,
    void *lookahead_buffer,
    unsigned int lookahead_buffer_size,
    unsigned int packet_size) {
  NDISTRACE("NdisMTrIndicateReceive");
}

void __stdcall NdisMFddiIndicateReceive(
    void *filter,
    ndis_handle_t mac_receive_context,
    char *address,
    unsigned int address_length,
    void *header_buffer,
    unsigned int header_buffer_size,
    void *lookahead_buffer,
    unsigned int lookahead_buffer_size,
    unsigned int packet_size) {
  NDISTRACE("NdisMFddiIndicateReceive");
}

void __stdcall NdisMEthIndicateReceiveComplete(void *filter) {
  NDISTRACE("NdisMEthIndicateReceiveComplete");
}

void __stdcall NdisMTrIndicateReceiveComplete(void *filter) {
  NDISTRACE("NdisMTrIndicateReceiveComplete");
}

void __stdcall NdisMFddiIndicateReceiveComplete(void *filter) {
  NDISTRACE("NdisMFddiIndicateReceiveComplete");
}

void __stdcall NdisMSendComplete(ndis_handle_t miniport_adapter_handle, struct ndis_packet *packet, ndis_status status) {
  NDISTRACE("NdisMSendComplete");
}

void __stdcall NdisMSendResourcesAvailable(ndis_handle_t miniport_adapter_handle) {
  NDISTRACE("NdisMSendResourcesAvailable");
}

void __stdcall NdisMTransferDataComplete(
    ndis_handle_t miniport_adapter_handle,
    struct ndis_packet *packet,
    ndis_status status,
    unsigned int bytes_transferred) {
  NDISTRACE("NdisMTransferDataComplete");
}

//
// NDIS Send and Receive Functions for WAN Miniport Drivers
//

void __stdcall NdisMWanSendComplete(
    ndis_handle_t miniport_adapter_handle,
   void *packet,
    ndis_status status) {
  NDISTRACE("NdisMWanSendComplete");
}

void __stdcall NdisMWanIndicateReceive(
    ndis_status *status,
    ndis_handle_t miniport_adapter_handle,
    ndis_handle_t ndis_link_context,
    unsigned char *packet,
    unsigned long packet_size) {
  NDISTRACE("NdisMWanIndicateReceive");
}

void __stdcall NdisMWanIndicateReceiveComplete(
    ndis_handle_t miniport_adapter_handle,
    ndis_handle_t ndis_link_context) {
  NDISTRACE("NdisMWanIndicateReceiveComplete");
}

//
// NDIS Send and Receive Functions for Connection-Oriented Miniport Drivers
//

//
// NDIS Packet and Buffer Handling Functions
//

void ndisapi NdisAllocatePacketPool(
    ndis_status *status, 
    ndis_handle_t *pool_handle, 
    unsigned int number_of_descriptors, 
    unsigned int protocol_reserved_length) {
  NDISTRACE("NdisAllocatePacketPool");
}

void ndisapi NdisFreePacketPool(ndis_handle_t pool_handle) {
  NDISTRACE("NdisFreePacketPool");
}

void ndisapi NdisAllocatePacket(ndis_status *status, struct ndis_packet **packet, ndis_handle_t pool_handle) {
  NDISTRACE("NdisAllocatePacket");
}

void ndisapi NdisFreePacket(struct ndis_packet *packet) {
  NDISTRACE("NdisFreePacket");
}

void ndisapi NdisAllocateBufferPool(
    ndis_status *status, 
    ndis_handle_t *pool_handle, 
    unsigned int number_of_descriptors) {
  NDISTRACE("NdisAllocateBufferPool");
}

void ndisapi NdisFreeBufferPool(ndis_handle_t pool_handle) {
  NDISTRACE("NdisFreeBufferPool");
}

void ndisapi NdisAllocateBuffer(
    ndis_status *status,
    struct ndis_buffer **Buffer,
    ndis_handle_t pool_handle,
    void *virtual_address,
    unsigned int length) {
  NDISTRACE("NdisAllocateBuffer");
}

void ndisapi NdisFreeBuffer(struct ndis_buffer *buffer) {
  NDISTRACE("NdisFreeBuffer");
}

void ndisapi NdisQueryBuffer(struct ndis_buffer *buffer, void **virtual_address,  unsigned int *length) {
  NDISTRACE("NdisQueryBuffer");
}

void ndisapi NdisAdjustBufferLength(struct ndis_buffer *buffer, unsigned int length) {
  NDISTRACE("NdisAdjustBufferLength");
}

void ndisapi NdisQueryBufferOffset(struct ndis_buffer *buffer,  unsigned int *offset, unsigned int *length) {
  NDISTRACE("NdisQueryBufferOffset");
}

void ndisapi NdisUnchainBufferAtFront(struct ndis_packet *packet, struct ndis_buffer *buffer) {
  NDISTRACE("NdisUnchainBufferAtFront");
}

unsigned long ndisapi NDIS_BUFFER_TO_SPAN_PAGES(struct ndis_buffer *buffer) {
  NDISTRACE("NDIS_BUFFER_TO_SPAN_PAGES");
  return 0;
}

//
// NDIS Memory Support Functions
//

ndis_status ndisapi NdisAllocateMemoryWithTag(void **virtual_address, unsigned int length, unsigned long tag) {
  void *addr;

  NDISTRACE("NdisAllocateMemoryWithTag");
  
  addr = kmalloc(length);
  if (!addr) return NDIS_STATUS_FAILURE;

  *virtual_address = addr;  
  return 0;
}

void ndisapi NdisFreeMemory(void *virtual_address, unsigned int length, unsigned int memory_flags) {
  NDISTRACE("NdisFreeMemory");

  kfree(virtual_address);
}

//
// NDIS Logging Support Functions
//

void ndisapi NdisWriteErrorLogEntry(ndis_handle_t ndis_adapter_handle, unsigned long error_code, unsigned long errvals, ...) {
  NDISTRACE("NdisWriteErrorLogEntry");
  kprintf("ndis: error code 0x%x %d values\n", error_code, errvals);
}

//
// Setup callbacks
//

static void ndis_setup_callbacks(struct ndis_miniport_block *nmpb) {
  nmpb->packet_indicate_handler = NdisMIndicateReceivePacket;
  nmpb->send_complete_handler = NdisMSendComplete;
  nmpb->send_resources_handler = NdisMSendResourcesAvailable;
  nmpb->reset_complete_handler = NdisMResetComplete;

  nmpb->eth_rx_indicate_handler = NdisMEthIndicateReceive;
  nmpb->tr_rx_indicate_handler = NdisMTrIndicateReceive;
  nmpb->fddi_rx_indicate_handler = NdisMFddiIndicateReceive;

  nmpb->eth_rx_complete_handler = NdisMEthIndicateReceiveComplete;
  nmpb->tr_rx_complete_handler = NdisMTrIndicateReceiveComplete;
  nmpb->fddi_rx_complete_handler = NdisMFddiIndicateReceiveComplete;

  nmpb->status_handler = NdisMIndicateStatus;
  nmpb->status_complete_handler = NdisMIndicateStatusComplete;
  nmpb->td_complete_handler = NdisMTransferDataComplete;
  nmpb->query_complete_handler = NdisMQueryInformationComplete;
  nmpb->set_complete_handler = NdisMSetInformationComplete;

  nmpb->wan_send_complete_handler = NdisMWanSendComplete;
  nmpb->wan_rcv_handler = NdisMWanIndicateReceive;
  nmpb->wan_rcv_complete_handler = NdisMWanIndicateReceiveComplete;
}

//
// Module initialization
//

int __declspec(dllexport) install(struct unit *unit, char *opts) {
  char *modfn = opts;
  struct ndis_driver *driver;
  struct ndis_adapter *adapter;
  hmodule_t hmod;
  int rc;
  ndis_status open_status;
  ndis_status status;
  enum ndis_medium media;
  unsigned int selected_index;

  kprintf("ndis: loading driver %s for unit %08X\n", opts, unit->unitcode);

  // Try to find existing driver
  driver = NULL;
  hmod = getmodule(modfn);
  if (hmod) {
    driver = drivers;
    while (driver) {
      if (driver->hmod == hmod) break;
      driver = driver->next;
    }
  }

  // If driver not loaded, load it now and initialize
  if (!driver) {
    int (__stdcall *entry)(struct ndis_driver *driver, void *reserved);

    hmod = load(modfn, MODLOAD_NOINIT);
    if (!hmod) return -ENOEXEC;

    entry = getentrypoint(hmod);
    if (!entry) return -ENOEXEC;

    driver = kmalloc(sizeof(struct ndis_driver));
    if (!driver) return -ENOMEM;
    memset(driver, 0, sizeof(struct ndis_driver));

    rc = entry(driver, NULL);
    if (rc < 0) {
      kprintf("ndis: driver initialization failed with error code %08X\n", rc);
      kfree(driver);
      unload(hmod);
      return -ENXIO;
    }

    driver->hmod = hmod;
    driver->next = drivers;
    drivers = driver;
  }

  // Initialize new adapter
  adapter = kmalloc(sizeof(struct ndis_adapter));
  if (!adapter) return -ENOMEM;
  memset(adapter, 0, sizeof(struct ndis_adapter));
  ndis_setup_callbacks(&adapter->callbacks);
  adapter->unit = unit;
  adapter->adapterno = ++driver->numadapters;
  adapter->driver = driver;
  adapter->next = driver->adapters;
  driver->adapters = adapter;
  
  // Initialize adapter
  open_status = 0;
  selected_index = 0;
  media = NDIS_MEDIUM_802_3;
  status = adapter->driver->handlers.initialize_handler(&open_status, &selected_index, &media, 1, adapter, adapter);
  kprintf("ndis: initialize returned %08X\n", status);
  
  return 0;
}

int __stdcall start(hmodule_t hmod, int reason, void *reserved2) {
  kprintf("ndis: loaded\n");
  return 1;
}
