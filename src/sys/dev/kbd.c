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
#define CK_ALTGR	        8

#define KBSTATE_NORMAL          0
#define KBSTATE_SHIFT           1
#define KBSTATE_CTRL            2
#define KBSTATE_ALT             3
#define KBSTATE_NUMLOCK         4
#define KBSTATE_CAPSLOCK        5
#define KBSTATE_SHIFTCAPS       6
#define KBSTATE_SHIFTNUM        7
#define KBSTATE_ALTGR           8

#define MAX_KBSTATES            9
#define MAX_SCANCODES           0x80
#define MAX_KEYTABLES           2
 
#define KEYBOARD_BUFFER_SIZE    256

struct keytable
{
  unsigned short normal[MAX_SCANCODES][MAX_KBSTATES];
  unsigned short extended[MAX_SCANCODES][MAX_KBSTATES];
};

struct keytable *keytables[MAX_KEYTABLES];

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

// US keyboard table

struct keytable uskeys = 
{
  // Normal scancodes
  {
  // normal  shift   ctrl    alt     num     caps    scaps   snum    altgr

    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 00
    {0x1B,   0x1B,   0x1B,   0,      0x1B,   0x1B,   0x1B,   0x1B,   0},   // 01 esc
    {0x31,   0x21,   0,      0x7800, 0x31,   0x31,   0x21,   0x21,   0},   // 02 1
    {0x32,   0x40,   0x0300, 0x7900, 0x32,   0x32,   0x40,   0x40,   0},   // 03 2
    {0x33,   0x23,   0,      0x7A00, 0x33,   0x33,   0x23,   0x23,   0},   // 04 3
    {0x34,   0x24,   0,      0x7B00, 0x34,   0x34,   0x24,   0x24,   0},   // 05 4
    {0x35,   0x25,   0,      0x7C00, 0x35,   0x35,   0x25,   0x25,   0},   // 06 5
    {0x36,   0x5E,   0x1E,   0x7D00, 0x36,   0x36,   0x5E,   0x5E,   0},   // 07 6
    {0x37,   0x26,   0,      0x7E00, 0x37,   0x37,   0x26,   0x26,   0},   // 08 7
    {0x38,   0x2A,   0,      0x7F00, 0x38,   0x38,   0x2A,   0x2A,   0},   // 09 8
    {0x39,   0x28,   0,      0x8000, 0x39,   0x39,   0x28,   0x28,   0},   // 0A 9
    {0x30,   0x29,   0,      0x8100, 0x30,   0x30,   0x29,   0x29,   0},   // 0B 0
    {0x2D,   0x5F,   0x1F,   0x8200, 0x2D,   0x2D,   0x5F,   0x5F,   0},   // 0C -
    {0x3D,   0x2B,   0,      0x8300, 0x3D,   0x3D,   0x2B,   0x2B,   0},   // 0D =
    {0x08,   0x08,   0x7F,   0,      0x08,   0x08,   0x08,   0x08,   0},   // 0E bksp
    {0x09,   0x0F00, 0,      0,      0x09,   0x09,   0x0F00, 0x0F00, 0},   // 0F tab
    {0x71,   0x51,   0x11,   0x1000, 0x71,   0x51,   0x71,   0x51,   0},   // 10 Q
    {0x77,   0x57,   0x17,   0x1100, 0x77,   0x57,   0x77,   0x57,   0},   // 11 W
    {0x65,   0x45,   0x05,   0x1200, 0x65,   0x45,   0x65,   0x45,   0},   // 12 E
    {0x72,   0x52,   0x12,   0x1300, 0x72,   0x52,   0x72,   0x52,   0},   // 13 R
    {0x74,   0x54,   0x14,   0x1400, 0x74,   0x54,   0x74,   0x54,   0},   // 14 T
    {0x79,   0x59,   0x19,   0x1500, 0x79,   0x59,   0x79,   0x59,   0},   // 15 Y
    {0x75,   0x55,   0x15,   0x1600, 0x75,   0x55,   0x75,   0x55,   0},   // 16 U
    {0x69,   0x49,   0x09,   0x1700, 0x69,   0x49,   0x69,   0x49,   0},   // 17 I
    {0x6F,   0x4F,   0x0F,   0x1800, 0x6F,   0x4F,   0x6F,   0x4F,   0},   // 18 O
    {0x70,   0x50,   0x10,   0x1900, 0x70,   0x50,   0x70,   0x50,   0},   // 19 P
    {0x5B,   0x7B,   0x1B,   0x0,    0x5B,   0x5B,   0x7B,   0x7B,   0},   // 1A [
    {0x5D,   0x7D,   0x1D,   0,      0x5D,   0x5D,   0x7D,   0x7D,   0},   // 1B ]
    {0x0A,   0x0A,   0x0D,   0,      0x0A,   0x0A,   0x0D,   0x0D,   0},   // 1C enter
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 1D ctrl
    {0x61,   0x41,   0x01,   0x1E00, 0x61,   0x41,   0x61,   0x41,   0},   // 1E A
    {0x73,   0x53,   0x13,   0x1F00, 0x73,   0x53,   0x73,   0x53,   0},   // 1F S
    {0x64,   0x44,   0x04,   0x2000, 0x64,   0x44,   0x64,   0x44,   0},   // 20 D
    {0x66,   0x46,   0x06,   0x2100, 0x66,   0x46,   0x66,   0x46,   0},   // 21 F
    {0x67,   0x47,   0x07,   0x2200, 0x67,   0x47,   0x67,   0x47,   0},   // 22 G
    {0x68,   0x48,   0x08,   0x2300, 0x68,   0x48,   0x68,   0x48,   0},   // 23 H
    {0x6A,   0x4A,   0x0A,   0x2400, 0x6A,   0x4A,   0x6A,   0x4A,   0},   // 24 J
    {0x6B,   0x4B,   0x0B,   0x3500, 0x6B,   0x4B,   0x6B,   0x4B,   0},   // 25 K
    {0x6C,   0x4C,   0x0C,   0x2600, 0x6C,   0x4C,   0x6C,   0x4C,   0},   // 26 L
    {0x3B,   0x3A,   0,      0,      0x3B,   0x3B,   0x3A,   0x3A,   0},   // 27 ;
    {0x27,   0x22,   0,      0,      0x27,   0x27,   0x22,   0x22,   0},   // 28 '
    {0x60,   0x7E,   0,      0,      0x60,   0x60,   0x7E,   0x7E,   0},   // 29 ~
    {0x2A,   0,      0,      0,      0,      0,      0,      0,      0},   // 2A left shift
    {0x5C,   0x7C,   0x1C,   0,      0x5C,   0x5C,   0x7C,   0x7C,   0},   // 2B \ 
    {0x7A,   0x5A,   0x1A,   0x2C00, 0x7A,   0x5A,   0x7A,   0x5A,   0},   // 2C Z
    {0x78,   0x58,   0x18,   0x2D00, 0x78,   0x58,   0x78,   0x58,   0},   // 2D X
    {0x63,   0x43,   0x03,   0x2E00, 0x63,   0x43,   0x63,   0x43,   0},   // 2E C
    {0x76,   0x56,   0x16,   0x2F00, 0x76,   0x56,   0x76,   0x56,   0},   // 2F V
    {0x62,   0x42,   0x02,   0x3000, 0x62,   0x42,   0x62,   0x42,   0},   // 30 B
    {0x6E,   0x4E,   0x0E,   0x3100, 0x6E,   0x4E,   0x6E,   0x4E,   0},   // 31 N
    {0x6D,   0x4D,   0x0D,   0x3200, 0x6D,   0x4D,   0x6D,   0x4D,   0},   // 32 M
    {0x2C,   0x3C,   0,      0,      0x2C,   0x2C,   0x3C,   0x3C,   0},   // 33 ,
    {0x2E,   0x3E,   0,      0,      0x2E,   0x2E,   0x3E,   0x3E,   0},   // 34 .
    {0x2F,   0x3F,   0,      0,      0x2F,   0x2F,   0x3F,   0x3F,   0},   // 35 /
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 36 right shift
    {0x2A,   0x2A,   0,      0,      0x2A,   0x2A,   0,      0,      0},   // 37 * (kpad)
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 38 alt
    {0x20,   0x20,   0x20,   0,      0x20,   0x20,   0x20,   0x20,   0},   // 39 space
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 3A caps
    {0x3B00, 0x5400, 0x5E00, 0x6800, 0x3B00, 0x3B00, 0x5400, 0x5400, 0},   // 3B F1
    {0x3C00, 0x5500, 0x5F00, 0x6900, 0x3C00, 0x3C00, 0x5500, 0x5500, 0},   // 3C F2
    {0x3D00, 0x5600, 0x6000, 0x6A00, 0x3D00, 0x3D00, 0x5600, 0x5600, 0},   // 3D F3
    {0x3E00, 0x5700, 0x6100, 0x6B00, 0x3E00, 0x3E00, 0x5700, 0x5700, 0},   // 3E F4
    {0x3F00, 0x5800, 0x6200, 0x6C00, 0x3F00, 0x3F00, 0x5800, 0x5800, 0},   // 3F F5
    {0x4000, 0x5900, 0x6300, 0x6D00, 0x4000, 0x4000, 0x5900, 0x5900, 0},   // 40 F6
    {0x4100, 0x5A00, 0x6400, 0x6E00, 0x4100, 0x4100, 0x5A00, 0x5A00, 0},   // 41 F7
    {0x4200, 0x5B00, 0x6500, 0x6F00, 0x4200, 0x4200, 0x5B00, 0x5B00, 0},   // 42 F8
    {0x4300, 0x5C00, 0x6600, 0x7000, 0x4300, 0x4300, 0x5C00, 0x5C00, 0},   // 43 F9
    {0x4400, 0x5D00, 0x6700, 0x7100, 0x4400, 0x4400, 0x5D00, 0x5D00, 0},   // 44 F10
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 45 numlock
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 46 scrllock
    {0x4700, 0x37,   0x7700, 0,      0x37,   0x4700, 0x37,   0x4700, 0},   // 47 7 home (kpad)
    {0x4800, 0x38,   0,      0,      0x38,   0x4800, 0x38,   0x4800, 0},   // 48 8 up (kpad)
    {0x4900, 0x39,   0x8400, 0,      0x39,   0x4900, 0x39,   0x4900, 0},   // 49 9 pgup (kpad)
    {0x2D,   0x2D,   0,      0,      0x2D,   0x2D,   0x2D,   0x2D,   0},   // 4A - (kpad)
    {0x4B00, 0x34,   0x7300, 0,      0x34,   0x4B00, 0x34,   0x4B00, 0},   // 4B 4 left (kpad)
    {0x4C00, 0x35,   0,      0,      0x35,   0x4C00, 0x35,   0x4C00, 0},   // 4C 5 center (kpad)
    {0x4D00, 0x36,   0x7400, 0,      0x36,   0x4D00, 0x36,   0x4D00, 0},   // 4D 6 right (kpad)
    {0x2B,   0x2B,   0,      0,      0x2B,   0x2B,   0x2B,   0x2B,   0},   // 4E + (kpad)
    {0x4F00, 0x31,   0x7500, 0,      0x31,   0x4F00, 0x31,   0x4F00, 0},   // 4F 1 end (kpad)
    {0x5000, 0x32,   0,      0,      0x32,   0x5000, 0x32,   0x5000, 0},   // 50 2 down (kpad)
    {0x5100, 0x33,   0x7600, 0,      0x33,   0x5100, 0x33,   0x5100, 0},   // 51 3 pgdn (kpad)
    {0x5200, 0x30,   0,      0,      0x30,   0x5200, 0x30,   0x5200, 0},   // 52 0 ins (kpad)
    {0x5300, 0x2E,   0,      0,      0x2E,   0x5300, 0x2E,   0x5300, 0},   // 53 . del (kpad)
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 54 caps
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 55
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 56
    {0x5700, 0x5E00, 0x6800, 0x7200, 0x5700, 0x5700, 0x5E00, 0x5E00, 0},   // 57 F11
    {0x5800, 0x5F00, 0x6900, 0x7300, 0x5800, 0x5800, 0x5F00, 0x5F00, 0},   // 58 F12
  },

  // Extended scancodes
  {
  // normal  shift   ctrl    alt     num     caps    scaps   snum    altgr

    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 00
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 01
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 02
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 03
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 04
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 05
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 06
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 07
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 08
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 09
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 0A
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 0B
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 0C
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 0D
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 0E
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 0F
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 10
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 11
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 12
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 13
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 14
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 15
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 16
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 17
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 18
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 19
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 1A
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 1B
    {0x0A,   0x0A,   0x0D,   0,      0x0A,   0x0A,   0x0D,   0x0D,   0},   // 1C enter (kpad)
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 1D right ctrl
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 1E
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 1F
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 20
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 21
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 22
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 23
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 24
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 25
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 26
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 27
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 28
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 29
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 2A prtscr
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 2B
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 2C
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 2D
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 2E
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 2F
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 30
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 31
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 32
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 33
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 34
    {0x2F,   0x3F,   0,      0,      0x2F,   0x2F,   0x3F,   0x3F,   0},   // 35 / (kpad)
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 36
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 37 prtscr
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 38 altgr
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 39
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 3A
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 3B
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 3C
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 3D
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 3E
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 3F
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 40
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 41
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 42
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 43
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 44
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 45
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 46
    {0x4700, 0x4700, 0x7700, 0,      0x4700, 0x4700, 0x4700, 0x4700, 0},   // 47 home
    {0x4800, 0x4800, 0,      0,      0x4800, 0x4800, 0x4800, 0x4800, 0},   // 48 up
    {0x4900, 0x4900, 0x8400, 0,      0x4900, 0x4900, 0x4900, 0x4900, 0},   // 49 pgup
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 4A
    {0x4B00, 0x4B00, 0x7300, 0,      0x4B00, 0x4B00, 0x4B00, 0x4B00, 0},   // 4B left
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 4C
    {0x4D00, 0x4D00, 0x7400, 0,      0x4D00, 0x4D00, 0x4D00, 0x4D00, 0},   // 4D right
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 4E
    {0x4F00, 0x4F00, 0x7500, 0,      0x4F00, 0x4F00, 0x4F00, 0x4F00, 0},   // 4F end
    {0x5000, 0x5000, 0,      0,      0x5000, 0x5000, 0x5000, 0x5000, 0},   // 50 down
    {0x5100, 0x5100, 0x7600, 0,      0x5100, 0x5100, 0x5100, 0x5100, 0},   // 51 pgdn
    {0x5200, 0x5200, 0,      0,      0x5200, 0x5200, 0x5200, 0x5200, 0},   // 52 ins
    {0x5300, 0x5300, 0,      0,      0x5300, 0x5300, 0x5300, 0x5300, 0},   // 53 del
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 54 caps
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 55
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 56
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 57
    {0,      0,      0,      0,      0,      0,      0,      0,      0},   // 58
  }
};

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

  kprintf("scancode %02X %s%s\n", key & 0x7F, key & 0x80 ? "break" : "make", ext ? " ext" : "");

  // Ctrl-Alt-SysRq
  if ((control_keys & (CK_CTRL | CK_ALT)) && key == 0xD4) 
  {
    dbg_break();
    return;
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
  if (key == 0x1D) control_keys |= CK_CTRL;
  if (key == (0x1D | 0x80)) control_keys &= ~CK_CTRL;

  // Shift key
  if (key == 0x2A || key == 0x36) control_keys |= CK_SHIFT;
  if (key == (0x2A | 0x80) || key == (0x36 | 0x80)) control_keys &= ~CK_SHIFT;

  // Alt key
  if (key == 0x38) control_keys |= CK_ALT;
  if (key == 0x80 + 0x38) control_keys &= ~CK_ALT;
	  
  // AltGr key
  if (ext && key == 0x38) control_keys |= CK_ALTGR;
  if (ext && key == (0x38 | 0x80)) control_keys &= ~CK_ALTGR;

  if (key < MAX_SCANCODES)
  {
    if (control_keys & CK_ALTGR)
      state = KBSTATE_ALTGR;
    else if ((control_keys & CK_SHIFT) && (led_status & LED_CAPS_LOCK))
      state = KBSTATE_SHIFTCAPS;
    else if (control_keys & CK_SHIFT)
      state = KBSTATE_SHIFT;
    else if (control_keys & CK_CTRL) 
      state = KBSTATE_CTRL;
    else if (control_keys & CK_ALT) 
      state = KBSTATE_ALT;
    else if ((control_keys & CK_SHIFT) && (led_status & LED_NUM_LOCK))
      state = KBSTATE_SHIFTNUM;
    else if (led_status & LED_CAPS_LOCK) 
      state = KBSTATE_CAPSLOCK;
    else if (led_status & LED_NUM_LOCK) 
      state = KBSTATE_NUMLOCK;
    else if (control_keys == 0) 
      state = KBSTATE_NORMAL;

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

  if (key != 0xE0) ext = 0;
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
  // Initialize keyboard tables
  keytables[0] = &uskeys;

  // Set all keyboard LEDs off
  led_status = 0; 
  setleds();

  // Setup keyboard interrupt handler
  init_dpc(&kbddpc);
  init_sem(&kbdsem, 0);
  set_interrupt_handler(INTR_KBD, keyboard_handler, NULL);
  enable_irq(IRQ_KBD);
}
