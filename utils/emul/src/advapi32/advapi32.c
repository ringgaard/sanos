#include <os.h>
#include <win32.h>

#define USERNAME "root"

BOOL WINAPI GetUserNameA
(
  LPTSTR lpBuffer,
  LPDWORD nSize
)
{
  strcpy(lpBuffer, USERNAME);
  return TRUE;
}

LONG WINAPI RegCloseKey
(
  HKEY hKey
)
{
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
  syslog(LOG_DEBUG, "warning: RegOpenKeyEx(%p,%s) ignored\n", hKey, lpSubKey);
  //panic("RegOpenKeyExA not implemented");
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
  panic("RegQueryValueExA not implemented");
  return -1;
}

int __stdcall DllMain(handle_t hmod, int reason, void *reserved)
{
  return TRUE;
}
