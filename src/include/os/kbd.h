//
// keyboard.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Keyboard driver
//

#ifndef KEYBOARD_H
#define KEYBOARD_H

#define IOCTL_SET_KEYTIMEOUT     1024
#define IOCTL_CTRLALTDEL_ENABLED 1025
#define IOCTL_SET_KEYMAP         1026

extern int ctrl_alt_del_enabled;
extern int keymap;

void init_keyboard();

int getch(unsigned int timeout);
int kbhit();
void reboot();

#endif
