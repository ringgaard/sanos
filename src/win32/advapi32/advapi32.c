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

HANDLE WINAPI RegisterEventSourceA
(
  LPCTSTR lpUNCServerName,
  LPCTSTR lpSourceName
)
{
  TRACE("RegisterEventSourceA");
  return strdup(lpSourceName);
}

BOOL WINAPI DeregisterEventSource
(
  HANDLE hEventLog
)
{
  TRACE("DeregisterEventSource");
  free(hEventLog);
  return TRUE;
}

BOOL WINAPI ReportEventA
(
  HANDLE hEventLog,
  WORD wType,
  WORD wCategory,
  DWORD dwEventID,
  PSID lpUserSid,
  WORD wNumStrings,
  DWORD dwDataSize,
  LPCTSTR *lpStrings,
  LPVOID lpRawData
)
{
  int level;

  TRACE("ReportEventA");

  switch (wType)
  {
    case EVENTLOG_SUCCESS:
      level = LOG_NOTICE;
      break;

    case EVENTLOG_ERROR_TYPE:
      level = LOG_ERR;
      break;

    case EVENTLOG_WARNING_TYPE:
      level = LOG_WARNING;
      break;

    case EVENTLOG_INFORMATION_TYPE:
      level = LOG_INFO;
      break;

    case EVENTLOG_AUDIT_SUCCESS:
      level = LOG_INFO;
      break;

    case EVENTLOG_AUDIT_FAILURE:
      level = LOG_WARNING;
      break;

    default:
      level = LOG_DEBUG;
  }

  if (wNumStrings > 0)
    syslog(level, "%s: %s\n", hEventLog, lpStrings[0]);
  else
    syslog(level, "%s: message #%d\n", hEventLog, dwEventID);

  return TRUE;
}

BOOL WINAPI CryptAcquireContextA
(
  HCRYPTPROV *phProv,
  LPCTSTR pszContainer,
  LPCTSTR pszProvider,
  DWORD dwProvType,
  DWORD dwFlags
)
{
  TRACE("CryptAcquireContextA");
  panic("CryptAcquireContextA not implemented");
  return TRUE;
}

BOOL WINAPI CryptReleaseContext
(
  HCRYPTPROV hProv, 
  DWORD dwFlags 
)
{
  TRACE("CryptReleaseContext");
  panic("CryptReleaseContext not implemented");
  return TRUE;
}

BOOL WINAPI CryptGenRandom
(
  HCRYPTPROV hProv, 
  DWORD dwLen, 
  BYTE *pbBuffer 
)
{
  TRACE("CryptGenRandom");
  panic("CryptGenRandom not implemented");
  return TRUE;
}

int __stdcall DllMain(handle_t hmod, int reason, void *reserved)
{
  return TRUE;
}
