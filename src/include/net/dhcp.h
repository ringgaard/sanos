//
// dhcp.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Dynamic Host Configuration Protocol (DHCP)
//

#ifndef DHCP_H
#define DHCP_H

#define DHCP_CLIENT_PORT        68	
#define DHCP_SERVER_PORT        67

#define DHCP_CHADDR_LEN         16
#define DHCP_SNAME_LEN		64
#define DHCP_FILE_LEN		128
#define DHCP_OPTIONS_LEN        512
#define DHCP_MIN_OPTIONS_LEN    68

// Period (in seconds) of the application calling dhcp_tmrslow()

#define DHCP_SLOW_TIMER_SECS 60 

// Period (in milliseconds) of the application calling dhcp_tmrfast()

#define DHCP_FAST_TIMER_MSECS 500 

//
// DHCP message
//

struct dhcp_msg
{
  unsigned char op;			  // Message opcode/type
  unsigned char htype;			  // Hardware addr type (net/if_types.h)
  unsigned char hlen;			  // Hardware addr length
  unsigned char hops;			  // Number of relay agent hops from client
  unsigned long xid;			  // Transaction ID
  unsigned short secs;			  // Seconds since client started looking
  unsigned short flags;			  // Flag bits
  struct in_addr ciaddr;		  // Client IP address (if already in use)
  struct in_addr yiaddr;		  // Client IP address
  struct in_addr siaddr;		  // IP address of next server to talk to
  struct in_addr giaddr;		  // DHCP relay agent IP address
  unsigned char chaddr[DHCP_CHADDR_LEN];  // Client hardware address
  char sname[DHCP_SNAME_LEN];		  // Server name
  char file[DHCP_FILE_LEN];		  // Boot filename
  unsigned long cookie;                   // DHCP option cookie
  unsigned char options[0];   		  // 212: Optional parameters (actual length dependent on MTU).
};

//
// DHCP per interface state
//

struct dhcp_state
{
  struct dhcp_state *next;		  // For linked list purposes
  int state;				  // Current DHCP state (of DHCP state machine)
  int tries;				  // Retries of current request
  unsigned long xid;			  // Id of last sent request
  struct netif *netif;			  // Interface to be configured
  struct udp_pcb *pcb;			  // Our connection

  struct pbuf *p;			  // First pbuf of incoming msg
  struct dhcp_msg *msg_in;		  // Incoming msg
  struct dhcp_msg *options_in;		  // Incoming msg options
  int options_in_len;			  // Ingoing msg options length

  struct pbuf *p_out;			  // pbuf of outcoming msg
  struct dhcp_msg *msg_out;		  // Outgoing msg
  int options_out_len;			  // Outgoing msg options length

  int request_timeout;			  // #ticks with period DHCP_FAST_TIMER_SECS for request timeout
  int t1_timeout;			  // #ticks with period DHCP_SLOW_TIMER_SECS for renewal time
  int t2_timeout;			  // #ticks with period DHCP_SLOW_TIMER_SECS for rebind time

  struct ip_addr server_ip_addr;	  // DHCP server address that offered this lease 
  struct ip_addr offered_ip_addr;
  struct ip_addr offered_sn_mask;
  struct ip_addr offered_gw_addr;
  struct ip_addr offered_bc_addr;

  unsigned long offered_t0_lease;	  // Lease period (in seconds)
  unsigned long offered_t1_renew;	  // Recommended renew time (usually 50% of lease period)
  unsigned long offered_t2_rebind;	  // Recommended rebind time (usually 66% of lease period)
};

void dhcp_init();
struct dhcp_state *dhcp_start(struct netif *netif);
void dhcp_stop(struct dhcp_state *state);
err_t dhcp_renew(struct dhcp_state *state);
err_t dhcp_inform(struct netif *netif);

void dhcp_arp_reply(struct ip_addr *addr);

void dhcp_slowtmr();
void dhcp_fasttmr();
 
#define DHCP_COOKIE_OFS (DHCP_MSG_OFS + DHCP_MSG_LEN)
#define DHCP_OPTIONS_OFS (DHCP_MSG_OFS + DHCP_MSG_LEN + 4)

//
// DHCP client states
//

#define DHCP_REQUESTING   1
#define DHCP_INIT         2
#define DHCP_REBOOTING    3
#define DHCP_REBINDING    4
#define DHCP_RENEWING     5
#define DHCP_SELECTING    6
#define DHCP_INFORMING    7
#define DHCP_CHECKING     8
#define DHCP_PERMANENT    9
#define DHCP_BOUND        10
#define DHCP_BACKING_OFF  11
#define DHCP_OFF          12
 
//
// BOOTP (RFC951) message types
//

#define DHCP_BOOTREQUEST  1
#define DHCP_BOOTREPLY    2

//
// DHCP message types
//

#define DHCP_DISCOVER     1
#define DHCP_OFFER        2
#define DHCP_REQUEST      3
#define DHCP_DECLINE      4
#define DHCP_ACK          5
#define DHCP_NAK          6
#define DHCP_RELEASE      7
#define DHCP_INFORM       8

//
// Possible values for hardware type (htype) field
//

#define DHCP_HTYPE_ETHER	1               // Ethernet 10Mbps
#define DHCP_HTYPE_IEEE802	6               // IEEE 802.2 Token Ring
#define DHCP_HTYPE_FDDI		8		// FDDI

#define DHCP_BROADCAST_FLAG 15
#define DHCP_BROADCAST_MASK (1 << DHCP_FLAG_BROADCAST)

//
// BOOTP options
//

#define DHCP_OPTION_PAD			    0
#define DHCP_OPTION_SUBNET_MASK		    1 // RFC 2132 3.3
#define DHCP_OPTION_ROUTER		    3 
#define DHCP_OPTION_HOSTNAME		    12
#define DHCP_OPTION_IP_TTL		    23
#define DHCP_OPTION_MTU			    26
#define DHCP_OPTION_BROADCAST		    28
#define DHCP_OPTION_TCP_TTL		    37
#define DHCP_OPTION_END			    255

//
// DHCP options
//

#define DHCP_OPTION_REQUESTED_IP	    50 // RFC 2132 9.1, requested IP address
#define DHCP_OPTION_LEASE_TIME		    51 // RFC 2132 9.2, time in seconds, in 4 bytes 
#define DHCP_OPTION_OVERLOAD		    52 // RFC2132 9.3, use file and/or sname field for options

#define DHCP_OPTION_MESSAGE_TYPE	    53 // RFC 2132 9.6, important for DHCP
#define DHCP_OPTION_MESSAGE_TYPE_LEN 1

#define DHCP_OPTION_SERVER_ID		    54 // RFC 2131 9.7, server IP address
#define DHCP_OPTION_PARAMETER_REQUEST_LIST  55 // RFC 2131 9.8, requested option types

#define DHCP_OPTION_MAX_MSG_SIZE	    57 // RFC 2131 9.10, message size accepted >= 576
#define DHCP_OPTION_MAX_MSG_SIZE_LEN 2

#define DHCP_OPTION_T1			    58 // T1 renewal time
#define DHCP_OPTION_T2			    59 // T2 rebinding time
#define DHCP_OPTION_CLIENT_ID		    61
#define DHCP_OPTION_TFTP_SERVERNAME	    66
#define DHCP_OPTION_BOOTFILE		    67

//
// Possible combinations of overloading the file and sname fields with options
//

#define DHCP_OVERLOAD_NONE	  0
#define DHCP_OVERLOAD_FILE	  1
#define DHCP_OVERLOAD_SNAME	  2
#define DHCP_OVERLOAD_SNAME_FILE  3

#endif
