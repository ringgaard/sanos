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

//
// Keyboard Ports
//

#define KBD_DATA        0x60    // I/O port for keyboard data
#define KBD_COMMAND     0x64    // I/O port for keyboard commands (write)
#define KBD_STATUS      0x64    // I/O port for keyboard status (read)

//
// Keyboard Controller Commands
//

#define KBD_CCMD_READ_MODE      0x20    // Read mode bits
#define KBD_CCMD_WRITE_MODE     0x60    // Write mode bits
#define KBD_CCMD_GET_VERSION    0xA1    // Get controller version
#define KBD_CCMD_MOUSE_DISABLE  0xA7    // Disable mouse interface
#define KBD_CCMD_MOUSE_ENABLE   0xA8    // Enable mouse interface
#define KBD_CCMD_TEST_MOUSE     0xA9    // Mouse interface test
#define KBD_CCMD_SELF_TEST      0xAA    // Controller self test
#define KBD_CCMD_KBD_TEST       0xAB    // Keyboard interface test
#define KBD_CCMD_KBD_DISABLE    0xAD    // Keyboard interface disable
#define KBD_CCMD_KBD_ENABLE     0xAE    // Keyboard interface enable
#define KBD_CCMD_WRITE_AUX_OBUF 0xD3    // Write to output buffer as if initiated by the auxiliary device
#define KBD_CCMD_WRITE_MOUSE    0xD4    // Write the following byte to the mouse

//
// Keyboard Commands
//

#define KBD_CMD_SET_LEDS        0xED    // Set keyboard leds
#define KBD_CMD_SET_RATE        0xF3    // Set typematic rate
#define KBD_CMD_ENABLE          0xF4    // Enable scanning
#define KBD_CMD_DISABLE         0xF5    // Disable scanning
#define KBD_CMD_RESET           0xFF    // Reset

//
// Keyboard Replies
//

#define KBD_REPLY_POR           0xAA    // Power on reset
#define KBD_REPLY_ACK           0xFA    // Command ACK
#define KBD_REPLY_RESEND        0xFE    // Command NACK, send command again

//
// Keyboard Status Register Bits
//

#define KBD_STAT_OBF            0x01    // Keyboard output buffer full
#define KBD_STAT_IBF            0x02    // Keyboard input buffer full
#define KBD_STAT_SELFTEST       0x04    // Self test successful
#define KBD_STAT_CMD            0x08    // Last write was a command write
#define KBD_STAT_UNLOCKED       0x10    // Zero if keyboard locked
#define KBD_STAT_MOUSE_OBF      0x20    // Mouse output buffer full
#define KBD_STAT_GTO            0x40    // General receive/xmit timeout
#define KBD_STAT_PERR           0x80    // Parity error

//
// Controller Mode Register Bits
//

#define KBD_MODE_KBD_INT        0x01    // Keyboard data generate IRQ1
#define KBD_MODE_MOUSE_INT      0x02    // Mouse data generate IRQ12
#define KBD_MODE_SYS            0x04    // System flag
#define KBD_MODE_NO_KEYLOCK     0x08    // The keylock doesn't affect the keyboard if set
#define KBD_MODE_DISABLE_KBD    0x10    // Disable keyboard interface
#define KBD_MODE_DISABLE_MOUSE  0x20    // Disable mouse interface
#define KBD_MODE_KCC            0x40    // Scan code conversion to PC format
#define KBD_MODE_RFU            0x80

//
// Keyboard LEDs
//
#define LED_NUM_LOCK            2
#define LED_SCROLL_LOCK         1
#define LED_CAPS_LOCK           4

//
// Control Keys
//

#define CK_LSHIFT               0x01
#define CK_LALT                 0x02
#define CK_LCTRL                0x04
#define CK_RSHIFT               0x10
#define CK_RALT                 0x20
#define CK_RCTRL                0x40

#define KEYBOARD_BUFFER_SIZE    256
#define KEYBOARD_TIMEOUT        100000

struct keytable *keytables[MAX_KEYTABLES];

// Keyboard tables

#include "kbdext.h"
#include "kbdus.h"
#include "kbddk.h"
#include "kbduk.h"
#include "kbdfr.h"

// Keyboard status

unsigned char led_status = 0;
unsigned char control_keys = 0;
int ctrl_alt_del_enabled = 1;
int keymap = 0;
int ext = 0;

struct interrupt kbdintr;
struct dpc kbddpc;
struct sem kbdsem;
dev_t kbddev = -1;

// Circular keyboard buffer

unsigned char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
int keyboard_buffer_in = 0;
int keyboard_buffer_out = 0;

//
// Wait for keyboard ready
//

static void kbd_wait() {
  int tmo = KEYBOARD_TIMEOUT;

  while (inp(KBD_STATUS) & KBD_STAT_IBF) {
    if (--tmo == 0) {
      kprintf("kbd: busy timeout\n");
      return;
    }

    udelay(1);
  }
}

//
// Read data from keyboard
//

static int kbd_read_data() {
  unsigned char status;

  status = inp(KBD_STATUS);
  if (status & KBD_STAT_OBF) {
    unsigned char data = inp(KBD_DATA);

    if (status & (KBD_STAT_GTO | KBD_STAT_PERR)) {
      return -EIO;
    } else {
      return data;
    }
  } else {
    return -EAGAIN;
  }
}

//
// Wait for data from keyboard
//

static int kbd_wait_for_data() {
  int tmo = KEYBOARD_TIMEOUT;

  while (1) {
    int rc = kbd_read_data();
    if (rc >= 0) return rc;

    if (--tmo == 0) {
      //kprintf("kbd: read timeout\n");
      return -ETIMEOUT;
    }

    udelay(1);
  }
}

//
// Write data to keyboard
//

static void kbd_write_data(unsigned char data) {
  outp(KBD_DATA, data);
}

//
// Write command to keyboard
//

static void kbd_write_command(unsigned char cmd) {
  outp(KBD_COMMAND, cmd);
}

//
// Reboot machine
//

void kbd_reboot() {
  kbd_wait();
  kbd_write_command(0xFE);
  cli();
  halt();
}               

//
// Set keyboard LEDs
//

static void setleds() {
  kbd_write_data(KBD_CMD_SET_LEDS);
  kbd_wait();
  kbd_write_data(led_status);
  kbd_wait();
}

//
// Insert into keyboard buffer
//

static void insert_key(unsigned char ch) {
  if (((keyboard_buffer_in + 1) & (KEYBOARD_BUFFER_SIZE - 1)) != keyboard_buffer_out) {
    keyboard_buffer[keyboard_buffer_in] = ch;
    keyboard_buffer_in = (keyboard_buffer_in + 1) & (KEYBOARD_BUFFER_SIZE - 1);
    release_sem(&kbdsem, 1);
    dev_setevt(kbddev, IOEVT_READ);
  }
}

//
// Process keyboard scancode
//

static void process_scancode(unsigned int scancode) {
  unsigned int keycode = 0;
  struct keytable *kt;
  int state;

  // In scancode mode just insert scancode
  if (keymap == -1) {
    insert_key((unsigned char) scancode);
    return;
  }

  // Get keytable
  kt = keytables[keymap];
  if (!kt) return;

  // Extended scancode
  if (scancode == 0xE0) {
    ext = 1;
    return;
  }

  //kprintf("scancode %02X %s%s\n", scancode & 0x7F, scancode & 0x80 ? "break" : "make", ext ? " ext" : "");

  // Ctrl-Alt-SysRq
  if ((control_keys & (CK_LCTRL | CK_LALT)) && scancode == 0xD4) {
    dbg_break();
    return;
  }

  // Ctrl-Alt-Del
  if ((control_keys & (CK_LCTRL | CK_LALT)) && scancode == 0x53) {
    if (ctrl_alt_del_enabled) reboot();
  }

  // LED keys, i.e. scroll lock, num lock, and caps lock
  if (scancode == 0x3A) {
    // Caps lock
    led_status ^= LED_CAPS_LOCK;
    setleds();
  }

  if (scancode == 0x45) {
    // Num lock
    led_status ^= LED_NUM_LOCK;
    setleds();
  }

  if (scancode == 0x46) {
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

  if (scancode < MAX_SCANCODES) {
    if ((control_keys & (CK_LSHIFT | CK_RSHIFT)) && (led_status & LED_CAPS_LOCK)) {
      state = KBSTATE_SHIFTCAPS;
    } else if ((control_keys & CK_LSHIFT) && (control_keys & CK_LCTRL)) {
      state = KBSTATE_SHIFTCTRL;
    } else if (control_keys & (CK_LSHIFT | CK_RSHIFT)) {
      state = KBSTATE_SHIFT;
    } else if (control_keys & (CK_LCTRL | CK_RCTRL)) {
      state = KBSTATE_CTRL;
    } else if (control_keys & CK_LALT) {
      state = KBSTATE_ALT;
    } else if (control_keys & CK_RALT) {
      state = KBSTATE_ALTGR;
    } else if ((control_keys & (CK_LSHIFT | CK_RSHIFT)) && (led_status & LED_NUM_LOCK)) {
      state = KBSTATE_SHIFTNUM;
    } else if (led_status & LED_CAPS_LOCK) {
      state = KBSTATE_CAPSLOCK;
    } else if (led_status & LED_NUM_LOCK) {
      state = KBSTATE_NUMLOCK;
    } else if (control_keys == 0) {
      state = KBSTATE_NORMAL;
    }
    //kprintf("(%d,%x)", state, control_keys);

    if (ext) {
      keycode = extkeys[scancode][state];
    } else {
      keycode = kt->keys[scancode][state];
    }

    if (keycode != 0) {
      if (keycode <= 0xFF) {
        insert_key((unsigned char) keycode);
      } else {
        insert_key((unsigned char) (keycode & 0xFF));
        insert_key((unsigned char) (keycode >> 8));
      }
    }
  }

  ext = 0;
}

//
// Keyboard DPC
//

static void keyb_dpc(void *arg) {
  unsigned int scancode;
  unsigned char status;

  while ((status = inp(KBD_STATUS)) & KBD_STAT_OBF) {
    // Get next scancode from keyboard
    scancode = inp(KBD_DATA) & 0xFF;
    //kprintf("key %d\n", scancode);

    // Process scan code
    process_scancode(scancode);
  }

  eoi(IRQ_KBD);
}

//
// Keyboard interrupt handler
//

int keyboard_handler(struct context *ctxt, void *arg) {
  queue_irq_dpc(&kbddpc, keyb_dpc, NULL);

  return 0;
}

//
// Get one character from keyboard
//

int getch(unsigned int timeout) {
  unsigned char ch;
  int rc;

  rc = wait_for_one_object(&kbdsem, timeout, 1);
  if (rc < 0) return rc;

  ch = keyboard_buffer[keyboard_buffer_out];
  keyboard_buffer_out = (keyboard_buffer_out + 1) & (KEYBOARD_BUFFER_SIZE - 1);

  if (keyboard_buffer_in == keyboard_buffer_out) {
    dev_clrevt(kbddev, IOEVT_READ);
  }

  return ch;
}

//
// Test for any keyboard input.
// Returns 0 if keyboard buffer is empty, else returns 1.
//

int kbhit() {
  return keyboard_buffer_in != keyboard_buffer_out;
}

//
// Reset keyboard hardware
//

int reset_keyboard() {
  int status;

  // Test the keyboard interface
  kbd_wait();
  kbd_write_command(KBD_CCMD_SELF_TEST);
  if (kbd_wait_for_data() != 0x55) {
    kprintf(KERN_WARNING "kbd: Keyboard failed self test\n");
    return -EIO;
  }
  
  // Perform a keyboard interface test. This causes the controller
  // to test the keyboard clock and data lines.
  kbd_wait();
  kbd_write_command(KBD_CCMD_KBD_TEST);
  if (kbd_wait_for_data() != 0x00) {
    kprintf(KERN_WARNING "kbd: Keyboard interface failed self test\n");
    return -EIO;
  }

  // Enable the keyboard by allowing the keyboard clock to run
  kbd_wait();
  kbd_write_command(KBD_CCMD_KBD_ENABLE);

  // Reset keyboard. If the read times out then the assumption is that no keyboard is
  // plugged into the machine.
  while (1) {
    kbd_wait();
    kbd_write_data(KBD_CMD_RESET);
    status = kbd_wait_for_data();
    if (status == KBD_REPLY_ACK) break;
    if (status != KBD_REPLY_RESEND) {
      kprintf(KERN_ERR  "kbd: Keyboard reset failed, no ACK\n");
      return -EIO;
    }
  }

  if ((status = kbd_wait_for_data()) != KBD_REPLY_POR) {
    kprintf(KERN_ERR  "kbd: Keyboard reset failed, no POR (%d)\n", status);
    return -EIO;
  }

  // Set keyboard controller mode. During this, the keyboard should be
  // in the disabled state.
  while (1) {
    kbd_wait();
    kbd_write_data(KBD_CMD_DISABLE);
    status = kbd_wait_for_data();
    if (status == KBD_REPLY_ACK) break;
    if (status != KBD_REPLY_RESEND) {
      kprintf(KERN_ERR  "kbd: Disable keyboard: no ACK\n");
      return -EIO;
    }
  }

  kbd_wait();
  kbd_write_command(KBD_CCMD_WRITE_MODE);

  kbd_wait();
  kbd_write_data(KBD_MODE_KBD_INT | KBD_MODE_SYS | KBD_MODE_DISABLE_MOUSE | KBD_MODE_KCC);

  kbd_wait();
  kbd_write_data(KBD_CMD_ENABLE);
  if (kbd_wait_for_data() != KBD_REPLY_ACK) {
    kprintf("kbd: Enable keyboard: no ACK\n");
    return -EIO;
  }

  // Set the typematic rate to maximum
  kbd_wait();
  kbd_write_data(KBD_CMD_SET_RATE);
  if (kbd_wait_for_data() != KBD_REPLY_ACK) {
    kprintf(KERN_ERR "kbd: Set rate: no ACK\n");
    return -EIO;
  }
  
  kbd_wait();
  kbd_write_data(0x00);
  if (kbd_wait_for_data() != KBD_REPLY_ACK) {
    kprintf(KERN_ERR "kbd: Set rate: no ACK\n");
    return -EIO;
  }

  // Set all keyboard LEDs off
  led_status = 0; 
  setleds();

  return 0;
}

//
// Change keyboard map
//

int change_keyboard_map(char *kbdname) {
  int i;

  for (i = 0; i < MAX_KEYTABLES; i++) {
    if (keytables[i] && strcmp(keytables[i]->name, kbdname) == 0)  {
      keymap = i;
      return 0;
    }
  }

  keymap = 0;
  return -1;
}

int change_keyboard_map_id(int id) {
  if (id < -1 || id >= MAX_KEYTABLES || id != -1 && !keytables[id]) return -EINVAL;
  keymap = id;
  return 0;
}

//
// Initialize keyboard
//

void init_keyboard(dev_t devno, int reset) {
  char *kbdname;

  // Save keyboard device number
  kbddev = devno;

  // Initialize keyboard tables
  keytables[0] = &uskeys;
  keytables[1] = &dkkeys;
  keytables[2] = &ukkeys;
  keytables[3] = &frkeys;

  // Select keyboard table
  kbdname = get_property(krnlcfg, "kernel", "keyboard", "us");
  change_keyboard_map(kbdname);

  // Reset keyboard
  if (reset) reset_keyboard();

  // Setup keyboard interrupt handler
  init_dpc(&kbddpc);
  init_sem(&kbdsem, 0);
  register_interrupt(&kbdintr, INTR_KBD, keyboard_handler, NULL);
  enable_irq(IRQ_KBD);
}
