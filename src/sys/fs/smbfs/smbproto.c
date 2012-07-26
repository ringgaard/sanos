//
// smbproto.c
//
// SMB protocol
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
#include "smb.h"

struct smb_server *servers = NULL;

struct smb *smb_init(struct smb_share *share, int aux) {
  struct smb *smb;
  
  if (aux) {
    smb = (struct smb *) share->server->auxbuf;
  } else {
    smb = (struct smb *) share->server->buffer;
  }

  memset(smb, 0, sizeof(struct smb));
  return smb;
}

int smb_send(struct smb_share *share, struct smb *smb, unsigned char cmd, int params, char *data, int datasize) {
  int len;
  int rc;
  char *p;

  len = SMB_HEADER_LEN + params * 2 + 2 + datasize;

  smb->len[0] = (len & 0xFF000000) >> 24;
  smb->len[1] = (len & 0xFF0000) >> 16;
  smb->len[2] = (len & 0xFF00) >> 8;
  smb->len[3] = (len & 0xFF);
    
  smb->protocol[0] = 0xFF;
  smb->protocol[1] = 'S';
  smb->protocol[2] = 'M';
  smb->protocol[3] = 'B';

  smb->cmd = cmd;
  smb->tid = share->tid;
  smb->uid = share->server->uid;
  smb->wordcount = (unsigned char) params;
  smb->flags = (1 << 3);
  smb->flags2 = 1;

  p = (char *) smb->params.words + params * 2;
  *((unsigned short *) p) = (unsigned short) datasize;
  if (datasize) memcpy(p + 2, data, datasize);

  rc = send(share->server->sock, (char *) smb, len + 4, 0);
  if (rc < 0) return rc;
  if (rc != len + 4) return -EIO;

  return 0;
}

int smb_recv(struct smb_share *share, struct smb *smb) {
  int len;
  int rc;

  rc = recv_fully(share->server->sock, (char *) smb, 4, 0);
  if (rc < 0) return rc;
  if (rc != 4) return -EIO;

  len = smb->len[3] | (smb->len[2] << 8) | (smb->len[1] << 16) | (smb->len[0] << 24);
  if (len < 4 || len > SMB_MAX_BUFFER) return -EMSGSIZE;

  rc = recv_fully(share->server->sock, (char *) &smb->protocol, len, 0);
  if (rc < 0) return rc;
  if (rc != len) return -EIO;
  if (smb->protocol[0] != 0xFF || smb->protocol[1] != 'S' || smb->protocol[2] != 'M' || smb->protocol[3] != 'B') return -EPROTO;
  if (smb->error_class != SMB_SUCCESS) return smb_errno(smb);

  return 0;
}

int smb_request(struct smb_share *share, struct smb *smb, unsigned char cmd, int params, char *data, int datasize, int retry) {
  int rc;

  if (retry) {
    rc = smb_check_connection(share);
    if (rc < 0) return rc;
  }

  rc = smb_send(share, smb, cmd, params, data, datasize);
  if ((rc == -ECONN || rc == -ERST) && retry) {
    rc = smb_reconnect(share);
    if (rc < 0) return rc;

    rc = smb_send(share, smb, cmd, params, data, datasize);
    if (rc < 0) return rc;
  } else {
    if (rc < 0) return rc;
  }

  rc = smb_recv(share, smb);
  if (rc < 0) return rc;

  return 0;
}

int smb_trans_send(struct smb_share *share, unsigned short cmd, 
                   void *params, int paramlen,
                   void *data, int datalen,
                   int maxparamlen, int maxdatalen) {
  struct smb *smb;
  int wordcount = 15;
  int paramofs = ROUNDUP(SMB_HEADER_LEN + 2 * wordcount + 2 + 3);
  int dataofs = ROUNDUP(paramofs + paramlen);
  int bcc = dataofs + datalen - (SMB_HEADER_LEN + 2 * wordcount + 2);
  int len = SMB_HEADER_LEN + 2 * wordcount + 2 + bcc;
  char *p;
  int rc;

  rc = smb_check_connection(share);
  if (rc < 0) return rc;

  smb = (struct smb *) share->server->buffer;
  memset(smb, 0, sizeof(struct smb));

  smb->len[0] = (len > 0xFF000000) >> 24;
  smb->len[1] = (len > 0xFF0000) >> 16;
  smb->len[2] = (len & 0xFF00) >> 8;
  smb->len[3] = (len & 0xFF);
    
  smb->protocol[0] = 0xFF;
  smb->protocol[1] = 'S';
  smb->protocol[2] = 'M';
  smb->protocol[3] = 'B';

  smb->cmd = SMB_COM_TRANSACTION2;
  smb->tid = share->tid;
  smb->uid = share->server->uid;
  smb->wordcount = wordcount;
  smb->flags = (1 << 3);
  smb->flags2 = 1;

  smb->params.req.trans.total_parameter_count = paramlen;
  smb->params.req.trans.total_data_count = datalen;
  smb->params.req.trans.max_parameter_count = maxparamlen;
  smb->params.req.trans.max_data_count = maxdatalen;
  smb->params.req.trans.max_setup_count = 0;
  smb->params.req.trans.parameter_count = paramlen;
  smb->params.req.trans.parameter_offset = paramofs; 
  smb->params.req.trans.data_count = datalen;
  smb->params.req.trans.data_offset = dataofs;
  smb->params.req.trans.setup_count = 1;
  smb->params.req.trans.setup[0] = cmd;

  p = (char *) smb->params.words + wordcount * 2;
  *((unsigned short *) p) = (unsigned short) bcc;

  p = (char *) smb + 4;

  if (params) memcpy(p + paramofs, params, paramlen);
  if (data) memcpy(p + dataofs, data, datalen);

  rc = send(share->server->sock, (char *) smb, len + 4, 0);
  if (rc == -ECONN || rc == -ERST) {
    rc = smb_reconnect(share);
    if (rc < 0) return rc;

    rc = send(share->server->sock, (char *) smb, len + 4, 0);
    if (rc < 0) return rc;
    if (rc != len + 4) return -EIO;
  } else {
    if (rc < 0) return rc;
    if (rc != len + 4) return -EIO;
  }

  return 0;
}

int smb_trans_recv(struct smb_share *share,
                   void *params, int *paramlen,
                   void *data, int *datalen) {
  struct smb *smb;
  int paramofs, paramdisp, paramcnt;
  int dataofs, datadisp, datacnt;
  int rc;

  while (1) {
    // Receive next block
    smb = (struct smb *) share->server->buffer;
    rc = smb_recv(share, smb);
    if (rc < 0) return rc;

    // Copy parameters
    paramofs = smb->params.rsp.trans.parameter_offset;
    paramdisp = smb->params.rsp.trans.parameter_displacement;
    paramcnt = smb->params.rsp.trans.parameter_count;
    if (params) {
      if (paramdisp + paramcnt > *paramlen) return -EBUF;
      if (paramcnt) memcpy((char *) params + paramdisp, (char *) smb + paramofs + 4, paramcnt);
    }

    // Copy data
    dataofs = smb->params.rsp.trans.data_offset;
    datadisp = smb->params.rsp.trans.data_displacement;
    datacnt = smb->params.rsp.trans.data_count;
    if (data) {
      if (datadisp + datacnt > *datalen) return -EBUF;
      if (datacnt) memcpy((char *) data + datadisp, (char *) smb + dataofs + 4, datacnt);
    }

    // Check for last block
    if (paramdisp + paramcnt == smb->params.rsp.trans.total_parameter_count &&
        datadisp + datacnt == smb->params.rsp.trans.total_data_count) {
      *paramlen = smb->params.rsp.trans.total_parameter_count;
      *datalen = smb->params.rsp.trans.total_data_count;
      return 0;
    }
  }
}

int smb_trans(struct smb_share *share,
              unsigned short cmd, 
              void *reqparams, int reqparamlen,
              void *reqdata, int reqdatalen,
              void *rspparams, int *rspparamlen,
              void *rspdata, int *rspdatalen) {
  int rc;
  int dummyparamlen;
  int dummydatalen;

  if (!rspparamlen) {
    dummyparamlen = 0;
    rspparamlen = &dummyparamlen;
  }

  if (!rspdatalen) {
    dummydatalen = 0;
    rspdatalen = &dummydatalen;
  }

  rc = smb_trans_send(share, cmd, reqparams, reqparamlen, reqdata, reqdatalen, *rspparamlen, *rspdatalen);
  if (rc < 0) return rc;

  rc = smb_trans_recv(share, rspparams, rspparamlen, rspdata, rspdatalen);
  if (rc == -ERST) {
    rc = smb_reconnect(share);
    if (rc < 0) return rc;

    rc = smb_trans_send(share, cmd, reqparams, reqparamlen, reqdata, reqdatalen, *rspparamlen, *rspdatalen);
    if (rc < 0) return rc;

    rc = smb_trans_recv(share, rspparams, rspparamlen, rspdata, rspdatalen);
  }

  if (rc < 0) return rc;

  return 0;
}

int smb_connect_tree(struct smb_share *share) {
  struct smb *smb;
  int rc;
  char *p;
  char buf[SMB_NAMELEN];

  // Connect to share
  smb = smb_init(share, 1);
  smb->params.req.connect.andx.cmd = 0xFF;
  smb->params.req.connect.password_length = strlen(share->server->password);

  p = buf;
  p = addstr(p, share->server->password);
  p = addpathz(p, share->sharename);
  p = addstrz(p, SMB_SERVICE_DISK);

  rc = smb_request(share, smb, SMB_COM_TREE_CONNECT_ANDX, 4, buf, p - buf, 0);
  if (rc < 0) return rc;

  share->tid = smb->tid;
  share->mounttime = time(0);

  return 0;
}

int smb_disconnect_tree(struct smb_share *share) {
  struct smb *smb;

  // Disconnect from share
  smb = smb_init(share, 1);
  smb_request(share, smb, SMB_COM_TREE_DISCONNECT, 0, NULL, 0, 0);
  share->tid = 0xFFFF;

  return 0;
}

int smb_connect(struct smb_share *share) {
  struct smb_server *server = share->server;
  struct smb *smb;
  struct sockaddr_in sin;
  int rc;
  unsigned short max_mpx_count;
  char *p;
  char buf[SMB_NAMELEN];

  // Connect to SMB server
  rc = socket(AF_INET, SOCK_STREAM, IPPROTO_IP, &server->sock);
  if (rc < 0) return rc;

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = server->ipaddr.addr;
  sin.sin_port = htons(server->port);
  
  rc = connect(server->sock, (struct sockaddr *) &sin, sizeof(sin));
  if (rc < 0) goto error;

  // Negotiate protocol version
  smb = smb_init(share, 1);
  rc = smb_request(share, smb, SMB_COM_NEGOTIATE, 0, "\002NT LM 0.12", 12, 0); 
  if (rc < 0) goto error;

  if (smb->params.rsp.negotiate.dialect_index == 0xFFFF) {
    rc = -EREMOTEIO;
    goto error;
  }

  server->tzofs = smb->params.rsp.negotiate.server_timezone * 60;
  server->server_caps = smb->params.rsp.negotiate.capabilities;
  server->max_buffer_size = smb->params.rsp.negotiate.max_buffer_size;
  max_mpx_count = smb->params.rsp.negotiate.max_mpx_count;

  // Setup session
  smb = smb_init(share, 1);
  smb->params.req.setup.andx.cmd = 0xFF;
  smb->params.req.setup.max_buffer_size = (unsigned short) SMB_MAX_BUFFER;
  smb->params.req.setup.max_mpx_count = max_mpx_count;
  smb->params.req.setup.ansi_password_length = strlen(server->password);
  smb->params.req.setup.unicode_password_length = 0;
  smb->params.req.setup.capabilities = SMB_CAP_NT_SMBS;

  p = buf;
  p = addstr(p, server->password);
  p = addstrz(p, server->username);
  p = addstrz(p, server->domain);
  p = addstrz(p, SMB_CLIENT_OS);
  p = addstrz(p, SMB_CLIENT_LANMAN);

  rc = smb_request(share, smb, SMB_COM_SESSION_SETUP_ANDX, 13, buf, p - buf, 0); 
  if (rc < 0) goto error;

  server->uid = smb->uid;

  return 0;

error:
  if (server->sock) {
    closesocket(server->sock);
    server->sock = NULL;
  }

  return rc;
}

int smb_disconnect(struct smb_share *share) {
  struct smb_server *server = share->server;
  struct smb *smb;

  if (server->sock) {
    // Logoff server
    if (server->uid != 0xFFFF) {
      smb = smb_init(share, 1);
      smb->params.andx.cmd = 0xFF;
      smb_request(share, smb, SMB_COM_LOGOFF_ANDX, 2, NULL, 0, 0);
      server->uid = 0xFFFF;
    }

    // Close socket
    closesocket(server->sock);
    server->sock = NULL;
  }

  return 0;
}

int smb_get_connection(struct smb_share *share, struct ip_addr *ipaddr, unsigned short port, char *domain, char *username, char *password) {
  struct smb_server *server;
  int rc;

  // Try to find existing connection to server
  server = servers;
  while (server) {
    if (ip_addr_cmp(&server->ipaddr, ipaddr) && server->port == port) {
      // Add share to server
      share->server = server;
      share->next = server->shares;
      server->shares = share;
      server->refcnt++;

      return 0;
    }

    server = server->next;
  }

  // Allocate new server block
  server = (struct smb_server *) kmalloc(sizeof(struct smb_server));
  if (!server) return -ENOMEM;
  memset(server, 0, sizeof(struct smb_server));

  server->ipaddr = *ipaddr;
  server->port = port;
  strcpy(server->domain, domain); 
  strcpy(server->username, username); 
  strcpy(server->password, password);
  server->uid = 0xFFFF;
  init_mutex(&server->lock, 0);

  // Add share to server
  share->tid = 0xFFFF;
  share->server = server;
  share->next = server->shares;
  server->shares = share;
  server->refcnt++;

  // Connect to server
  rc = smb_connect(share);
  if (rc < 0) {
    share->server = NULL;
    kfree(server);
    return rc;
  }

  // Add server to server list
  server->next = servers;
  servers = server;

  return 0;
}

int smb_release_connection(struct smb_share *share) {
  struct smb_server *server = share->server;

  if (!server) return 0;

  if (--server->refcnt > 0) {
    // Remove share from server list
    if (server->shares == share) {
      server->shares = share->next;
    } else {
      struct smb_share *s;

      for (s = server->shares; s != NULL; s = s->next) {
        if (s->next == share) {
          s->next = share->next;
          break;
        }
      }
    }

    share->server = NULL;
    return 0;
  }

  // Disconnect from server
  smb_disconnect(share);

  // Remove server block
  if (servers == server) {
   servers = server->next;
  } else {
    struct smb_server *s;

    for (s = servers; s != NULL; s = s->next) {
      if (s->next == server) {
        s->next = server->next;
        break;
      }
    }
  }

  kfree(server);
  share->server = NULL;

  return 0;
}

int smb_check_connection(struct smb_share *share) {
  struct smb_server *server = share->server;
  int rc;

  if (!server->sock) {
    // Create new connection to server
    rc = smb_connect(share);
    if (rc < 0) return rc;
  }

  if (share->tid == 0xFFFF) {
    // Reconnect share
    rc = smb_connect_tree(share);
    if (rc < 0) return rc;
  }

  return 0;
}

int smb_reconnect(struct smb_share *share) {
  struct smb_server *server = share->server;
  struct smb_share *s;
  int rc;

  s = server->shares;
  while (s) {
    s->tid = 0xFFFF;
    s = s->next;
  }
  server->uid = 0xFFFF;

  if (server->sock) smb_disconnect(share);

  rc = smb_connect(share);
  if (rc < 0) return rc;

  rc = smb_connect_tree(share);
  if (rc < 0) return rc;

  return 0;
}
