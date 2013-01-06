//
// ping.c
//
// Send ICMP echo request to network host
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
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <shlib.h>

#define ICMP_ECHO 8 
#define ICMP_ECHOREPLY 0 
 
#define ICMP_MIN 8              // Minimum 8 byte icmp packet (just header) 
 
// 
// IP header 
//

#pragma pack(push, 1)

struct iphdr {
  unsigned short h_len_and_vers; // Length of the header and IP version
  unsigned short total_len;      // Total length of the packet 
  unsigned short ident;          // Unique identifier 
  unsigned short frag_and_flags; // Flags 
  unsigned char  ttl;
  unsigned char proto;           // Protocol (TCP, UDP etc) 
  unsigned short checksum;       // IP checksum 
   
  unsigned int source_ip;        // Source IP address
  unsigned int dest_ip;          // Destination IP address
};

// 
// ICMP header 
//

struct icmphdr {
  unsigned char i_type; 
  unsigned char i_code; // type sub code
  unsigned short i_cksum; 
  unsigned short i_id; 
  unsigned short i_seq; 
  
  // This is not the std header, but we reserve space for time
  unsigned long timestamp; 
}; 
 
#pragma pack(pop)

struct pingstat {
  int tmin;
  int tmax;
  int tsum;
  int ntransmitted;
  int nreceived;
};

#define DEF_PACKET_SIZE 32 
#define MAX_PACKET 1024 
#define PKTSIZ (sizeof(struct icmphdr) + MAX_PACKET)

static void fill_icmp_data(char *icmp_data, int datasize);
static unsigned short checksum(unsigned short *buffer, int size); 
static void decode_resp(char *buf, int bytes, struct sockaddr_in *from, struct pingstat *stat);

shellcmd(ping) {
  int sockraw; 
  struct sockaddr_in dest, from; 
  struct hostent *hp; 
  int bread, datasize, rc; 
  int fromlen = sizeof(from); 
  int timeout = 1000; 
  char *dest_ip; 
  char icmp_data[PKTSIZ]; 
  char recvbuf[PKTSIZ];
  unsigned int addr = 0; 
  unsigned short seq_no = 0; 
  int numpackets;
  char *hostname;
  struct pingstat stat;

  if (argc < 2) {
    fprintf(stderr, "usage: ping HOST [SIZE]\n"); 
    return 0;
  }
  hostname = argv[1];

  memset(&dest, 0, sizeof(dest)); 
  hp = gethostbyname(hostname);
  if (!hp) addr = inet_addr(hostname); 
  
  if (!hp && addr == INADDR_NONE) {
    fprintf(stderr, "%s: unknown host %s\n", argv[0], hostname);
    return 1; 
  } 

  if (hp != NULL) {
    memcpy(&(dest.sin_addr), hp->h_addr, hp->h_length); 
  } else {
    dest.sin_addr.s_addr = addr; 
  }

  if (hp) {
    dest.sin_family = (unsigned char) hp->h_addrtype; 
  } else  {
    dest.sin_family = AF_INET; 
  }
 
  dest.sin_port = htons(53);
  dest_ip = inet_ntoa(dest.sin_addr); 
 
  if (argc > 2) { 
    datasize = atoi(argv[2]); 
    if (datasize == 0) datasize = DEF_PACKET_SIZE; 
  } else {
    datasize = DEF_PACKET_SIZE; 
  }

  if (datasize + sizeof(struct icmphdr) > PKTSIZ) { 
    fprintf(stderr, "%s: packet size too large\n", argv[0]); 
    return 1; 
  } 

  sockraw = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);   
  if (sockraw < 0) {
    perror("ping: socket");
    return 1; 
  } 

  rc = setsockopt(sockraw, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)); 
  if (rc < 0) {
    perror("ping: recv timeout");
    return 1; 
  } 

  timeout = 1000; 
  rc = setsockopt(sockraw, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeout, sizeof(timeout)); 
  if (rc < 0) { 
    perror("ping: send timeout");
    return 1; 
  }

  stat.tmin = 999999999;
  stat.tmax = 0;
  stat.tsum = 0;
  stat.ntransmitted = 0;
  stat.nreceived = 0;

  memset(icmp_data, 0, MAX_PACKET); 
  fill_icmp_data(icmp_data, datasize + sizeof(struct icmphdr));
 
  if (dest.sin_family == AF_INET) {
    printf("PING %s (%s): %d data bytes\n", hostname, inet_ntoa(dest.sin_addr), datasize);
  } else {
    printf("PING %s: %d data bytes\n", hostname, datasize);
  }
  printf("\n");

  for (numpackets = 0; numpackets < 5; numpackets++) { 
    int bwrote;
    struct icmphdr *icmphdr = (struct icmphdr *) &icmp_data;

    msleep(100);
    icmphdr->i_cksum = 0; 
    icmphdr->timestamp = clock(); 
    icmphdr->i_seq = seq_no++; 
    icmphdr->i_cksum = checksum((unsigned short *) icmp_data, datasize + sizeof(struct icmphdr));
 
    bwrote = sendto(sockraw, icmp_data, datasize + sizeof(struct icmphdr), 0, (struct sockaddr *) &dest, sizeof(dest)); 
    if (bwrote < 0) { 
      if (errno == ETIMEDOUT) {
        printf("timed out\n"); 
        continue; 
      } 
      perror("ping: sendto"); 
      return 1; 
    }
 
    if (bwrote < datasize) {
      fprintf(stdout, "Wrote %d bytes\n", bwrote); 
    }

    fflush(stdout);

    stat.ntransmitted++;

    bread = recvfrom(sockraw ,recvbuf, MAX_PACKET, 0, (struct sockaddr *) &from, &fromlen); 
    if (bread < 0) {
      if (errno == ETIMEDOUT) {
        printf("timed out\n"); 
        continue; 
      } 
      perror("ping: recvfrom"); 
      return 1; 
    }

    decode_resp(recvbuf, bread, &from, &stat); 
    msleep(1000); 
  } 

  printf("----%s PING Statistics----\n", hostname);
  printf("%d packets transmitted, ", stat.ntransmitted);
  printf("%d packets received, ", stat.nreceived);
  if (stat.ntransmitted) {
    if (stat.nreceived > stat.ntransmitted) {
      printf("-- somebody's printing up packets!");
    } else {
      printf("%d%% packet loss", (int) (((stat.ntransmitted - stat.nreceived) * 100) / stat.ntransmitted));
    }
    printf("\n");
  }

  if (stat.nreceived) {
    printf("round-trip (ms)  min/avg/max = %d/%d/%d\n", stat.tmin, stat.tsum / stat.nreceived, stat.tmax);
  }

  close(sockraw);
  return 0; 
} 

static void decode_resp(char *buf, int bytes, struct sockaddr_in *from, struct pingstat *stat) {
  struct iphdr *iphdr; 
  struct icmphdr *icmphdr; 
  unsigned short iphdrlen; 
  int triptime;

  iphdr = (struct iphdr *) buf;
  iphdrlen = (iphdr->h_len_and_vers & 0x0F) * 4;

  if (bytes < iphdrlen + ICMP_MIN) {
    printf("Too few bytes from %s\n", inet_ntoa(from->sin_addr)); 
    return;
  } 

  icmphdr = (struct icmphdr *) (buf + iphdrlen); 

  if (icmphdr->i_type != ICMP_ECHOREPLY && icmphdr->i_type != ICMP_ECHO) {
    fprintf(stderr, "non-echo type %d recvd\n", icmphdr->i_type); 
    return; 
  } 

  if (icmphdr->i_id != (unsigned short) gettid()) {
    fprintf(stderr, "someone else's packet!\n"); 
    return; 
  } 

  triptime = clock() - icmphdr->timestamp;
  stat->tsum += triptime;
  if (triptime < stat->tmin) stat->tmin = triptime;
  if (triptime > stat->tmax) stat->tmax = triptime;
  stat->nreceived++;

  bytes -= iphdrlen + sizeof(struct icmphdr);
  printf("%d bytes from %s:", bytes, inet_ntoa(from->sin_addr)); 
  printf(" icmp_seq=%d", icmphdr->i_seq); 
  printf(" time=%d ms", triptime); 
  printf(" TTL=%d", iphdr->ttl); 
  printf("\n"); 
} 
 
static unsigned short checksum(unsigned short *buffer, int size) {
  unsigned long cksum = 0; 
 
  while (size > 1) {
    cksum += *buffer++; 
    size -= sizeof(unsigned short); 
  } 
   
  if (size) cksum += *(unsigned char *) buffer; 
 
  cksum = (cksum >> 16) + (cksum & 0xffff); 
  cksum += (cksum >> 16); 
  return (unsigned short) (~cksum); 
}

static void fill_icmp_data(char *icmp_data, int datasize) {
  struct icmphdr *icmp_hdr; 
  char *datapart; 
 
  icmp_hdr = (struct icmphdr *) icmp_data; 
 
  icmp_hdr->i_type = ICMP_ECHO; 
  icmp_hdr->i_code = 0; 
  icmp_hdr->i_id = (unsigned short) gettid(); 
  icmp_hdr->i_cksum = 0; 
  icmp_hdr->i_seq = 0; 
   
  datapart = icmp_data + sizeof(struct icmphdr); 
  memset(datapart, 'E', datasize - sizeof(struct icmphdr));  
}
