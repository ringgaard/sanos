//
// kbd.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Keyboard driver
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
unsigned int last_key = -1;
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
  unsigned int key = last_key;
  unsigned int key_ascii = 0;
  struct keytable *kt;
  int state;

  last_key = -1;

  // In scancode mode just insert scancode
  if (keymap == -1)
  {
    insert_key((unsigned char) key);
    return;
  }

  // Get keytable
  kt = keytables[keymap];

  // Extended scancode
  if (key == 0xE0)
  {
    ext = 1;
    return;
  }

  //kprintf("scancode %02X %s%s\n", key & 0x7F, key & 0x80 ? "break" : "make", ext ? " ext" : "");

  // Ctrl-Alt-SysRq
  if ((control_keys & (CK_LCTRL | CK_LALT)) && key == 0xD4) 
  {
    dbg_break();
    return;
  }

  // Ctrl-Alt-Del
  if ((control_keys & (CK_LCTRL | CK_LALT)) && key == 0x53) 
  {
    if (ctrl_alt_del_enabled) reboot();
  }

  // LED keys, i.e. scroll lock, num lock, and caps lock
  if (key == 0x3A)
  {
    // Caps lock
    led_status ^= LED_CAPS_LOCK;
    setleds();
  }

  if (key == 0x45)
  {
    // Num lock
    led_status ^= LED_NUM_LOCK;
    setleds();
  }

  if (key == 0x46)
  {
    // Scroll lock
    led_status ^= LED_SCROLL_LOCK;
    setleds();
  }

  // Ctrl keys
  if (!ext && key == 0x1D) control_keys |= CK_LCTRL;
  if (!ext && key == (0x1D | 0x80)) control_keys &= ~CK_LCTRL;

  if (ext && key == 0x1D) control_keys |= CK_RCTRL;
  if (ext && key == (0x1D | 0x80)) control_keys &= ~CK_RCTRL;

  // Shift keys
  if (key == 0x2A) control_keys |= CK_LSHIFT;
  if (key == (0x2A | 0x80)) control_keys &= ~CK_LSHIFT;

  if (key == 0x36) control_keys |= CK_RSHIFT;
  if (key == (0x36 | 0x80)) control_keys &= ~CK_RSHIFT;

  // Alt key
  if (!ext && key == 0x38) control_keys |= CK_LALT;
  if (!ext && key == 0x80 + 0x38) control_keys &= ~CK_LALT;
	  
  // AltGr key
  if (ext && key == 0x38) control_keys |= CK_RALT;
  if (ext && key == (0x38 | 0x80)) control_keys &= ~CK_RALT;

  if (key < MAX_SCANCODES)
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
      key_ascii = kt->extended[key][state];
    else
      key_ascii = kt->normal[key][state];

    if (key_ascii != 0)
    {
      if (key_ascii <= 0xFF)
      {
	insert_key((unsigned char) key_ascii);
      }
      else
      {
	insert_key((unsigned char) (key_ascii & 0xFF));
	insert_key((unsigned char) (key_ascii >> 8));
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
  if (last_key == -1) 
  {
    // Record scan code
    last_key = _inp(KB_DATA) & 0xFF;
    queue_irq_dpc(&kbddpc, keyb_dpc, NULL);
    //kprintf("key %d\n", last_key);
  }
  else
  {
    // Keyboard overflow, ignore
    unsigned int lost_key = _inp(KB_DATA) & 0xFF;
    //kprintf("key overflow %d\n", lost_key);
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

#if 0
  {
    char x[256];
    unsigned short *arr; 

    memset(x, 0, 256);
    arr = (unsigned short *) &(keytables[keymap]->normal[0]);
    for (i = 0; i < 0x80 * 9; i++)
    {
      if (*arr > 0xFF) x[*arr >> 8] = 1;
      arr++;
    }

    arr = (unsigned short *) &(keytables[keymap]->extended[0]);
    for (i = 0; i < 0x80 * 9; i++)
    {
      if (*arr > 0xFF) x[*arr >> 8] = 1;
      arr++;
    }

    for (i = 0; i < 256; i++) if (x[i] == 0) kprintf("code %02X unused\n", i);
  }
#endif

  // Set all keyboard LEDs off
  led_status = 0; 
  setleds();

  // Setup keyboard interrupt handler
  init_dpc(&kbddpc);
  init_sem(&kbdsem, 0);
  set_interrupt_handler(INTR_KBD, keyboard_handler, NULL);
  enable_irq(IRQ_KBD);
}
