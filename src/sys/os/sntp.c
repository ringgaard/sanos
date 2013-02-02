//
// sntp.c
//
// Simple Network Time Protocol
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

#include <os.h>
#include <string.h>
#include <inifile.h>

#define NTP_EPOCH            (86400U * (365U * 70U + 17U))
#define NTP_PORT             123
#define MAX_NTP_SERVERS      16
#define NTP_REPLY_TIMEOUT    6000
#define TIME_ADJUST_INTERVAL (8 * 60 * 60 * 1000)
#define TIME_ADJUST_RETRY    (5 * 60 * 1000)

struct ntp_server {
  struct sockaddr_in sa;
  char *hostname;
};

struct ntp_packet {
  unsigned char mode_vn_li;
  unsigned char stratum;
  char poll;
  char precision;
  unsigned long root_delay;
  unsigned long root_dispersion;
  unsigned long reference_identifier;
  unsigned long reference_timestamp_secs;
  unsigned long reference_timestamp_fraq;
  unsigned long originate_timestamp_secs;
  unsigned long originate_timestamp_fraq;
  unsigned long receive_timestamp_seqs;
  unsigned long receive_timestamp_fraq;
  unsigned long transmit_timestamp_secs;
  unsigned long transmit_timestamp_fraq;
};

static struct ntp_server ntp_servers[MAX_NTP_SERVERS];
static int num_ntp_servers;

int sntp_get(struct ntp_server *srv, struct timeval *tv) {
  struct ntp_packet pkt;
  int s;
  int rc;
  int timeout;

  //syslog(LOG_AUX, "sntp_get: retrieving time from %a", &srv->sa.sin_addr);

  s = socket(AF_INET, SOCK_DGRAM, 0);
  if (s < 0) return s;

  rc = connect(s, (struct sockaddr *) &srv->sa, sizeof(struct sockaddr_in));
  if (rc < 0) {
    close(s);
    return rc;
  }

  memset(&pkt, 0, sizeof pkt);
  pkt.mode_vn_li = (4 << 3) | 3;
  pkt.originate_timestamp_secs = htonl(time(0) + NTP_EPOCH);

  rc = send(s, &pkt, sizeof pkt, 0);
  if (rc != sizeof pkt) {
    close(s);
    return rc;
  }

  timeout = NTP_REPLY_TIMEOUT;
  setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(int)); 

  rc = recvfrom(s, &pkt, sizeof pkt, 0, NULL, NULL);
  if (rc != sizeof pkt) {
    close(s);
    return rc;
  }

  tv->tv_sec = ntohl(pkt.transmit_timestamp_secs) - NTP_EPOCH;
  tv->tv_usec = ntohl(pkt.transmit_timestamp_fraq) / 4295;

  close(s);
  return 0;
}

void __stdcall sntpd(void *arg) {
  int i, j;
  struct ntp_server *srv;
  struct hostent *hp;
  int success;
  struct timeval tv;

  //syslog(LOG_AUX, "sntpd: started");
  while (1) {
    success = 0;
    for (i = 0; i < num_ntp_servers; i++) {
      srv = &ntp_servers[i];

      if (srv->hostname != NULL) {
        hp = gethostbyname(srv->hostname);
        if (hp) {
          for (j = 0; hp->h_addr_list[j] != NULL; j++) {
            struct in_addr *addr = (struct in_addr *) (hp->h_addr_list[j]);
            memcpy(&srv->sa.sin_addr, addr, hp->h_length);
            if (sntp_get(srv, &tv) >= 0) {
              success = 1;
              break;
            }
          }
        }
      } else {
        if (sntp_get(srv, &tv) >= 0) success = 1;
      }

      if (success) break;
    }

    if (success) {
      //struct timeval now;
      //gettimeofday(&now);
      settimeofday(&tv);
      //syslog(LOG_AUX, "sntpd: adjusting %d %d %a", tv.tv_sec - now.tv_sec, tv.tv_usec - now.tv_usec, &srv->sa.sin_addr);
      msleep(TIME_ADJUST_INTERVAL);
    } else {
      syslog(LOG_AUX, "sntpd: error obtaining time from time server");
      msleep(TIME_ADJUST_RETRY);
    }
  }
}

void init_sntpd() {
  int idx;
  struct section *sect;
  struct property *prop;
  struct peb *peb = getpeb();
  
  if (peb->ipaddr.s_addr == INADDR_ANY) return;

  idx = 0;
  if (peb->ntp_server1.s_addr != INADDR_ANY) {
    ntp_servers[idx].hostname = NULL;
    ntp_servers[idx].sa.sin_addr.s_addr = peb->ntp_server1.s_addr;
    ntp_servers[idx].sa.sin_family = AF_INET;
    ntp_servers[idx].sa.sin_port = htons(NTP_PORT);
    idx++;
  }

  if (peb->ntp_server2.s_addr != INADDR_ANY) {
    ntp_servers[idx].hostname = NULL;
    ntp_servers[idx].sa.sin_addr.s_addr = peb->ntp_server2.s_addr;
    ntp_servers[idx].sa.sin_family = AF_INET;
    ntp_servers[idx].sa.sin_port = htons(NTP_PORT);
    idx++;
  }

  sect = find_section(osconfig(), "ntp");
  if (sect) {
    prop = sect->properties;
    while (prop) {
      if (idx == MAX_NTP_SERVERS) break;

      ntp_servers[idx].hostname = prop->name;
      ntp_servers[idx].sa.sin_addr.s_addr = INADDR_ANY;
      ntp_servers[idx].sa.sin_family = AF_INET;
      ntp_servers[idx].sa.sin_port = htons(NTP_PORT);

      idx++;
      prop = prop->next;
    }
  }

  num_ntp_servers = idx;
  if (idx > 0) beginthread(sntpd, 0, NULL, 0, "sntpd", NULL);
}
