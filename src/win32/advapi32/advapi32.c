//
// winmm.c
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Win32 ADVAPI32 emulation
//

#include <os.h>
#include <win32.h>
#include <string.h>
#include <inifile.h>

BOOL WINAPI GetUserNameA
(
  LPTSTR lpBuffer,
  LPDWORD nSize
)
{
  TRACE("GetUserNameA");
  strcpy(lpBuffer, get_property(config, "os", "user", "root"));
  return TRUE;
}

LONG WINAPI RegCloseKey
(
  HKEY hKey
)
{
  TRACE("RegCloseKey");
  panic("RegCloseKey not implemented");
  return -1;
}

LONG WINAPI RegEnumKeyExA
(
  HKEY hKey,
  DWORD dwIndex,
  LPTSTR lpName,
  LPDWORD lpcName,
  LPDWORD lpReserved,
  LPTSTR lpClass,
  LPDWORD lpcClass,
  PFILETIME lpftLastWriteTime
)
{
  TRACE("RegEnumKeyExA");
  panic("RegEnumKeyExA not implemented");
  return -1;
}

LONG WINAPI RegOpenKeyExA
(
  HKEY hKey,
  LPCTSTR lpSubKey,
  DWORD ulOptions,
  REGSAM samDesired,
  PHKEY phkResult
)
{
  TRACE("RegOpenKeyExA");
  syslog(LOG_DEBUG, "warning: RegOpenKeyEx(%p,%s) ignored\n", hKey, lpSubKey);
  return -1;
}

LONG WINAPI RegQueryInfoKeyA
(
  HKEY hKey,
  LPTSTR lpClass,
  LPDWORD lpcClass,
  LPDWORD lpReserved,
  LPDWORD lpcSubKeys,
  LPDWORD lpcMaxSubKeyLen,
  LPDWORD lpcMaxClassLen,
  LPDWORD lpcValues,
  LPDWORD lpcMaxValueNameLen,
  LPDWORD lpcMaxValueLen,
  LPDWORD lpcbSecurityDescriptor,
  PFILETIME lpftLastWriteTime
)
{
  TRACE("RegQueryInfoKeyA");
  panic("RegQueryInfoKeyA not implemented");
  return -1;
}

LONG WINAPI RegQueryValueExA
(
  HKEY hKey,
  LPCTSTR lpValueName,
  LPDWORD lpReserved,
  LPDWORD lpType,
  LPBYTE lpData,
  LPDWORD lpcbData
)
{
  TRACE("RegQueryValueExA");
  panic("RegQueryValueExA not implemented");
  return -1;
}

int __stdcall DllMain(handle_t hmod, int reason, void *reserved)
{
  return TRUE;
}
