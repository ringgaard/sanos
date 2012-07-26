//
// rtttl.c
//
// Ringing Tones Text Transfer Language (RTTTL) player
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

#include <os.h>

int notefreq[12] = {4186, 4434, 4698, 4978, 5274, 5587, 5919, 6271, 6644, 7040, 7458, 7902};

int note2freq(int note) {
  return notefreq[note % 12] / (1 << (9 - (note / 12)));
}

void play(char *song) {
  char *p = song;
  int defdur = 4;
  int defscale = 6;
  int bpm = 63;
  int silence = 0;
  
  // Skip name
  while (*p && *p != ':') p++;
  if (!*p) return;
  p++;

  // Parse defaults
  while (*p) {
    char param;
    int value;

    while (*p == ' ') p++;
    if (!*p) return;
    if (*p == ':') break;

    param = *p++;
    if (*p != '=') return;
    
    p++;
    value = 0;
    while (*p >= '0' && *p <= '9') value = value * 10 + (*p++ - '0');

    switch (param) {
      case 'd': defdur = 32 / value; break;
      case 'o': defscale = value; break;
      case 'b': bpm = value; break;
    }

    while (*p == ' ') p++;
    if (*p == ',') p++;
  }
  p++;

  while (*p) {
    int note = -1;
    int scale = defscale;
    int dur = defdur;
    int ms;
    int freq;

    // Skip whitespace
    while (*p == ' ') p++;
    if (!*p) return;

    // Parse duration
    if (*p >= '0' && *p <= '9') {
      int value = 0;
      while (*p >= '0' && *p <= '9') value = value * 10 + (*p++ - '0');

      dur = 32 / value;
    }

    // Parse note
    switch (*p) {
      case 0: return;
      case 'C': case 'c': note = 0; break;
      case 'D': case 'd': note = 2; break;
      case 'E': case 'e': note = 4; break;
      case 'F': case 'f': note = 5; break;
      case 'G': case 'g': note = 7; break;
      case 'A': case 'a': note = 9; break;
      case 'H': case 'h': note = 11; break;
      case 'B': case 'b': note = 11; break;
      case 'P': case 'p': note = -1; break;
    }
    p++;
    if (*p == '#') {
      note++;
      p++;
    }
    if (*p == 'b') {
      note--;
      p++;
    }

    // Parse special duration
    if (*p == '.') {
      dur += dur / 2;
      p++;
    }

    // Parse scale
    if (*p >= '0' && *p <= '9') scale = (*p++ - '0');

    // Parse special duration (again...)
    if (*p == '.') {
      dur += dur / 2;
      p++;
    }

    // Skip delimiter
    while (*p == ' ') p++;
    if (*p == ',') p++;

    // Play note
    ms = dur * 60000 / (bpm * 8);
    if (note == -1) {
      freq = 0;
    } else {
      freq = note2freq((scale + 1) * 12 + note);
    }

    if (freq) {
      ioctl(1, IOCTL_SOUND, &freq, 4);
      msleep(ms * 7 / 8);
      ioctl(1, IOCTL_SOUND, &silence, 4);
      msleep(ms / 8);
    } else {
      msleep(ms);
    }
  }
}
