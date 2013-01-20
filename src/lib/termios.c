//
// termios.c
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

#include <os.h>
#include <termios.h>

int cfsetispeed(struct termios *termios, speed_t speed) {
  errno = ENOSYS;
  return -1;
}

int cfsetospeed(struct termios *termios, speed_t speed) {
  errno = ENOSYS;
  return -1;
}


speed_t cfgetispeed (struct termios *termios) {
  errno = ENOSYS;
  return -1;
}

speed_t cfgetospeed (struct termios *termios) {
  errno = ENOSYS;
  return -1;
}

int tcdrain(handle_t f) {
  errno = ENOSYS;
  return -1;
}

int tcflow(handle_t f, int action) {
  errno = ENOSYS;
  return -1;
}

int tcflush(handle_t f, int control) {
  errno = ENOSYS;
  return -1;
}

int tcsendbreak(handle_t f, int duration) {
  errno = ENOSYS;
  return -1;
}


int tcgetattr(handle_t f, struct termios *termios) {
  errno = ENOSYS;
  return -1;
}

int tcsetattr(handle_t f, int flag, struct termios *termios) {
  errno = ENOSYS;
  return -1;
}

int tcgetsize(handle_t f, int *rows, int *cols) {
  errno = ENOSYS;
  return -1;
}
