//
// dhcp.c
//
// Dynamic Host Configuration Protocol (DHCP)
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
// Portions Copyright (C) 2001, 2002 Leon Woestenberg <leon.woestenberg@axon.nl>
// Portions Copyright (C) 2001, 2002 Axon Digital Design B.V., The Netherlands.
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

#include <net/net.h>

static struct dhcp_state *dhcp_client_list = NULL;
int dhcp_arp_check = 0;

//
// DHCP client state machine functions
//

static void dhcp_handle_ack(struct dhcp_state *state);
static void dhcp_handle_nak(struct dhcp_state *state);
static void dhcp_handle_offer(struct dhcp_state *state);
static err_t dhcp_discover(struct dhcp_state *state);
static err_t dhcp_select(struct dhcp_state *state);
static void dhcp_check(struct dhcp_state *state);
static err_t dhcp_decline(struct dhcp_state *state);
static void dhcp_bind(struct dhcp_state *state);
static err_t dhcp_rebind(struct dhcp_state *state);
static err_t dhcp_release(struct dhcp_state *state);
static void dhcp_set_state(struct dhcp_state *state, unsigned char new_state);

//
// Receive, unfold, process and free incoming messages
//

static err_t dhcp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, unsigned short port);
static err_t dhcp_unfold_reply(struct dhcp_state *state);
static unsigned char *dhcp_get_option_ptr(struct dhcp_state *state, unsigned char option_type);
static unsigned char dhcp_get_option_byte(unsigned char *ptr);
static unsigned short dhcp_get_option_short(unsigned char *ptr);
static unsigned long dhcp_get_option_long(unsigned char *ptr);
static void dhcp_free_reply(struct dhcp_state *state);
static void dhcp_timeout_handler(struct dhcp_state *state);
static void dhcp_t1_timeout(struct dhcp_state *state);
static void dhcp_t2_timeout(struct dhcp_state *state);
void dhcp_arp_reply(struct ip_addr *addr);

//
// Build outgoing messages
//

static err_t dhcp_create_request(struct dhcp_state *state);
static void dhcp_delete_request(struct dhcp_state *state);
static void dhcp_option(struct dhcp_state *state, unsigned char option_type, unsigned char option_len);
static void dhcp_option_byte(struct dhcp_state *state, unsigned char value);
static void dhcp_option_short(struct dhcp_state *state, unsigned short value);
static void dhcp_option_long(struct dhcp_state *state, unsigned long value);
static void dhcp_option_trailer(struct dhcp_state *state);

struct dhcp_state *dhcp_find_client(struct netif *netif);
static void dhcp_dump_options(struct dhcp_state *state);

//
// dhcpstat_proc
//

static int dhcpstat_proc(struct proc_file *pf, void *arg) {
  struct dhcp_state *state;
  static char *statename[] = {
    "<none>", "REQUESTING", "INIT", "REBOOTING", "REBINDING",
    "RENEWING", "SELECTING", "INFORMING", "CHECKING", "PERMANENT",
    "BOUND", "BACKING OFF", "OFF"
  };

  state = dhcp_client_list;
  while (state) {
    time_t lease_age = time(NULL) - state->bind_time;

    pprintf(pf, "DHCP configuration for %s:\n", state->netif->name);
    pprintf(pf, "  State .................. : %s\n", statename[state->state]);
    pprintf(pf, "  DHCP Server ............ : %a\n", &state->server_ip_addr);
    pprintf(pf, "  IP Adress .............. : %a\n", &state->offered_ip_addr);
    pprintf(pf, "  Subnet Mask ............ : %a\n", &state->offered_sn_mask);
    pprintf(pf, "  Gateway Adress ......... : %a\n", &state->offered_gw_addr);
    pprintf(pf, "  Broadcast Address ...... : %a\n", &state->offered_bc_addr);
    pprintf(pf, "  Primary DNS Server ..... : %a\n", &state->offered_dns1_addr);
    pprintf(pf, "  Secondary DNS Server ... : %a\n", &state->offered_dns2_addr);
    pprintf(pf, "  Primary NTP Server ..... : %a\n", &state->offered_ntpserv1_addr);
    pprintf(pf, "  Secondary NTP Server ... : %a\n", &state->offered_ntpserv2_addr);
    pprintf(pf, "  Domain Name ............ : %s\n", state->offered_domain_name);
    pprintf(pf, "  Lease Period ........... : %d seconds (%d left)\n", state->offered_t0_lease, state->offered_t0_lease - lease_age);
    pprintf(pf, "  Renew Period ........... : %d seconds (%d left)\n", state->offered_t1_renew, state->offered_t1_renew - lease_age);
    pprintf(pf, "  Rebind Period .......... : %d seconds (%d left)\n", state->offered_t2_rebind, state->offered_t2_rebind - lease_age);
    pprintf(pf, "  Lease Age .............. : %d seconds\n", lease_age);

    if (state->next) pprintf(pf, "\n");
    state = state->next;
  }

  return 0;
}

//
// dhcp_handle_nak
//

static void dhcp_handle_nak(struct dhcp_state *state) {
  int msecs = 10 * 1000;  
  mod_timer(&state->request_timeout_timer, ticks + msecs / MSECS_PER_TICK);
  kprintf(KERN_WARNING "dhcp_handle_nak: request timeout %u msecs\n", msecs);
  dhcp_set_state(state, DHCP_BACKING_OFF);
}

//
// dhcp_check
//
// Checks if the offered address is already in use.
// It does so by sending an ARP query.
//

static void dhcp_check(struct dhcp_state *state) {
  struct pbuf *p;
  err_t result;
  int msecs;

  p = arp_query(state->netif, (struct eth_addr *) &state->netif->hwaddr, &state->offered_ip_addr);
  if (p != NULL) {
    kprintf("dhcp_check: sending ARP request len %u\n", p->tot_len);
    result = dev_transmit((dev_t) state->netif->state, p);
    pbuf_free(p);
    //return result;
  }
  state->tries++;
  msecs = state->tries * 1000;
  mod_timer(&state->request_timeout_timer, ticks + msecs / MSECS_PER_TICK);

  dhcp_set_state(state, DHCP_CHECKING);
}

//
// dhcp_handle_offer
//

static void dhcp_handle_offer(struct dhcp_state *state)
{
  unsigned char *option_ptr = dhcp_get_option_ptr(state, DHCP_OPTION_DHCP_SERVER_IDENTIFIER);
  if (option_ptr != NULL) {
    state->server_ip_addr.addr = htonl(dhcp_get_option_long(&option_ptr[2]));
    //kprintf("dhcp_handle_offer: server 0x%08lx\n", state->server_ip_addr.addr);
    ip_addr_set(&state->offered_ip_addr, (struct ip_addr *) &state->msg_in->yiaddr);
    //kprintf("dhcp_handle_offer: offer for 0x%08lx\n", state->offered_ip_addr.addr);
    dhcp_select(state);
  }
}

//
// dhcp_select
//
// Select a DHCP server offer out of all offers.
// We simply select the first offer received.
//

static err_t dhcp_select(struct dhcp_state *state) {
  err_t result;
  int msecs;

  // Create and initialize the DHCP message header
  result = dhcp_create_request(state);
  if (result == 0) {
    dhcp_option(state, DHCP_OPTION_DHCP_MESSAGE_TYPE, 1);
    dhcp_option_byte(state, DHCP_REQUEST);

    dhcp_option(state, DHCP_OPTION_DHCP_MAX_MESSAGE_SIZE, 2);
    dhcp_option_short(state, 576);

    dhcp_option(state, DHCP_OPTION_DHCP_REQUESTED_ADDRESS, 4);
    dhcp_option_long(state, ntohl(state->offered_ip_addr.addr));

    dhcp_option(state, DHCP_OPTION_DHCP_SERVER_IDENTIFIER, 4);
    dhcp_option_long(state, ntohl(state->server_ip_addr.addr));

    dhcp_option_trailer(state);

    pbuf_realloc(state->p_out, sizeof(struct dhcp_msg) + state->options_out_len);

    udp_bind(state->pcb, IP_ADDR_ANY, DHCP_CLIENT_PORT);
    udp_connect(state->pcb, IP_ADDR_BROADCAST, DHCP_SERVER_PORT);
    if (udp_send(state->pcb, state->p_out, NULL, 0, state->netif) >= 0) state->p_out = NULL;

    // reconnect to any (or to server here?!)
    udp_connect(state->pcb, IP_ADDR_ANY, DHCP_SERVER_PORT);
    dhcp_delete_request(state);
  }
  
  state->tries++;
  msecs = state->tries < 4 ? state->tries * 1000 : 4 * 1000;
  mod_timer(&state->request_timeout_timer, ticks + msecs / MSECS_PER_TICK);

  dhcp_set_state(state, DHCP_REQUESTING);
  return result;
}

//
// dhcp_timeout_handler
//
// Something has timed out, handle this
//

static void dhcp_timeout_handler(struct dhcp_state *state) {
  if (state->state == DHCP_BACKING_OFF || state->state == DHCP_SELECTING) {
    kprintf(KERN_WARNING "dhcp_timeout: restarting discovery\n");
    dhcp_discover(state);
  } else if (state->state == DHCP_REQUESTING) {
    kprintf(KERN_WARNING "dhcp_timeout: REQUESTING, DHCP request timed out\n");
    if (state->tries <= 5) {
      dhcp_select(state);
    } else {
      struct netif *netif = state->netif;
      kprintf(KERN_WARNING "dhcp_timeout: REQUESTING, releasing, restarting\n");
      dhcp_release(state);
      dhcp_discover(state);
    }
  } else if (state->state == DHCP_CHECKING) {
    kprintf(KERN_WARNING "dhcp_timeout: CHECKING, ARP request timed out\n");
    if (state->tries <= 1) {
      dhcp_check(state);
    } else {
      // No ARP replies on the offered address, looks like the IP address is indeed free
      dhcp_bind(state);
    }
  } else if (state->state == DHCP_RENEWING) {
    kprintf(KERN_WARNING "dhcp_timeout: RENEWING, DHCP request timed out\n");
    dhcp_renew(state);
  } else if (state->state == DHCP_REBINDING) {
    kprintf(KERN_WARNING "dhcp_timeout: REBINDING, DHCP request timed out\n");
    if (state->tries <= 8) {
      dhcp_rebind(state);
    } else {
      struct netif *netif = state->netif;
      kprintf(KERN_WARNING "dhcp_timeout: REBINDING, release, restart\n");
      dhcp_release(state);
      dhcp_discover(state);
    }
  }
}

//
// dhcp_timeout
//

static void dhcp_timeout(void *arg) {
  struct dhcp_state *state = arg;

  queue_task(&sys_task_queue, &state->request_timeout_task, (taskproc_t) dhcp_timeout_handler, state);
}

//
// dhcp_t1_timeout
//

static void dhcp_t1_timeout(struct dhcp_state *state) {
  if (state->state == DHCP_REQUESTING || state->state == DHCP_BOUND || state->state == DHCP_RENEWING) {
    kprintf(KERN_WARNING "dhcp_t1_timeout: must renew\n");
    queue_task(&sys_task_queue, &state->t1_timeout_task, (taskproc_t) dhcp_renew, state);
  }
}

//
// dhcp_t2_timeout
//

static void dhcp_t2_timeout(struct dhcp_state *state) {
  if (state->state == DHCP_REQUESTING || state->state == DHCP_BOUND || state->state == DHCP_RENEWING) {
    kprintf(KERN_WARNING "dhcp_t2_timeout: must rebind\n");
    queue_task(&sys_task_queue, &state->t2_timeout_task, (taskproc_t) dhcp_rebind, state);
  }
}

//
// dhcp_handle_ack
//

static void dhcp_handle_ack(struct dhcp_state *state) {
  unsigned char *option_ptr;
  state->offered_sn_mask.addr = 0;
  state->offered_gw_addr.addr = 0;
  state->offered_bc_addr.addr = 0;
  state->offered_dns1_addr.addr = 0;
  state->offered_dns2_addr.addr = 0;
  state->bind_time = time(NULL);

  option_ptr = dhcp_get_option_ptr(state, DHCP_OPTION_DHCP_LEASE_TIME);
  if (option_ptr != NULL) {
    state->offered_t0_lease = dhcp_get_option_long(option_ptr + 2);
    state->offered_t1_renew = state->offered_t0_lease / 2;
    state->offered_t2_rebind = state->offered_t0_lease;
  }

  option_ptr = dhcp_get_option_ptr(state, DHCP_OPTION_DHCP_RENEWAL_TIME);
  if (option_ptr != NULL) {
    state->offered_t1_renew = dhcp_get_option_long(option_ptr + 2);
  }

  option_ptr = dhcp_get_option_ptr(state, DHCP_OPTION_DHCP_REBINDING_TIME);
  if (option_ptr != NULL) {
    state->offered_t2_rebind = dhcp_get_option_long(option_ptr + 2);
  }

  ip_addr_set(&state->offered_ip_addr, &state->msg_in->yiaddr);

  option_ptr = dhcp_get_option_ptr(state, DHCP_OPTION_SUBNET_MASK);
  if (option_ptr != NULL) {
    state->offered_sn_mask.addr = htonl(dhcp_get_option_long(option_ptr + 2));
  }

  option_ptr = dhcp_get_option_ptr(state, DHCP_OPTION_ROUTERS);
  if (option_ptr != NULL) {
    state->offered_gw_addr.addr = htonl(dhcp_get_option_long(option_ptr + 2));
  }

  option_ptr = dhcp_get_option_ptr(state, DHCP_OPTION_BROADCAST_ADDRESS);
  if (option_ptr != NULL) {
    state->offered_bc_addr.addr = htonl(dhcp_get_option_long(option_ptr + 2));
  }

  option_ptr = dhcp_get_option_ptr(state, DHCP_OPTION_DOMAIN_NAME_SERVERS);
  if (option_ptr != NULL) {
    if (option_ptr[1] >= 4) state->offered_dns1_addr.addr = htonl(dhcp_get_option_long(option_ptr + 2));
    if (option_ptr[1] >= 8) state->offered_dns2_addr.addr = htonl(dhcp_get_option_long(option_ptr + 6));
  }

  option_ptr = dhcp_get_option_ptr(state, DHCP_OPTION_NTP_SERVERS);
  if (option_ptr != NULL) {
    if (option_ptr[1] >= 4) state->offered_ntpserv1_addr.addr = htonl(dhcp_get_option_long(option_ptr + 2));
    if (option_ptr[1] >= 8) state->offered_ntpserv2_addr.addr = htonl(dhcp_get_option_long(option_ptr + 6));
  }

  option_ptr = dhcp_get_option_ptr(state, DHCP_OPTION_DOMAIN_NAME);
  if (option_ptr != NULL) {
    memcpy(state->offered_domain_name, option_ptr + 2, option_ptr[1]);
    state->offered_domain_name[option_ptr[1]] = 0;
  }
}

//
// dhcp_init
//

void dhcp_init() {
  register_proc_inode("dhcpstat", dhcpstat_proc, NULL);
}

//
// dhcp_start
//
// Start the DHCP negotiation
//

struct dhcp_state *dhcp_start(struct netif *netif) {
  struct dhcp_state *state = NULL;
  struct dhcp_state *list_state;
  err_t result;

  state = dhcp_find_client(netif);
  if (state != NULL) {
    kprintf("dhcp_start: already active on interface\n");
    
    // Just restart the DHCP negotiation
    result = dhcp_discover(state);
    if (result < 0) return NULL;
  }

  //kprintf("dhcp_start: starting new DHCP client\n");
  state = kmalloc(sizeof(struct dhcp_state));
  if (!state) return NULL;
  memset(state, 0, sizeof(struct dhcp_state));

  state->pcb = udp_new();
  if (!state->pcb) {
    kfree(state);
    return NULL;
  }

  state->pcb->flags |= UDP_FLAGS_BROADCAST;
  state->netif = netif;
  state->next = NULL;
  init_event(&state->binding_complete, 0, 0);

  init_task(&state->request_timeout_task);
  init_task(&state->t1_timeout_task);
  init_task(&state->t2_timeout_task);

  init_timer(&state->request_timeout_timer, (timerproc_t) dhcp_timeout, state);
  init_timer(&state->t1_timeout_timer, (timerproc_t) dhcp_t1_timeout, state);
  init_timer(&state->t2_timeout_timer, (timerproc_t) dhcp_t2_timeout, state);

  if (!dhcp_client_list) {
    dhcp_client_list = state;
  } else {
    list_state = dhcp_client_list;
    while (list_state->next != NULL) list_state = list_state->next;
    list_state->next = state;
  }
  
  dhcp_discover(state);
  return state;
}

//
// dhcp_inform
//

err_t dhcp_inform(struct netif *netif) {
  struct dhcp_state *state;
  err_t result;

  state = kmalloc(sizeof(struct dhcp_state));
  if (!state) return -ENOMEM;
  memset(state, 0, sizeof(struct dhcp_state));

  state->pcb = udp_new();
  if (!state->pcb) {
    kfree(state);
    return -ENOMEM;
  }

  state->pcb->flags |= UDP_FLAGS_BROADCAST;
  state->netif = netif;
  state->next = NULL;

  // Create and initialize the DHCP message header
  result = dhcp_create_request(state);
  if (!result) return -ENOMEM;

  dhcp_option(state, DHCP_OPTION_DHCP_MESSAGE_TYPE, 1);
  dhcp_option_byte(state, DHCP_INFORM);

  dhcp_option(state, DHCP_OPTION_DHCP_MAX_MESSAGE_SIZE, 2);
  dhcp_option_short(state, 576);

  dhcp_option_trailer(state);

  pbuf_realloc(state->p_out, sizeof(struct dhcp_msg) + state->options_out_len);

  udp_bind(state->pcb, IP_ADDR_ANY, DHCP_CLIENT_PORT);
  udp_connect(state->pcb, IP_ADDR_BROADCAST, DHCP_SERVER_PORT);

  if (udp_send(state->pcb, state->p_out, NULL, 0, state->netif) >= 0) state->p_out = NULL;
  udp_connect(state->pcb, IP_ADDR_ANY, DHCP_SERVER_PORT);
  dhcp_delete_request(state);

  udp_remove(state->pcb);
  kfree(state);

  return 0;
}

//
// dhcp_arp_reply
//

void dhcp_arp_reply(struct ip_addr *addr) {
  struct dhcp_state *state = dhcp_client_list;

  while (state != NULL) {
    // Is this DHCP client doing an ARP check?
    if (state->state == DHCP_CHECKING) {
      kprintf("dhcp_arp_reply: CHECKING, arp reply for 0x%08lx\n", addr->addr);

      // Does a host respond with the address we were offered by the DHCP server?
      if (ip_addr_cmp(addr, &state->offered_ip_addr)) {
        // We will not accept the offered address
        kprintf("dhcp_arp_reply: arp reply matched with offered address, declining\n");
        dhcp_decline(state);
      }
    }

    state = state->next;
  }
}

//
// dhcp_decline
//

static err_t dhcp_decline(struct dhcp_state *state) {
  err_t result;
  int msecs;

  // Create and initialize the DHCP message header
  result = dhcp_create_request(state);
  if (result == 0) {
    dhcp_option(state, DHCP_OPTION_DHCP_MESSAGE_TYPE, 1);
    dhcp_option_byte(state, DHCP_DECLINE);

    dhcp_option(state, DHCP_OPTION_DHCP_MAX_MESSAGE_SIZE, 2);
    dhcp_option_short(state, 576);

    dhcp_option_trailer(state);

    pbuf_realloc(state->p_out, sizeof(struct dhcp_msg) + state->options_out_len);

    udp_bind(state->pcb, IP_ADDR_ANY, DHCP_CLIENT_PORT);
    udp_connect(state->pcb, &state->server_ip_addr, DHCP_SERVER_PORT);
    if (udp_send(state->pcb, state->p_out, NULL, 0, state->netif) >= 0) state->p_out = NULL;
    dhcp_delete_request(state);
  }

  state->tries++;
  msecs = 10 * 1000;
  mod_timer(&state->request_timeout_timer, ticks + msecs / MSECS_PER_TICK);

  dhcp_set_state(state, DHCP_BACKING_OFF);
  return result;
}


//
// dhcp_discover
//
// Start the DHCP process, discover a server
//

static err_t dhcp_discover(struct dhcp_state *state) {
  err_t result;
  int msecs;

  ip_addr_set(&state->offered_ip_addr, IP_ADDR_ANY);

  // Create and initialize the DHCP message header
  result = dhcp_create_request(state);
  if (result == 0) {
    dhcp_option(state, DHCP_OPTION_DHCP_MESSAGE_TYPE, 1);
    dhcp_option_byte(state, DHCP_DISCOVER);

    dhcp_option(state, DHCP_OPTION_DHCP_MAX_MESSAGE_SIZE, 2);
    dhcp_option_short(state, 576);

#if 0
    dhcp_option(state, DHCP_OPTION_DHCP_PARAMETER_REQUEST_LIST, 6);
    dhcp_option_byte(state, DHCP_OPTION_SUBNET_MASK);
    dhcp_option_byte(state, DHCP_OPTION_ROUTERS);
    dhcp_option_byte(state, DHCP_OPTION_BROADCAST_ADDRESS);
    dhcp_option_byte(state, DHCP_OPTION_DOMAIN_NAME_SERVERS);
    dhcp_option_byte(state, DHCP_OPTION_DOMAIN_NAME);
    dhcp_option_byte(state, DHCP_OPTION_NTP_SERVERS);
#endif

    dhcp_option_trailer(state);

    pbuf_realloc(state->p_out, sizeof(struct dhcp_msg) + state->options_out_len);

    udp_recv(state->pcb, dhcp_recv, state);
    udp_bind(state->pcb, IP_ADDR_ANY, DHCP_CLIENT_PORT);
    udp_connect(state->pcb, IP_ADDR_BROADCAST, DHCP_SERVER_PORT);

    if (udp_send(state->pcb, state->p_out, NULL, 0, state->netif) >= 0) state->p_out = NULL;
    udp_bind(state->pcb, IP_ADDR_ANY, DHCP_CLIENT_PORT);
    udp_connect(state->pcb, IP_ADDR_ANY, DHCP_SERVER_PORT);

    dhcp_delete_request(state);
  }

  state->tries++;
  msecs = state->tries < 4 ? (state->tries + 1) * 1000 : 10 * 1000;
  mod_timer(&state->request_timeout_timer, ticks + msecs / MSECS_PER_TICK);

  dhcp_set_state(state, DHCP_SELECTING);
  return result;
}

//
// dhcp_bind
//
// Bind the interface to the offered IP address
//

static void dhcp_bind(struct dhcp_state *state) {
  struct ip_addr sn_mask, gw_addr;

  if (state->offered_t1_renew != 0xFFFFFFFF) {
    mod_timer(&state->t1_timeout_timer, ticks + state->offered_t1_renew * TICKS_PER_SEC);
  }

  if (state->offered_t2_rebind != 0xFFFFFFFF) {
    mod_timer(&state->t2_timeout_timer, ticks + state->offered_t2_rebind * TICKS_PER_SEC);
  }
  
  ip_addr_set(&sn_mask, &state->offered_sn_mask);
  if (sn_mask.addr == 0) {
    // Subnet mask not given
    // Choose a safe subnet mask given the network class

    unsigned char first_octet = ip4_addr1(&sn_mask);
    if (first_octet <= 127) { 
      sn_mask.addr = htonl(0xFF000000);
    } else if (first_octet >= 192) {
      sn_mask.addr = htonl(0xFFFFFF00);
    } else {
      sn_mask.addr = htonl(0xFFFF0000);
    }
  }

  ip_addr_set(&gw_addr, &state->offered_gw_addr);
  if (gw_addr.addr == 0) {
    // Gateway address not given
    gw_addr.addr = state->offered_ip_addr.addr;
    gw_addr.addr &= sn_mask.addr;
    gw_addr.addr |= 0x01000000;
  }

  netif_set_ipaddr(state->netif, &state->offered_ip_addr);
  netif_set_netmask(state->netif, &sn_mask);
  netif_set_gw(state->netif, &gw_addr);

  // Determine broadcast address
  state->netif->broadcast.addr = (state->netif->ipaddr.addr & state->netif->netmask.addr) | ~(state->netif->netmask.addr);

  //dhcp_dump_options(state);

  if (peb) {
    peb->primary_dns.s_addr = state->offered_dns1_addr.addr;
    peb->secondary_dns.s_addr = state->offered_dns2_addr.addr;
    memcpy(peb->default_domain, state->offered_domain_name, 256);
    peb->ntp_server1.s_addr = state->offered_ntpserv1_addr.addr;
    peb->ntp_server2.s_addr = state->offered_ntpserv2_addr.addr;
  }

  set_event(&state->binding_complete);
  dhcp_set_state(state, DHCP_BOUND);
}

//
// dhcp_renew
//

err_t dhcp_renew(struct dhcp_state *state) {
  err_t result;
  int msecs;

  // Create and initialize the DHCP message header
  result = dhcp_create_request(state);
  if (result == 0) {
    dhcp_option(state, DHCP_OPTION_DHCP_MESSAGE_TYPE, 1);
    dhcp_option_byte(state, DHCP_REQUEST);

    dhcp_option(state, DHCP_OPTION_DHCP_MAX_MESSAGE_SIZE, 2);
    dhcp_option_short(state, 576);

    dhcp_option(state, DHCP_OPTION_DHCP_REQUESTED_ADDRESS, 4);
    dhcp_option_long(state, ntohl(state->offered_ip_addr.addr));

    dhcp_option(state, DHCP_OPTION_DHCP_SERVER_IDENTIFIER, 4);
    dhcp_option_long(state, ntohl(state->server_ip_addr.addr));

    dhcp_option_trailer(state);

    pbuf_realloc(state->p_out, sizeof(struct dhcp_msg) + state->options_out_len);

    udp_bind(state->pcb, IP_ADDR_ANY, DHCP_CLIENT_PORT);
    udp_connect(state->pcb, &state->server_ip_addr, DHCP_SERVER_PORT);
    if (udp_send(state->pcb, state->p_out, NULL, 0, state->netif) >= 0) state->p_out = NULL;
    dhcp_delete_request(state);
  }
  state->tries++;
  msecs = state->tries < 10 ? state->tries * 2000 : 20 * 1000;
  mod_timer(&state->request_timeout_timer, ticks + msecs / MSECS_PER_TICK);

  dhcp_set_state(state, DHCP_RENEWING);
  return result;
}

//
// dhcp_rebind
//

static err_t dhcp_rebind(struct dhcp_state *state) {
  err_t result;
  int msecs;

  // create and initialize the DHCP message header
  result = dhcp_create_request(state);
  if (result == 0) {
    dhcp_option(state, DHCP_OPTION_DHCP_MESSAGE_TYPE, 1);
    dhcp_option_byte(state, DHCP_REQUEST);

    dhcp_option(state, DHCP_OPTION_DHCP_MAX_MESSAGE_SIZE, 2);
    dhcp_option_short(state, 576);

    dhcp_option(state, DHCP_OPTION_DHCP_REQUESTED_ADDRESS, 4);
    dhcp_option_long(state, ntohl(state->offered_ip_addr.addr));

    dhcp_option(state, DHCP_OPTION_DHCP_SERVER_IDENTIFIER, 4);
    dhcp_option_long(state, ntohl(state->server_ip_addr.addr));

    dhcp_option_trailer(state);

    pbuf_realloc(state->p_out, sizeof(struct dhcp_msg) - state->options_out_len);

    udp_bind(state->pcb, IP_ADDR_ANY, DHCP_CLIENT_PORT);
    udp_connect(state->pcb, IP_ADDR_BROADCAST, DHCP_SERVER_PORT);
    if (udp_send(state->pcb, state->p_out, NULL, 0, state->netif) >= 0) state->p_out = NULL;
    udp_connect(state->pcb, IP_ADDR_ANY, DHCP_SERVER_PORT);
    dhcp_delete_request(state);
  }

  state->tries++;
  msecs = state->tries < 10 ? state->tries * 1000 : 10 * 1000;
  mod_timer(&state->request_timeout_timer, ticks + msecs / MSECS_PER_TICK);

  dhcp_set_state(state, DHCP_REBINDING);
  return result;
}

//
// dhcp_release
//

static err_t dhcp_release(struct dhcp_state *state) {
  err_t result;
  int msecs;

  // Create and initialize the DHCP message header
  result = dhcp_create_request(state);
  if (result == 0) {
    dhcp_option(state, DHCP_OPTION_DHCP_MESSAGE_TYPE, 1);
    dhcp_option_byte(state, DHCP_RELEASE);

    dhcp_option_trailer(state);

    pbuf_realloc(state->p_out, sizeof(struct dhcp_msg) + state->options_out_len);

    udp_bind(state->pcb, IP_ADDR_ANY, DHCP_CLIENT_PORT);
    udp_connect(state->pcb, &state->server_ip_addr, DHCP_SERVER_PORT);
    if (udp_send(state->pcb, state->p_out, NULL, 0, state->netif) >= 0) state->p_out = NULL;
    dhcp_delete_request(state);
  }

  state->tries++;
  msecs = state->tries < 10 ? state->tries * 1000 : 10 * 1000;
  mod_timer(&state->request_timeout_timer, ticks + msecs / MSECS_PER_TICK);

  // Remove IP address from interface
  netif_set_ipaddr(state->netif, IP_ADDR_ANY);
  netif_set_gw(state->netif, IP_ADDR_ANY);
  netif_set_netmask(state->netif, IP_ADDR_ANY);
  
  // ... and idle DHCP client
  dhcp_set_state(state, DHCP_OFF);
  return result;
}

//
// dhcp_stop
//

void dhcp_stop(struct netif *netif) {
  struct dhcp_state *state;
  struct dhcp_state *list_state;

  state = dhcp_find_client(netif);
  if (state) {
    if (state->state == DHCP_BOUND) dhcp_release(state);

    del_timer(&state->request_timeout_timer);
    del_timer(&state->t1_timeout_timer);
    del_timer(&state->t2_timeout_timer);

    udp_remove(state->pcb);
    kfree(state);

    if (dhcp_client_list == state) {
      dhcp_client_list = state->next;
    } else {
      list_state = dhcp_client_list;
      while ((list_state != NULL) && (list_state->next != state)) list_state = list_state->next;
      if (list_state) list_state->next = state->next;
    }
  }
}

//
// dhcp_set_state
//

static void dhcp_set_state(struct dhcp_state *state, unsigned char new_state) {
  if (new_state != state->state) {
    state->state = new_state;
    state->tries = 0;
  }
}

//
// dhcp_option
//

static void dhcp_option(struct dhcp_state *state, unsigned char option_type, unsigned char option_len)
{
  state->msg_out->options[state->options_out_len++] = option_type;
  state->msg_out->options[state->options_out_len++] = option_len;
}

//
// dhcp_option_byte
//

static void dhcp_option_byte(struct dhcp_state *state, unsigned char value)
{
  state->msg_out->options[state->options_out_len++] = value;
}

//
// dhcp_option_short
//

static void dhcp_option_short(struct dhcp_state *state, unsigned short value)
{
  state->msg_out->options[state->options_out_len++] = (value & 0xFF00) >> 8;
  state->msg_out->options[state->options_out_len++] =  value & 0x00FF;
}

//
// dhcp_option_long
//

static void dhcp_option_long(struct dhcp_state *state, unsigned long value)
{
  state->msg_out->options[state->options_out_len++] = (unsigned char) ((value & 0xFF000000) >> 24);
  state->msg_out->options[state->options_out_len++] = (unsigned char) ((value & 0x00FF0000) >> 16);
  state->msg_out->options[state->options_out_len++] = (unsigned char) ((value & 0x0000FF00) >> 8);
  state->msg_out->options[state->options_out_len++] = (unsigned char) (value & 0x000000FF);
}

//
// dhcp_unfold_reply
//
// Extract the dhcp_msg and options each into linear pieces of memory
//

static err_t dhcp_unfold_reply(struct dhcp_state *state) {
  struct pbuf *p = state->p;
  unsigned char *ptr;
  int i;
  int j = 0;

  state->msg_in = NULL;
  state->options_in = NULL;

  // Options present?
  if (state->p->tot_len > sizeof(struct dhcp_msg)) {
    state->options_in_len = state->p->tot_len - sizeof(struct dhcp_msg);
    state->options_in = kmalloc(state->options_in_len);
    if (!state->options_in) return -ENOMEM;
  }

  state->msg_in = kmalloc(sizeof(struct dhcp_msg));
  if (!state->msg_in) {
    kfree(state->options_in);
    state->options_in = NULL;
    return -ENOMEM;
  }

  ptr = (unsigned char *) state->msg_in;

  // Proceed through struct dhcp_msg
  for (i = 0; i < sizeof(struct dhcp_msg); i++) {
    *ptr++ = ((unsigned char *) p->payload)[j++];
    
    if (j == p->len) {
      p = p->next;
      j = 0;
    }
  }

  if (state->options_in) {
    ptr = (unsigned char *) state->options_in;

    for (i = 0; i < state->options_in_len; i++) {
      *ptr++ = ((unsigned char *) p->payload)[j++];

      if (j == p->len) {
        p = p->next;
        j = 0;
      }
    }
  }

  return 0;
}

//
// dhcp_free_reply
//

static void dhcp_free_reply(struct dhcp_state *state) {
  kfree(state->msg_in);
  kfree(state->options_in);
  state->msg_in = NULL;
  state->options_in = NULL;
  state->options_in_len = 0;
}

//
// dhcp_recv
//

static err_t dhcp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, unsigned short port) {
  struct dhcp_state *state = (struct dhcp_state *) arg;
  struct dhcp_msg *reply_msg = (struct dhcp_msg *) p->payload;

  state->p = p;
  if (reply_msg->op == DHCP_BOOTREPLY) {
    if (memcmp(&state->netif->hwaddr, reply_msg->chaddr, ETHER_ADDR_LEN) == 0) {
      if (ntohl(reply_msg->xid) == state->xid) {
        unsigned char *options_ptr;

        del_timer(&state->request_timeout_timer);

        if (dhcp_unfold_reply(state) == 0) {
          options_ptr = dhcp_get_option_ptr(state, DHCP_OPTION_DHCP_MESSAGE_TYPE);
          if (options_ptr != NULL) {
            unsigned char msg_type = dhcp_get_option_byte(options_ptr + 2);

            if (msg_type == DHCP_ACK) {
              //kprintf("DHCP_ACK received\n");
              dhcp_handle_ack(state);
              if (state->state == DHCP_REQUESTING) {
                if (dhcp_arp_check) {
                  dhcp_check(state);
                } else {
                  dhcp_bind(state);
                }
              } else if (state->state == DHCP_REBOOTING || state->state == DHCP_REBINDING || state->state == DHCP_RENEWING) {
                dhcp_bind(state);
              }
            } else if ((msg_type == DHCP_NAK) &&
                       (state->state == DHCP_REBOOTING || state->state == DHCP_REQUESTING || 
                        state->state == DHCP_REBINDING || state->state == DHCP_RENEWING)) {
              // Received a DHCP_NAK in appropriate state
              kprintf("DHCP_NAK received\n");
              dhcp_handle_nak(state);
            } else if (msg_type == DHCP_OFFER && state->state == DHCP_SELECTING) {
              // Received a DHCP_OFFER in DHCP_SELECTING state
              //kprintf("DHCP_OFFER received in DHCP_SELECTING state\n");
              dhcp_handle_offer(state);
            }
          }

          dhcp_free_reply(state);
        }
      }
    }
  }

  pbuf_free(p);
  return 0;
}

//
// dhcp_create_request
//

static err_t dhcp_create_request(struct dhcp_state *state) {
  int i;

  state->p_out = pbuf_alloc(PBUF_TRANSPORT, sizeof(struct dhcp_msg) + DHCP_OPTIONS_LEN, PBUF_RW);
  if (!state->p_out) return -ENOMEM;
  state->msg_out = (struct dhcp_msg *) state->p_out->payload;
  memset(state->msg_out, 0, sizeof(struct dhcp_msg) + DHCP_OPTIONS_LEN);

  state->xid++;

  state->msg_out->op = DHCP_BOOTREQUEST;
  state->msg_out->htype = DHCP_HTYPE_ETHER;
  state->msg_out->hlen = ETHER_ADDR_LEN;
  state->msg_out->xid = htonl(state->xid);
  state->msg_out->ciaddr.s_addr = state->netif->ipaddr.addr;
  for (i = 0; i < ETHER_ADDR_LEN; i++) state->msg_out->chaddr[i] = state->netif->hwaddr.addr[i];
  state->msg_out->cookie = htonl(0x63825363);
  state->options_out_len = 0;

  return 0;
}

//
// dhcp_delete_request
//

static void dhcp_delete_request(struct dhcp_state *state) {
  pbuf_free(state->p_out);
  state->p_out = NULL;
  state->msg_out = NULL;
}

//
// dhcp_option_trailer
//
// Adds the END option to the DHCP message, and up to three padding bytes.
//

static void dhcp_option_trailer(struct dhcp_state *state) {
  state->msg_out->options[state->options_out_len++] = DHCP_OPTION_END;

  // Packet is still too small, or not 4 byte aligned?
  while ((state->options_out_len < DHCP_MIN_OPTIONS_LEN) || (state->options_out_len & 3)) {
    state->msg_out->options[state->options_out_len++] = 0;
  }
}

//
// dhcp_get_option_ptr
//
// Return a byte offset into the udp message where the option was found, or
// NULL if the given option was not found
//

static unsigned char *dhcp_get_option_ptr(struct dhcp_state *state, unsigned char option_type) {
  unsigned char overload = DHCP_OVERLOAD_NONE;

  // Options available?
  if (state->options_in != NULL && state->options_in_len > 0) {
    // Start with options field
    unsigned char *options = (unsigned char *) state->options_in;
    int offset = 0;
    
    // At least 1 byte to read and no end marker, then at least 3 bytes to read?
    while ((offset < state->options_in_len) && (options[offset] != DHCP_OPTION_END)) {
      // Are the sname and/or file field overloaded with options?
      if (options[offset] == DHCP_OPTION_DHCP_OPTION_OVERLOAD) {
        // Skip option type and length
        offset += 2;
        overload = options[offset++];
      } else if (options[offset] == option_type) {
        return &options[offset];
      } else if (options[offset] == DHCP_OPTION_PAD) {
        offset++;
      } else {
        //kprintf("opt(%d,%d)", options[offset], options[offset + 1]);
        offset++;
        offset += 1 + options[offset];
      }
    }

    // Is this an overloaded message?
    if (overload != DHCP_OVERLOAD_NONE) {
      int field_len;
      if (overload == DHCP_OVERLOAD_FILE) {
        options = (unsigned char *) &state->msg_in->file;
        field_len = DHCP_FILE_LEN;
      } else if (overload == DHCP_OVERLOAD_SNAME) {
        options = (unsigned char *) &state->msg_in->sname;
        field_len = DHCP_SNAME_LEN;
      } else {
        options = (unsigned char *) &state->msg_in->sname;
        field_len = DHCP_FILE_LEN + DHCP_SNAME_LEN;
      }
      offset = 0;

      // At least 1 byte to read and no end marker
      while ((offset < field_len) && (options[offset] != DHCP_OPTION_END)) {
        if (options[offset] == option_type) {
          return &options[offset];
        } else if (options[offset] == DHCP_OPTION_PAD) {
          offset++;
        } else {
         //kprintf("opt(%d,%d)", options[offset], options[offset + 1]);
          offset++;
          offset += 1 + options[offset];
        }
      }
    }
  }

  return NULL;
}

//
// dhcp_get_option_byte
//

static unsigned char dhcp_get_option_byte(unsigned char *ptr) {
  return *ptr;
}                            

//
// dhcp_get_option_short
//

static unsigned short dhcp_get_option_short(unsigned char *ptr) {
  unsigned short value;

  value = *ptr++ << 8;
  value |= *ptr;
  
  return value;
}                            

//
// dhcp_get_option_long
//

static unsigned long dhcp_get_option_long(unsigned char *ptr) {
  unsigned long value;

  value = (unsigned long) (*ptr++) << 24;
  value |= (unsigned long) (*ptr++) << 16;
  value |= (unsigned long) (*ptr++) << 8;
  value |= (unsigned long) (*ptr++);

  return value;
}                            

//
// dhcp_find_client
//
// Given an network interface, return the corresponding dhcp state
// or NULL if the interface was not under DHCP control.
//

struct dhcp_state *dhcp_find_client(struct netif *netif) {
  struct dhcp_state *state = dhcp_client_list;

  while (state) {
    if (state->netif == netif) return state;
    state = state->next;
  }
  return NULL;
}

//
// dhcp_dump_options
//

static void dhcp_dump_options(struct dhcp_state *state) {
  unsigned char overload = DHCP_OVERLOAD_NONE;

  kprintf("dhcp options:\n");
  // Options available?
  if (state->options_in != NULL && state->options_in_len > 0) {
    // Start with options field
    unsigned char *options = (unsigned char *) state->options_in;
    int offset = 0;
    
    // At least 1 byte to read and no end marker, then at least 3 bytes to read?
    while ((offset < state->options_in_len) && (options[offset] != DHCP_OPTION_END)) {
      // Are the sname and/or file field overloaded with options?
      if (options[offset] == DHCP_OPTION_DHCP_OPTION_OVERLOAD) {
        // Skip option type and length
        offset += 2;
        overload = options[offset++];
      } else if (options[offset] == DHCP_OPTION_PAD) {
        offset++;
      } else {
        kprintf(" option %d len %d\n", options[offset], options[offset + 1]);
        offset++;
        offset += 1 + options[offset];
      }
    }

    // Is this an overloaded message?
    if (overload != DHCP_OVERLOAD_NONE) {
      int field_len;
      if (overload == DHCP_OVERLOAD_FILE) {
        options = (unsigned char *) &state->msg_in->file;
        field_len = DHCP_FILE_LEN;
      } else if (overload == DHCP_OVERLOAD_SNAME) {
        options = (unsigned char *) &state->msg_in->sname;
        field_len = DHCP_SNAME_LEN;
      } else {
        options = (unsigned char *) &state->msg_in->sname;
        field_len = DHCP_FILE_LEN + DHCP_SNAME_LEN;
      }
      offset = 0;

      // At least 1 byte to read and no end marker
      while ((offset < field_len) && (options[offset] != DHCP_OPTION_END)) {
        if (options[offset] == DHCP_OPTION_PAD) {
          offset++;
        } else {
          kprintf(" option %d len %d\n", options[offset], options[offset + 1]);
          offset++;
          offset += 1 + options[offset];
        }
      }
    }
  }
}
