//
// kbdext.h
//
// Keyboard table for extended scancodes
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

unsigned short extkeys[MAX_SCANCODES][MAX_KBSTATES] = {
// normal  shift   ctrl    alt     num     caps    scaps   snum    altgr
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 00
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 01
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 02
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 03
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 04
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 05
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 06
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 07
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 08
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 09
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 0A
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 0B
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 0C
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 0D
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 0E
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 0F
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 10
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 11
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 12
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 13
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 14
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 15
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 16
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 17
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 18
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 19
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 1A
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 1B
  {0x0D,   0x0D,   0x0A,   0xA600, 0x0D,   0x0D,   0x0A,   0x0A,   0},       // 1C enter (kpad)
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 1D right ctrl
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 1E
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 1F
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 20
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 21
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 22
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 23
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 24
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 25
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 26
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 27
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 28
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 29
  {0x0500, 0,      0x7200, 0,      0,      0,      0,      0,      0},       // 2A prtscr
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 2B
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 2C
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 2D
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 2E
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 2F
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 30
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 31
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 32
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 33
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 34
  {0x2F,   0x3F,   0x9500, 0xA400, 0x2F,   0x2F,   0x3F,   0x3F,   0},       // 35 / (kpad)
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 36
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 37
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 38 right alt
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 39
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 3A
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 3B
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 3C
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 3D
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 3E
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 3F
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 40
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 41
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 42
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 43
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 44
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 45
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 46
  {0x4700, 0xB700, 0x7700, 0x9700, 0x4700, 0x4700, 0x4700, 0x4700, 0x4700, 0xD700},  // 47 home
  {0x4800, 0xB800, 0x8D00, 0x9800, 0x4800, 0x4800, 0x4800, 0x4800, 0x4800, 0xD800},  // 48 up
  {0x4900, 0xB900, 0x8400, 0x9900, 0x4900, 0x4900, 0x4900, 0x4900, 0x4900, 0xD900},  // 49 pgup
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 4A
  {0x4B00, 0xBB00, 0x7300, 0x9B00, 0x4B00, 0x4B00, 0x4B00, 0x4B00, 0x4B00, 0xDB00},  // 4B left
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 4C
  {0x4D00, 0xBD00, 0x7400, 0x9D00, 0x4D00, 0x4D00, 0x4D00, 0x4D00, 0x4D00, 0xDD00},  // 4D right
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 4E
  {0x4F00, 0xBF00, 0x7500, 0x9F00, 0x4F00, 0x4F00, 0x4F00, 0x4F00, 0x4F00, 0xDF00},  // 4F end
  {0x5000, 0xC000, 0x9100, 0xA000, 0x5000, 0x5000, 0x5000, 0x5000, 0x5000, 0xE000},  // 50 down
  {0x5100, 0xC100, 0x7600, 0xA100, 0x5100, 0x5100, 0x5100, 0x5100, 0x5100, 0xE100},  // 51 pgdn
  {0x5200, 0xC200, 0x9200, 0xA200, 0x5200, 0x5200, 0x5200, 0x5200, 0x5200, 0xE200},  // 52 ins
  {0x5300, 0xC300, 0x9300, 0xA300, 0x5300, 0x5300, 0x5300, 0x5300, 0x5300, 0xE300},  // 53 del
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 54
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 55
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 56
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 57
  {0,      0,      0,      0,      0,      0,      0,      0,      0},       // 58
};
