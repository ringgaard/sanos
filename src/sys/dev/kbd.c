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

#define CK_SHIFT	        1
#define	CK_ALT		        2
#define CK_CTRL		        4

#define KEYBOARD_BUFFER_SIZE    256

// Keyboard status

unsigned char led_status = 0;
unsigned char control_keys = 0;
unsigned int last_key = -1;
int ctrl_alt_del_enabled = 0;
int keymap = 0;

struct dpc kbddpc;
struct sem kbdsem;

// Circular keyboard buffer

unsigned char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
int keyboard_buffer_in = 0;
int keyboard_buffer_out = 0;

// Scancode to ASCII conversion table

unsigned int scan2ascii[][8] = 
{
  //      ASCII - shift - ctrl -  alt -   num -   caps -  shift caps -    shift num
  {       0,      0,      0,      0,      0,      0,      0,              0},
  {       0x1B,   0x1B,   0x1B,   0,      0x1B,   0x1B,   0x1B,           0x1B},
  // 1234567890
  {       0x31,   0x21,   0,      0x7800, 0x31,   0x31,   0x21,           0x21},
  {       0x32,   0x40,   0x0300, 0x7900, 0x32,   0x32,   0x40,           0x40},
  {       0x33,   0x23,   0,      0x7A00, 0x33,   0x33,   0x23,           0x23},
  {       0x34,   0x24,   0,      0x7B00, 0x34,   0x34,   0x24,           0x24},
  {       0x35,   0x25,   0,      0x7C00, 0x35,   0x35,   0x25,           0x25},
  {       0x36,   0x5E,   0x1E,   0x7D00, 0x36,   0x36,   0x5E,           0x5E},
  {       0x37,   0x26,   0,      0x7E00, 0x37,   0x37,   0x26,           0x26},
  {       0x38,   0x2A,   0,      0x7F00, 0x38,   0x38,   0x2A,           0x2A},
  {       0x39,   0x28,   0,      0x8000, 0x39,   0x39,   0x28,           0x28},
  {       0x30,   0x29,   0,      0x8100, 0x30,   0x30,   0x29,           0x29},
  // -, =, bksp, tab
  {       0x2D,   0x5F,   0x1F,   0x8200, 0x2D,   0x2D,   0x5F,           0x5F},
  {       0x3D,   0x2B,   0,      0x8300, 0x3D,   0x3D,   0x2B,           0x2B},
  {       0x08,   0x08,   0x7F,   0,      0x08,   0x08,   0x08,           0x08},
  {       0x09,   0x0F00, 0,      0,      0x09,   0x09,   0x0F00,         0x0F00},
  // QWERTYUIOP[]
  {       0x71,   0x51,   0x11,   0x1000, 0x71,   0x51,   0x71,           0x51},
  {       0x77,   0x57,   0x17,   0x1100, 0x77,   0x57,   0x77,           0x57},
  {       0x65,   0x45,   0x05,   0x1200, 0x65,   0x45,   0x65,           0x45},
  {       0x72,   0x52,   0x12,   0x1300, 0x72,   0x52,   0x72,           0x52},
  {       0x74,   0x54,   0x14,   0x1400, 0x74,   0x54,   0x74,           0x54},
  {       0x79,   0x59,   0x19,   0x1500, 0x79,   0x59,   0x79,           0x59},
  {       0x75,   0x55,   0x15,   0x1600, 0x75,   0x55,   0x75,           0x55},
  {       0x69,   0x49,   0x09,   0x1700, 0x69,   0x49,   0x69,           0x49},
  {       0x6F,   0x4F,   0x0F,   0x1800, 0x6F,   0x4F,   0x6F,           0x4F},
  {       0x70,   0x50,   0x10,   0x1900, 0x70,   0x50,   0x70,           0x50},
  {       0x5B,   0x7B,   0x1B,   0x0,    0x5B,   0x5B,   0x7B,           0x7B},
  {       0x5D,   0x7D,   0x1D,   0,      0x5D,   0x5D,   0x7D,           0x7D},
  // enter, ctrl
  {       0x0A,   0x0A,   0x0D,   0,      0x0A,   0x0A,   0x0D,           0x0D},
  {       0,      0,      0,      0,      0,      0,      0,              0},
  // ASDFGHJKL;'~
  {       0x61,   0x41,   0x01,   0x1E00, 0x61,   0x41,   0x61,           0x41},
  {       0x73,   0x53,   0x13,   0x1F00, 0x73,   0x53,   0x73,           0x53},
  {       0x64,   0x44,   0x04,   0x2000, 0x64,   0x44,   0x64,           0x44},
  {       0x66,   0x46,   0x06,   0x2100, 0x66,   0x46,   0x66,           0x46},
  {       0x67,   0x47,   0x07,   0x2200, 0x67,   0x47,   0x67,           0x47},
  {       0x68,   0x48,   0x08,   0x2300, 0x68,   0x48,   0x68,           0x48},
  {       0x6A,   0x4A,   0x0A,   0x2400, 0x6A,   0x4A,   0x6A,           0x4A},
  {       0x6B,   0x4B,   0x0B,   0x3500, 0x6B,   0x4B,   0x6B,           0x4B},
  {       0x6C,   0x4C,   0x0C,   0x2600, 0x6C,   0x4C,   0x6C,           0x4C},
  {       0x3B,   0x3A,   0,      0,      0x3B,   0x3B,   0x3A,           0x3A},
  {       0x27,   0x22,   0,      0,      0x27,   0x27,   0x22,           0x22},
  {       0x60,   0x7E,   0,      0,      0x60,   0x60,   0x7E,           0x7E},
  // left shift
  {       0x2A,   0,      0,      0,      0,      0,      0,              0},
  // \ZXCVBNM,./
  {       0x5C,   0x7C,   0x1C,   0,      0x5C,   0x5C,   0x7C,           0x7C},
  {       0x7A,   0x5A,   0x1A,   0x2C00, 0x7A,   0x5A,   0x7A,           0x5A},
  {       0x78,   0x58,   0x18,   0x2D00, 0x78,   0x58,   0x78,           0x58},
  {       0x63,   0x43,   0x03,   0x2E00, 0x63,   0x43,   0x63,           0x43},
  {       0x76,   0x56,   0x16,   0x2F00, 0x76,   0x56,   0x76,           0x56},
  {       0x62,   0x42,   0x02,   0x3000, 0x62,   0x42,   0x62,           0x42},
  {       0x6E,   0x4E,   0x0E,   0x3100, 0x6E,   0x4E,   0x6E,           0x4E},
  {       0x6D,   0x4D,   0x0D,   0x3200, 0x6D,   0x4D,   0x6D,           0x4D},
  {       0x2C,   0x3C,   0,      0,      0x2C,   0x2C,   0x3C,           0x3C},
  {       0x2E,   0x3E,   0,      0,      0x2E,   0x2E,   0x3E,           0x3E},
  {       0x2F,   0x3F,   0,      0,      0x2F,   0x2F,   0x3F,           0x3F},
  // right shift
  {       0,      0,      0,      0,      0,      0,      0,              0},
  // print screen
  {       0,      0,      0,      0,      0,      0,      0,              0},
  // alt
  {       0,      0,      0,      0,      0,      0,      0,              0},
  // space
  {       0x20,   0x20,   0x20,   0,      0x20,   0x20,   0x20,           0x20},
  // caps
  {       0,      0,      0,      0,      0,      0,      0,              0},
  // F1-F10
  {       0x3B00, 0x5400, 0x5E00, 0x6800, 0x3B00, 0x3B00, 0x5400,         0x5400},
  {       0x3C00, 0x5500, 0x5F00, 0x6900, 0x3C00, 0x3C00, 0x5500,         0x5500},
  {       0x3D00, 0x5600, 0x6000, 0x6A00, 0x3D00, 0x3D00, 0x5600,         0x5600},
  {       0x3E00, 0x5700, 0x6100, 0x6B00, 0x3E00, 0x3E00, 0x5700,         0x5700},
  {       0x3F00, 0x5800, 0x6200, 0x6C00, 0x3F00, 0x3F00, 0x5800,         0x5800},
  {       0x4000, 0x5900, 0x6300, 0x6D00, 0x4000, 0x4000, 0x5900,         0x5900},
  {       0x4100, 0x5A00, 0x6400, 0x6E00, 0x4100, 0x4100, 0x5A00,         0x5A00},
  {       0x4200, 0x5B00, 0x6500, 0x6F00, 0x4200, 0x4200, 0x5B00,         0x5B00},
  {       0x4300, 0x5C00, 0x6600, 0x7000, 0x4300, 0x4300, 0x5C00,         0x5C00},
  {       0x4400, 0x5D00, 0x6700, 0x7100, 0x4400, 0x4400, 0x5D00,         0x5D00},
  // num lock, scrl lock
  {       0,      0,      0,      0,      0,      0,      0,              0},
  {       0,      0,      0,      0,      0,      0,      0,              0},
  // home, up, pgup, -kpad, left, center, right, +keypad, end, down, pgdn, ins, del
  {       0x4700, 0x37,   0x7700, 0,      0x37,   0x4700, 0x37,           0x4700},
  {       0x4800, 0x38,   0,      0,      0x38,   0x4800, 0x38,           0x4800},
  {       0x4900, 0x39,   0x8400, 0,      0x39,   0x4900, 0x39,           0x4900},
  {       0x2D,   0x2D,   0,      0,      0x2D,   0x2D,   0x2D,           0x2D},
  {       0x4B00, 0x34,   0x7300, 0,      0x34,   0x4B00, 0x34,           0x4B00},
  {       0x4C00, 0x35,   0,      0,      0x35,   0x4C00, 0x35,           0x4C00},
  {       0x4D00, 0x36,   0x7400, 0,      0x36,   0x4D00, 0x36,           0x4D00},
  {       0x2B,   0x2B,   0,      0,      0x2B,   0x2B,   0x2B,           0x2B},
  {       0x4F00, 0x31,   0x7500, 0,      0x31,   0x4F00, 0x31,           0x4F00},
  {       0x5000, 0x32,   0,      0,      0x32,   0x5000, 0x32,           0x5000},
  {       0x5100, 0x33,   0x7600, 0,      0x33,   0x5100, 0x33,           0x5100},
  {       0x5200, 0x30,   0,      0,      0x30,   0x5200, 0x30,           0x5200},
  {       0x5300, 0x2E,   0,      0,      0x2E,   0x5300, 0x2E,           0x5300}
};

#define MAXSCANCODE (sizeof(scan2ascii) / (8 * sizeof(int)))

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

  last_key = -1;

  //kprintf("scancode %08X\n", key);

  if (keymap == -1)
  {
    insert_key((unsigned char) key);
  }

  // Ctrl-Alt-Del
  if ((control_keys & (CK_CTRL | CK_ALT)) && key == 0x53) 
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

  // Ctrl key
  if (key == 0x1D && !(control_keys & CK_CTRL)) control_keys |= CK_CTRL;
  if (key == 0x80 + 0x1D) control_keys &= ~CK_CTRL;

  // Shift key
  if ((key == 0x2A || key == 0x36) && !(control_keys & CK_SHIFT)) control_keys |= CK_SHIFT;
  if ((key == 0x80 + 0x2A) || (key == 0x80 + 0x36)) control_keys &= ~CK_SHIFT;

  // Alt key
  if (key == 0x38 && (!control_keys & CK_ALT)) control_keys |= CK_ALT;
  if (key == 0x80 + 0x38) control_keys &= ~CK_ALT;
	  
  if (key < MAXSCANCODE)
  {
    if ((control_keys & CK_SHIFT) && (led_status & LED_CAPS_LOCK))
      key_ascii = scan2ascii[key][6]; 
    else if (control_keys & CK_SHIFT)
      key_ascii = scan2ascii[key][1];
    else if (control_keys & CK_CTRL) 
      key_ascii = scan2ascii[key][2];
    else if (control_keys & CK_ALT) 
      key_ascii = scan2ascii[key][3];	
    else if ((control_keys & CK_SHIFT) && (led_status & LED_NUM_LOCK))
      key_ascii = scan2ascii[key][7];
    else if (led_status & LED_CAPS_LOCK) 
      key_ascii = scan2ascii[key][5];
    else if (led_status & LED_NUM_LOCK) 
      key_ascii = scan2ascii[key][4];
    else if(control_keys == 0) 
      key_ascii = scan2ascii[key][0];

    if (key_ascii != 0)
    {
      if (key_ascii <= 0xFF)
      {
	insert_key((unsigned char) key_ascii);
      }
      else
      {
	insert_key((unsigned char) (key_ascii >> 8));
	insert_key((unsigned char) (key_ascii & 0xFF));
      }
    }
  }
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
  // Set all keyboard LEDs off
  led_status = 0; 
  setleds();

  // Setup keyboard interrupt handler
  init_dpc(&kbddpc);
  init_sem(&kbdsem, 0);
  set_interrupt_handler(INTR_KBD, keyboard_handler, NULL);
  enable_irq(IRQ_KBD);
}
