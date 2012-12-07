//
// telnetd.c
//
// Telnet daemon
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
#include <inifile.h>
#include <stdio.h>
#include <string.h>

//
// Events
//

#define APPDON  0  // Application terminated
#define USRATT  10 // User needs attention for reading input
#define USRRDY  11 // User is ready to receive output
#define APPATT  12 // Application needs attention for writing output
#define APPRDY  13 // Application is ready to receive input

//
// States
//

#define STATE_NORMAL 0
#define STATE_IAC    1
#define STATE_OPT    2
#define STATE_SB     3
#define STATE_OPTDAT 4
#define STATE_SE     5

//
// Special telnet characters
//

#define TELNET_SE    240   // End of subnegotiation parameters
#define TELNET_NOP   241   // No operation
#define TELNET_MARK  242   // Data mark
#define TELNET_BRK   243   // Break
#define TELNET_IP    244   // Interrupt process
#define TELNET_AO    245   // Abort output
#define TELNET_AYT   246   // Are you there
#define TELNET_EC    247   // Erase character
#define TELNET_EL    248   // Erase line
#define TELNET_GA    249   // Go ahead
#define TELNET_SB    250   // Start of subnegotiation parameters
#define TELNET_WILL  251   // Will option code
#define TELNET_WONT  252   // Won't option code
#define TELNET_DO    253   // Do option code
#define TELNET_DONT  254   // Don't option code
#define TELNET_IAC   255   // Interpret as command

//
// Telnet options
//

#define TELOPT_TRANSMIT_BINARY      0  // Binary Transmission (RFC856)
#define TELOPT_ECHO                 1  // Echo (RFC857)
#define TELOPT_SUPPRESS_GO_AHEAD    3  // Suppress Go Ahead (RFC858)
#define TELOPT_STATUS               5  // Status (RFC859)
#define TELOPT_TIMING_MARK          6  // Timing Mark (RFC860)
#define TELOPT_NAOCRD              10  // Output Carriage-Return Disposition (RFC652)
#define TELOPT_NAOHTS              11  // Output Horizontal Tab Stops (RFC653)
#define TELOPT_NAOHTD              12  // Output Horizontal Tab Stop Disposition (RFC654)
#define TELOPT_NAOFFD              13  // Output Formfeed Disposition (RFC655)
#define TELOPT_NAOVTS              14  // Output Vertical Tabstops (RFC656)
#define TELOPT_NAOVTD              15  // Output Vertical Tab Disposition (RFC657)
#define TELOPT_NAOLFD              16  // Output Linefeed Disposition (RFC658)
#define TELOPT_EXTEND_ASCII        17  // Extended ASCII (RFC698)
#define TELOPT_TERMINAL_TYPE       24  // Terminal Type (RFC1091)
#define TELOPT_NAWS                31  // Negotiate About Window Size (RFC1073)
#define TELOPT_TERMINAL_SPEED      32  // Terminal Speed (RFC1079)
#define TELOPT_TOGGLE_FLOW_CONTROL 33  // Remote Flow Control (RFC1372)
#define TELOPT_LINEMODE            34  // Linemode (RFC1184)
#define TELOPT_AUTHENTICATION      37  // Authentication (RFC1416)

//
// Globals
//

#define STOPPED 0
#define RUNNING 1

int port;
char *pgm;
int sock = NOHANDLE;
int state = STOPPED;

int off = 0;
int on = 1;

struct buffer {
  unsigned char data[4096];
  unsigned char *start;
  unsigned char *end;
};

struct termstate {
  int sock;
  int state;
  int code;
  unsigned char optdata[256];
  int optlen;
  struct term term;
  struct buffer bi;
  struct buffer bo;
};

void sendopt(struct termstate *ts, int code, int option) {
  unsigned char buf[3];

  buf[0] = TELNET_IAC;
  buf[1] = (unsigned char) code;
  buf[2] = (unsigned char) option;
  write(ts->sock, buf, 3);
}

void parseopt(struct termstate *ts, int code, int option)
{
  //static char *codename[4] = {"WILL", "WONT", "DO", "DONT"};
  //syslog(LOG_DEBUG, "%s %d", codename[code - 251], option);

  switch (option) {
    case TELOPT_ECHO:
    case TELOPT_SUPPRESS_GO_AHEAD:
    case TELOPT_NAWS:
      break;

    case TELOPT_TERMINAL_TYPE:
    case TELOPT_TERMINAL_SPEED:
      sendopt(ts, TELNET_DO, option);
      break;

    default:
      if (code == TELNET_WILL || code == TELNET_WONT) {
        sendopt(ts, TELNET_DONT, option);
      } else {
        sendopt(ts, TELNET_WONT, option);
      }
  }
}

void parseoptdat(struct termstate *ts, int option, unsigned char *data, int len) {
  //syslog(LOG_DEBUG, "OPTION %d data (%d bytes)", option, len);

  switch (option) {
    case TELOPT_NAWS:
      if (len == 4) {
        int cols = ntohs(*(unsigned short *) data);
        int lines = ntohs(*(unsigned short *) (data + 2));
        //syslog(LOG_DEBUG, "WINDOW %d cols %d lines", cols, lines);
        if (cols != 0) ts->term.cols = cols;
        if (lines != 0) ts->term.lines = lines;
      }
      break;

    case TELOPT_TERMINAL_SPEED:
      //syslog(LOG_DEBUG, "TERMINAL SPEED %*s", len, data);
      break;

    case TELOPT_TERMINAL_TYPE:
      //syslog(LOG_DEBUG, "TERMINAL TYPE %*s", len, data);
      break;
  }
}

void parse(struct termstate *ts) {
  unsigned char *p = ts->bi.start;
  unsigned char *q = p;

  while (p < ts->bi.end) {
    int c = *p++;

    switch (ts->state) {
      case STATE_NORMAL:
        if (c == TELNET_IAC) {
          ts->state = STATE_IAC;
        } else {
          *q++ = c;
        }
        break;

      case STATE_IAC:
        switch (c) {
          case TELNET_IAC:
            *q++ = c;
            ts->state = STATE_NORMAL;
            break;

          case TELNET_WILL:
          case TELNET_WONT:
          case TELNET_DO:
          case TELNET_DONT:
            ts->code = c;
            ts->state = STATE_OPT;
            break;

          case TELNET_SB:
            ts->state = STATE_SB;
            break;

          default:
            ts->state = STATE_NORMAL;
        }
        break;

      case STATE_OPT:
        parseopt(ts, ts->code, c);
        ts->state = STATE_NORMAL;
        break;

      case STATE_SB:
        ts->code = c;
        ts->optlen = 0;
        ts->state = STATE_OPTDAT;
        break;

      case STATE_OPTDAT:
        if (c == TELNET_IAC) {
          ts->state = STATE_SE;
        } else if (ts->optlen < sizeof(ts->optdata)) {
          ts->optdata[ts->optlen++] = c;
        }
        break;

      case STATE_SE:
        if (c == TELNET_SE) parseoptdat(ts, ts->code, ts->optdata, ts->optlen);
        ts->state = STATE_NORMAL;
        break;
    } 
  }

  ts->bi.end = q;
}

//
//    +-----------+            +-----------+ pin[1]     pin[0]=in +-----------+
//    |           |            |           |--------------------->|           |
//    |   user    |<---------->|  telnetd  |       pipes          |    app    |
//    |           |      s     |           |<---------------------|           |
//    +-----------+            +-----------+ pout[0]  pout[1]=out +-----------+
//

void __stdcall telnet_task(void *arg) {
  struct process *proc = gettib()->proc;
  struct tib *apptib;
  int s = (int) arg;
  int pin[2];
  int pout[2];
  int events[2];
  int app;
  int mux;
  int n;
  int last_was_cr;
  struct termstate ts;

  // Set process identifer
  if (!proc->ident && !proc->cmdline) {
    struct sockaddr_in sin;
    int sinlen = sizeof sin;

    getpeername(s, (struct sockaddr *) &sin, &sinlen);
    proc->ident = strdup("user");
    proc->cmdline = strdup(inet_ntoa(sin.sin_addr));
  }

  // Initialize terminal state
  memset(&ts, 0, sizeof(struct termstate));
  ts.sock = s;
  ts.state = STATE_NORMAL;
  ts.term.type = TERM_VT100;
  ts.term.cols = 80;
  ts.term.lines = 25;

  // Allocate pipes
  if (pipe(pin) < 0) return;
  if (pipe(pout) < 0) return;

  // Set non-blocking writes
  ioctl(s, FIONBIO, &on, sizeof(on));
  ioctl(pin[1], FIONBIO, &on, sizeof(on));

  // Initialize multiplexer
  mux = mkiomux(0);
  if (mux < 0) return;
  dispatch(mux, s, IOEVT_READ | IOEVT_ERROR | IOEVT_CLOSE, USRATT);
  dispatch(mux, pout[0], IOEVT_READ | IOEVT_ERROR | IOEVT_CLOSE, APPATT);
  
  // Spawn initial telnet application
  app = spawn(P_NOWAIT | P_SUSPEND | P_DETACH, NULL, pgm, NULL, &apptib);
  if (app < 0) return;

  // Setup standard input, output, and error
  apptib->proc->iob[0] = pin[0];
  apptib->proc->iob[1] = pout[1];
  apptib->proc->iob[2] = dup(pout[1]);
  apptib->proc->term = &ts.term;
  ts.term.ttyin = pin[0];
  ts.term.ttyout = pout[1];
  ioctl(pin[0], IOCTL_SET_TTY, &on, sizeof(on));
  ioctl(pout[1], IOCTL_SET_TTY, &on, sizeof(on));
  resume(app);

  // Initialize event handle array
  events[0] = app;
  events[1] = mux;

  // Send initial options
  sendopt(&ts, TELNET_WILL, TELOPT_ECHO);
  sendopt(&ts, TELNET_WILL, TELOPT_SUPPRESS_GO_AHEAD);
  sendopt(&ts, TELNET_WONT, TELOPT_LINEMODE);
  sendopt(&ts, TELNET_DO, TELOPT_NAWS);

  last_was_cr = 0;
  for (;;) {
    // Wait for application termination or multiplexer event
    int rc = waitany(events, 2, INFINITE);

    switch (rc) {
      case APPDON: 
        // Application terminated
        goto done;

      case USRATT:
        // Data arrived from user
        if (ts.bi.start == ts.bi.end) {
          int i;

          // Read data from user
          n = read(s, ts.bi.data, sizeof ts.bi.data);
          if (n < 0) goto done;

          // Convert cr nul to cr lf.
          for (i = 0; i < n; ++i) {
            unsigned char ch = ts.bi.data[i];
            if (ch == 0 && last_was_cr) ts.bi.data[i] = '\n';
            last_was_cr = (ch == '\r');
          }

          ts.bi.start = ts.bi.data;
          ts.bi.end = ts.bi.data + n;
        }

        // Parse user input for telnet options
        parse(&ts);

        // Fall through

      case APPRDY:
        // Application ready to receive
        if (ts.bi.start != ts.bi.end) {
          // Write data to application
          n = write(pin[1], ts.bi.start, ts.bi.end - ts.bi.start);
          if (n < 0) goto done;
          ts.bi.start += n;
        }

        if (ts.bi.start == ts.bi.end) {
          dispatch(mux, s, IOEVT_READ | IOEVT_ERROR | IOEVT_CLOSE, USRATT);
        } else {
          dispatch(mux, pin[1], IOEVT_WRITE | IOEVT_ERROR | IOEVT_CLOSE, APPRDY);
        }
        break;

      case APPATT:
        // Data arrived from application
        if (ts.bo.start == ts.bo.end) {
          // Read data from application
          n = read(pout[0], ts.bo.data, sizeof ts.bo.data);
          if (n < 0) goto done;
          ts.bo.start = ts.bo.data;
          ts.bo.end = ts.bo.data + n;
        }

        // Fall through

      case USRRDY:
        // User ready to receive
        if (ts.bo.start != ts.bo.end) {
          // Write data to user
          n = write(s, ts.bo.start, ts.bo.end - ts.bo.start);
          if (n < 0) goto done;
          ts.bo.start += n;
        }

        if (ts.bo.start == ts.bo.end) {
          dispatch(mux, pout[0], IOEVT_READ | IOEVT_ERROR | IOEVT_CLOSE, APPATT);
        } else {
          dispatch(mux, s, IOEVT_WRITE | IOEVT_ERROR | IOEVT_CLOSE, USRRDY);
        }
        break;

      default:
        // Error occured
        goto done;
    }
  }

done:
  syslog(LOG_DEBUG, "telnet client disconnected");
  close(pin[1]);
  close(pout[0]);
  close(s);
  close(app);
  close(mux);
}

int main(int argc, char *argv[]) {
  int s;
  int rc;
  struct sockaddr_in sin;
  int hthread;

  if (argc == 2) {
    if (strcmp(argv[1], "start") == 0) {
      char path[MAXPATH];
      
      getmodpath(NULL, path, MAXPATH);
      hthread = spawn(P_NOWAIT | P_DETACH, path, "", NULL, NULL);
      if (hthread < 0) syslog(LOG_ERR, "error %d (%s) in spawn", errno, strerror(errno));
      close(hthread);
      return 0;
    } else if (strcmp(argv[1], "stop") == 0) {
      state = STOPPED;
      close(sock);
      return 0;
    }
  }

  port = get_numeric_property(osconfig(), "telnetd", "port", 23);
  pgm = get_property(osconfig(), "telnetd", "pgm", "login");

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    syslog(LOG_ERR, "error %d (%s) in socket", errno, strerror(errno));
    return 1;
  }

  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(port);
  rc = bind(sock, (struct sockaddr *) &sin, sizeof sin);
  if (rc < 0) {
    syslog(LOG_ERR, "error %d (%s) in bind", errno, strerror(errno));
    return 1;
  }

  rc = listen(sock, 5);
  if (rc < 0) {
    syslog(LOG_ERR, "error %d (%s) in listen", errno, strerror(errno));
    return 1;
  }

  syslog(LOG_INFO, "telnetd started on port %d", port);
  state = RUNNING;
  while (1) {
    struct sockaddr_in sin;

    s = accept(sock, (struct sockaddr *) &sin, NULL);
    if (state == STOPPED) break;
    if (s < 0) {
      syslog(LOG_ERR, "error %d (%s) in accept", errno, strerror(errno));
      return 1;
    }

    syslog(LOG_INFO, "client connected from %a", &sin.sin_addr);

    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char *) &off, sizeof(off));

    hthread = beginthread(telnet_task, 0, (void *) s, CREATE_NEW_PROCESS | CREATE_DETACHED, "telnet", NULL);
    close(hthread);
  }

  syslog(LOG_INFO, "telnetd stopped");
}
