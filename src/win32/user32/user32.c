//
// user32.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Win32 USER32 emulation
//

#include <os.h>
#include <win32.h>

int WINAPI MessageBoxA
(
  HWND hWnd,
  LPCTSTR lpText,
  LPCTSTR lpCaption,
  UINT uType
)
{
  TRACE("MessageBoxA");
  panic("MessageBoxA not implemented");
  return 0;
}

int __stdcall DllMain(handle_t hmod, int reason, void *reserved)
{
  return TRUE;
}
