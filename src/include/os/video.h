//
// video.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Video funtions
//

#ifndef VIDEO_H
#define VIDEO_H

void init_video();

void print_buffer(const char *str, int len);
void print_char(int ch);
void print_string(const char *str);

void show_cursor();
void hide_cursor();
void set_cursor(int x, int y);

void clear_screen();

#endif
