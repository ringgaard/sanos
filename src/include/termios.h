//
// termios.h
//
// Terminal control interface
//
// Copyright (C) 2013 Michael Ringgaard. All rights reserved.
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

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef TERMIOS_H
#define TERMIOS_H

#include <sys/types.h>

typedef unsigned long speed_t;
typedef unsigned long tcflag_t;
typedef unsigned char cc_t;

//
// Terminal state
//

#define NCCS       11

struct termios   {
  tcflag_t c_iflag;       // Input modes
  tcflag_t c_oflag;       // Output modes
  tcflag_t c_cflag;       // Control modes
  tcflag_t c_lflag;       // Local modes
  cc_t c_cc[NCCS];        // Control characters
  speed_t c_ispeed;       // Input baudrate
  speed_t c_ospeed;       // Output baudrate
};

//
// Control characters (c_cc)
//

#define VEOF       0      // End of file
#define VEOL       1      // End of line
#define VERASE     2	  // Erase one character
#define VWERASE    3	  // Erase one word
#define VKILL      4	  // Kill current line of input
#define VINTR      5	  // Signal process group
#define VQUIT      6	  // Abort process group
#define VSTART     7	  // Start output
#define VSTOP      8      // Stop output
#define VMIN       9
#define VTIME     10

//
// Input modes (c_iflag)
//

#define IGNBRK    0x0001  // Ignore breaks
#define BRKINT    0x0002  // Map breaks to VINTR
#define IGNPAR    0x0004  // Ignore parity errors
#define PARMRK    0x0008  // Mark parity errors
#define INPCK     0x0010  // Enable parity checking
#define ISTRIP    0x0020  // Strip high bit
#define INLCR     0x0040  // NL -> CR
#define IGNCR     0x0080  // Ignore CR
#define ICRNL     0x0100  // CR -> NL */
#define IXON      0x0200  // Outgoing flow control
#define IXOFF     0x0400  // Incoming flow control

//
// Output modes (c_oflag)
//

#define OPOST     0x0001  // Enable output processing
#define ONLCR     0x0002  // NL -> CR/NL

//
// Control modes (c_cflag)
//

#define CSIZE     0x000F  // Character size mask
#define CS5       0x0005  // 7 bits per character
#define CS6       0x0006  // 7 bits per character
#define CS7       0x0007  // 7 bits per character
#define CS8       0x0008  // 8 bits per character

#define PARENB    0x0010  // Parity enable
#define PARODD    0x0020  // Odd parity

#define CSTOPB    0x0040  // 2 stop bits, otherwise 1

#define CREAD     0x0100  // Enable receiver
#define HUPCL     0x0200  // Hang up on last close
#define CLOCAL    0x0400  // Ignore modem status lines

//
// Local modes (c_lflag)
//

#define ECHO      0x0001  // Enable echo
#define ECHOE     0x0002  // Echo erase character as error-correcting backspace
#define ECHOK     0x0004  // Echo KILL
#define ECHONL    0x0008  // Echo NL
#define ISIG      0x0010  // Enable signals
#define ICANON    0x0020  // Enable line-oriented input processing
#define NOFLSH    0x0040  // Disable flush after interrupt or quit

//
// Operations for tcsetattr()
//

#define TCSANOW   0	  // Change attributes immediately
#define TCSADRAIN 1       // Change attributes when output has drained
#define TCSAFLUSH 2       // Change attributes when output has drained; also flush pending input

//
// Baudrate
//

#define B0            0
#define B50          50
#define B75          75
#define B110        110
#define B134        134
#define B150        150
#define B200        200
#define B300        300
#define B600        600
#define B1200      1200
#define B1800      1800
#define B2400      2400
#define B4800      4800
#define B9600      9600
#define B19200    19200
#define B38400    38400

//
// Line control
//

#define TCIFLUSH  1       // Flush pending input
#define TCOFLUSH  2       // Flush untransmitted output
#define TCIOFLUSH 3       // Flush both pending input and untransmitted output

//
// Flow control
//

#define TCIOFF    0x0001  // Transmit a STOP character, intended to suspend input data
#define TCION     0x0002  // Transmit a START character, intended to restart input data
#define TCOOFF    0x0004  // Suspend output
#define TCOON     0x0008  // Restart output

#ifdef  __cplusplus
extern "C" {
#endif

int cfsetispeed(struct termios *termios, speed_t speed); 
int cfsetospeed(struct termios *termios, speed_t speed);
speed_t cfgetispeed (struct termios *termios);
speed_t cfgetospeed (struct termios *termios);

int tcdrain(handle_t f);
int tcflow(handle_t f, int action);
int tcflush(handle_t f, int control);
int tcsendbreak(handle_t f, int duration);

int tcgetattr(handle_t f, struct termios *termios);
int tcsetattr(handle_t f, int flag, struct termios *termios);
int tcgetsize(handle_t f, int *rows, int *cols);

#ifdef  __cplusplus
}
#endif

#endif
