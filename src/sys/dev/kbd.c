//
// kbd.c
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
// 3. Neither the name of Michael Ringgaard nor the names of its contributors
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

#define KB_DATA		0x60	// I/O port for keyboard data
#define KB_COMMAND	0x64	// I/O port for keyboard commands (write)
#define KB_STATUS	0x64	// I/O port for keyboard status (read)

#define KB_BUSY		0x02	// status bit set when KEYBD port ready
#define KB_LED	  	0xED	// command to keyboard to set LEDs

#define LED_NUM_LOCK		2
#define LED_SCROLL_LOCK		1
#define LED_CAPS_LOCK		4

#define CK_LSHIFT	        0x01
#define	CK_LALT		        0x02
#define CK_LCTRL		0x04
#define CK_RSHIFT 	        0x10
#define CK_RALT 	        0x20
#define CK_RCTRL 	        0x40

#define KEYBOARD_BUFFER_SIZE    256

struct keytable *keytables[MAX_KEYTABLES];

// Keyboard tables

#include "kbdus.h"
#include "kbddk.h"

// Keyboard status

unsigned char led_status = 0;
unsigned char control_keys = 0;
unsigned int last_scancode = -1;
int ctrl_alt_del_enabled = 1;
int keymap = 0;
int ext;

struct dpc kbddpc;
struct sem kbdsem;

// Circular keyboard buffer

unsigned char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
int keyboard_buffer_in = 0;
int keyboard_buffer_out = 0;


//
// Reboot machine
//

void reboot()
{
  while (_inp(KB_STATUS) & KB_BUSY);
  _outp(KB_COMMAND, 0xFE);
  cli();
  halt();
}		

//
// Set keyboard LEDs
//

static void setleds()
{
  _outp(KB_DATA, KB_LED);
  while (_inp(KB_STATUS) & KB_BUSY);
  _outp(KB_DATA, led_status);
  while (_inp(KB_STATUS) & KB_BUSY);
}

//
// Insert into keyboard buffer
//

static void insert_key(unsigned char ch)
{
  if (((keyboard_buffer_in + 1) & (KEYBOARD_BUFFER_SIZE - 1)) != keyboard_buffer_out)
  {
    keyboard_buffer[keyboard_buffer_in] = ch;
    keyboard_buffer_in = (keyboard_buffer_in + 1) & (KEYBOARD_BUFFER_SIZE - 1);
    release_sem(&kbdsem, 1);
  }
}

//
// Keyboard DPC
//

static void keyb_dpc(void *arg)
{
  unsigned int scancode;
  unsigned int keycode = 0;
  struct keytable *kt;
  int state;

  // Get scancode
  scancode = last_scancode;
  last_scancode = -1;
  if (scancode == -1) return;

  // In scancode mode just insert scancode
  if (keymap == -1)
  {
    insert_key((unsigned char) scancode);
    return;
  }

  // Get keytable
  kt = keytables[keymap];
  if (!kt) return;

  // Extended scancode
  if (scancode == 0xE0)
  {
    ext = 1;
    return;
  }

  //kprintf("scancode %02X %s%s\n", scancode & 0x7F, scancode & 0x80 ? "break" : "make", ext ? " ext" : "");

  // Ctrl-Alt-SysRq
  if ((control_keys & (CK_LCTRL | CK_LALT)) && scancode == 0xD4) 
  {
    dbg_break();
    return;
  }

  // Ctrl-Alt-Del
  if ((control_keys & (CK_LCTRL | CK_LALT)) && scancode == 0x53) 
  {
    if (ctrl_alt_del_enabled) reboot();
  }

  // LED keys, i.e. scroll lock, num lock, and caps lock
  if (scancode == 0x3A)
  {
    // Caps lock
    led_status ^= LED_CAPS_LOCK;
    setleds();
  }

  if (scancode == 0x45)
  {
    // Num lock
    led_status ^= LED_NUM_LOCK;
    setleds();
  }

  if (scancode == 0x46)
  {
    // Scroll lock
    led_status ^= LED_SCROLL_LOCK;
    setleds();
  }

  // Ctrl keys
  if (!ext && scancode == 0x1D) control_keys |= CK_LCTRL;
  if (!ext && scancode == (0x1D | 0x80)) control_keys &= ~CK_LCTRL;

  if (ext && scancode == 0x1D) control_keys |= CK_RCTRL;
  if (ext && scancode == (0x1D | 0x80)) control_keys &= ~CK_RCTRL;

  // Shift keys
  if (scancode == 0x2A) control_keys |= CK_LSHIFT;
  if (scancode == (0x2A | 0x80)) control_keys &= ~CK_LSHIFT;

  if (scancode == 0x36) control_keys |= CK_RSHIFT;
  if (scancode == (0x36 | 0x80)) control_keys &= ~CK_RSHIFT;

  // Alt key
  if (!ext && scancode == 0x38) control_keys |= CK_LALT;
  if (!ext && scancode == 0x80 + 0x38) control_keys &= ~CK_LALT;
	  
  // AltGr key
  if (ext && scancode == 0x38) control_keys |= CK_RALT;
  if (ext && scancode == (0x38 | 0x80)) control_keys &= ~CK_RALT;

  if (scancode < MAX_SCANCODES)
  {
    if ((control_keys & (CK_LSHIFT | CK_RSHIFT)) && (led_status & LED_CAPS_LOCK))
      state = KBSTATE_SHIFTCAPS;
    else if (control_keys & (CK_LSHIFT | CK_RSHIFT))
      state = KBSTATE_SHIFT;
    else if (control_keys & (CK_LCTRL | CK_RCTRL))
      state = KBSTATE_CTRL;
    else if (control_keys & CK_LALT) 
      state = KBSTATE_ALT;
    else if (control_keys & CK_RALT)
      state = KBSTATE_ALTGR;
    else if ((control_keys & (CK_LSHIFT | CK_RSHIFT)) && (led_status & LED_NUM_LOCK))
      state = KBSTATE_SHIFTNUM;
    else if (led_status & LED_CAPS_LOCK) 
      state = KBSTATE_CAPSLOCK;
    else if (led_status & LED_NUM_LOCK) 
      state = KBSTATE_NUMLOCK;
    else if (control_keys == 0) 
      state = KBSTATE_NORMAL;

    //kprintf("(%d,%x)", state, control_keys);

    if (ext)
      keycode = kt->extended[scancode][state];
    else
      keycode = kt->normal[scancode][state];

    if (keycode != 0)
    {
      if (keycode <= 0xFF)
      {
	insert_key((unsigned char) keycode);
      }
      else
      {
	insert_key((unsigned char) (keycode & 0xFF));
	insert_key((unsigned char) (keycode >> 8));
      }
    }
  }

  ext = 0;
}

//
// Keyboard interrupt handler
//

void keyboard_handler(struct context *ctxt, void *arg)
{
  if (last_scancode == -1) 
  {
    // Record scan code
    last_scancode = _inp(KB_DATA) & 0xFF;
    queue_irq_dpc(&kbddpc, keyb_dpc, NULL);
    //kprintf("key %d\n", last_scancode);
  }
  else
  {
    // Keyboard overflow, ignore
    unsigned int lost_scancode = _inp(KB_DATA) & 0xFF;
    //kprintf("key overflow %d\n", lost_scancode);
  }

  eoi(IRQ_KBD);
}

//
// Get one character from keyboard
//

int getch(unsigned int timeout)
{
  unsigned char ch;

  if (wait_for_object(&kbdsem, timeout) < 0) return -ETIMEOUT;

  ch = keyboard_buffer[keyboard_buffer_out];
  keyboard_buffer_out = (keyboard_buffer_out + 1) & (KEYBOARD_BUFFER_SIZE - 1);

  return ch;
}

//
// Test for any keyboard input.
// Returns 0 if keyboard buffer is empty, else returns 1.
//

int kbhit()
{
  return keyboard_buffer_in != keyboard_buffer_out;
}

//
// Initialize keyboard
//

void init_keyboard()
{
  char *kbdname;
  int i;

  // Initialize keyboard tables
  keytables[0] = &uskeys;
  keytables[1] = &dkkeys;

  // Select keyboard table
  kbdname = get_property(krnlcfg, "kernel", "keyboard", "us");
  for (i = 0; i < MAX_KEYTABLES; i++)
  {
    if (keytables[i] && strcmp(keytables[i]->name, kbdname) == 0) 
    {
      keymap = i;
      break;
    }
  }

  // Set all keyboard LEDs off
  led_status = 0; 
  setleds();

  // Setup keyboard interrupt handler
  init_dpc(&kbddpc);
  init_sem(&kbdsem, 0);
  set_interrupt_handler(INTR_KBD, keyboard_handler, NULL);
  enable_irq(IRQ_KBD);
}
