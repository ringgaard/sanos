//
// kbd.h
//
// Keyboard driver
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

#ifndef KBD_H
#define KBD_H

#define KBSTATE_NORMAL          0
#define KBSTATE_SHIFT           1
#define KBSTATE_CTRL            2
#define KBSTATE_ALT             3
#define KBSTATE_NUMLOCK         4
#define KBSTATE_CAPSLOCK        5
#define KBSTATE_SHIFTCAPS       6
#define KBSTATE_SHIFTNUM        7
#define KBSTATE_ALTGR           8
#define KBSTATE_SHIFTCTRL       9

#define MAX_KBSTATES            10

#define MAX_SCANCODES           0x80
#define MAX_KEYTABLES           16

struct keytable {
  char *name;
  unsigned short keys[MAX_SCANCODES][MAX_KBSTATES];
};

extern int ctrl_alt_del_enabled;
extern int keymap;

krnlapi struct keytable *keytables[MAX_KEYTABLES];

void init_keyboard(dev_t devno, int reset);
int change_keyboard_map_id(int id);

int getch(unsigned int timeout);
int kbhit();
void kbd_reboot();

#endif
