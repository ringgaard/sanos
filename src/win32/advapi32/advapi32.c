//
// advapi32.c
//
// Win32 ADVAPI32 emulation
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
#include <sys/types.h>
#include <win32.h>
#include <string.h>
#include <inifile.h>

BOOL WINAPI GetUserNameA(
    LPTSTR lpBuffer,
    LPDWORD nSize) {
  struct passwd *pwd;

  TRACE("GetUserNameA");
  pwd = getpwuid(getuid());
  if (!pwd) return FALSE;
  strncpy(lpBuffer, pwd->pw_name, *nSize);
  return TRUE;
}

LONG WINAPI RegCloseKey(
    HKEY hKey) {
  TRACE("RegCloseKey");
  syslog(LOG_DEBUG, "warning: RegCloseKey not implemented, ignored");
  return -1;
}

LONG WINAPI RegEnumKeyExA(
    HKEY hKey,
    DWORD dwIndex,
    LPTSTR lpName,
    LPDWORD lpcName,
    LPDWORD lpReserved,
    LPTSTR lpClass,
    LPDWORD lpcClass,
    PFILETIME lpftLastWriteTime) {
  TRACE("RegEnumKeyExA");
  syslog(LOG_DEBUG, "warning: RegEnumKeyExA ignored");
  return -1;
}

LONG WINAPI RegOpenKeyExA(
    HKEY hKey,
    LPCTSTR lpSubKey,
    DWORD ulOptions,
    REGSAM samDesired,
    PHKEY phkResult) {
  TRACE("RegOpenKeyExA");
  //syslog(LOG_DEBUG, "warning: RegOpenKeyEx(%p,%s) ignored", hKey, lpSubKey);
  return -1;
}

LONG WINAPI RegCreateKeyExA(
    HKEY hKey,
    LPCSTR lpSubKey,
    DWORD Reserved,
    LPSTR lpClass,
    DWORD dwOptions,
    REGSAM samDesired,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY phkResult,
    LPDWORD lpdwDisposition) {
  TRACE("RegCreateKeyExA");
  syslog(LOG_DEBUG, "warning: RegCreateKeyExA(%p,%s) ignored", hKey, lpSubKey);
  return -1;
}

LONG WINAPI RegEnumValueA(
    HKEY hKey,
    DWORD dwIndex,
    LPSTR lpValueName,
    LPDWORD lpcbValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData) {
  TRACE("RegEnumValueA");
  syslog(LOG_DEBUG, "warning: RegEnumValueA ignored");
  return -1;
}

LONG WINAPI RegDeleteValueA(
    HKEY hKey,
    LPCSTR lpValueName) {
  TRACE("RegDeleteValueA");
  syslog(LOG_DEBUG, "warning: RegDeleteValueA(%p,%s) ignored", hKey, lpValueName);
  return -1;
}

LONG WINAPI RegSetValueExA(
    HKEY hKey,
    LPCSTR lpValueName,
    DWORD Reserved,
    DWORD dwType,
    CONST BYTE *lpData,
    DWORD cbData) {
  TRACE("RegSetValueExA");
  syslog(LOG_DEBUG, "warning: RegSetValueExA(%p,%s) ignored", hKey, lpValueName);
  return -1;
}

LONG WINAPI RegFlushKey(
    HKEY hKey
  ) {
  TRACE("RegFlushKey");
  syslog(LOG_DEBUG, "warning: RegFlushKey ignored");
  return -1;
}

LONG WINAPI RegDeleteKeyA(
    HKEY hKey,
    LPCSTR lpSubKey) {
  TRACE("RegDeleteKeyA");
  syslog(LOG_DEBUG, "warning: RegDeleteKeyA(%p,%s) ignored", hKey, lpSubKey);
  return -1;
}

LONG WINAPI RegQueryInfoKeyA(
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
    PFILETIME lpftLastWriteTime) {
  TRACE("RegQueryInfoKeyA");
  syslog(LOG_DEBUG, "warning: RegQueryInfoKeyA ignored");
  return -1;
}

LONG WINAPI RegQueryValueExA(
    HKEY hKey,
    LPCTSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData) {
  TRACE("RegQueryValueExA");
  syslog(LOG_DEBUG, "warning: RegQueryValueExA ignored");
  return -1;
}

LONG WINAPI RegQueryValueExW(
    HKEY hKey,
    LPCWSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData) {
  TRACE("RegQueryValueExW");
  syslog(LOG_DEBUG, "warning: RegQueryValueExW ignored");
  return -1;
}

LONG WINAPI RegOpenKeyExW(
    HKEY hKey,
    LPCWSTR lpSubKey,
    DWORD ulOptions,
    REGSAM samDesired,
    PHKEY phkResult) {
  TRACE("RegOpenKeyExW");
  syslog(LOG_DEBUG, "warning: RegOpenKeyExW ignored");
  return -1;
}

HANDLE WINAPI RegisterEventSourceA(
    LPCTSTR lpUNCServerName,
    LPCTSTR lpSourceName) {
  TRACE("RegisterEventSourceA");
  return strdup(lpSourceName);
}

BOOL WINAPI DeregisterEventSource(
    HANDLE hEventLog) {
  TRACE("DeregisterEventSource");
  free(hEventLog);
  return TRUE;
}

BOOL WINAPI ReportEventA(
    HANDLE hEventLog,
    WORD wType,
    WORD wCategory,
    DWORD dwEventID,
    PSID lpUserSid,
    WORD wNumStrings,
    DWORD dwDataSize,
    LPCTSTR *lpStrings,
    LPVOID lpRawData) {
  int level;

  TRACE("ReportEventA");

  switch (wType) {
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

  if (wNumStrings > 0) {
    syslog(level, "%s: %s", hEventLog, lpStrings[0]);
  } else {
    syslog(level, "%s: message #%d", hEventLog, dwEventID);
  }

  return TRUE;
}

BOOL WINAPI CryptAcquireContextA(
    HCRYPTPROV *phProv,
    LPCTSTR pszContainer,
    LPCTSTR pszProvider,
    DWORD dwProvType,
    DWORD dwFlags) {
  TRACE("CryptAcquireContextA");
  panic("CryptAcquireContextA not implemented");
  return TRUE;
}

BOOL WINAPI CryptReleaseContext(
    HCRYPTPROV hProv, 
    DWORD dwFlags) {
  TRACE("CryptReleaseContext");
  panic("CryptReleaseContext not implemented");
  return TRUE;
}

BOOL WINAPI CryptGenRandom(
    HCRYPTPROV hProv, 
    DWORD dwLen, 
    BYTE *pbBuffer) {
  TRACE("CryptGenRandom");
  panic("CryptGenRandom not implemented");
  return TRUE;
}

BOOL WINAPI StartServiceCtrlDispatcherA(
    CONST LPSERVICE_TABLE_ENTRY lpServiceTable) {
  TRACE("StartServiceCtrlDispatcherA");
  panic("StartServiceCtrlDispatcherA not implemented");
  return TRUE;
}

BOOL WINAPI SetServiceStatus(
    SERVICE_STATUS_HANDLE hServiceStatus,
    LPSERVICE_STATUS lpServiceStatus) {
  TRACE("SetServiceStatus");
  panic("SetServiceStatus not implemented");
  return TRUE;
}

SERVICE_STATUS_HANDLE WINAPI RegisterServiceCtrlHandlerA(
    LPCTSTR lpServiceName,
    LPHANDLER_FUNCTION lpHandlerProc) {
  TRACE("RegisterServiceCtrlHandlerA");
  panic("RegisterServiceCtrlHandlerA not implemented");
  return 0;
}

BOOL WINAPI AllocateAndInitializeSid(
    PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority,
    BYTE nSubAuthorityCount,
    DWORD dwSubAuthority0,
    DWORD dwSubAuthority1,
    DWORD dwSubAuthority2,
    DWORD dwSubAuthority3,
    DWORD dwSubAuthority4,
    DWORD dwSubAuthority5,
    DWORD dwSubAuthority6,
    DWORD dwSubAuthority7,
    PSID *pSid) {
  TRACE("AllocateAndInitializeSid");
  panic("AllocateAndInitializeSid not implemented");
  return FALSE;
}


BOOL WINAPI AddAccessAllowedAce(
    PACL pAcl,
    DWORD dwAceRevision,
    DWORD AccessMask,
    PSID pSid) {
  TRACE("AddAccessAllowedAce");
  panic("AddAccessAllowedAce not implemented");
  return FALSE;
}

BOOL WINAPI CopySid(
    DWORD nDestinationSidLength,
    PSID pDestinationSid,
    PSID pSourceSid) {
  TRACE("CopySid");
  panic("CopySid not implemented");
  return FALSE;
}

PVOID WINAPI FreeSid(
    PSID pSid) {
  TRACE("FreeSid");
  panic("FreeSid not implemented");
  return NULL;
}

DWORD WINAPI GetLengthSid(
    PSID pSid) {
  TRACE("GetLengthSid");
  panic("GetLengthSid not implemented");
  return 0;
}

BOOL WINAPI GetSecurityDescriptorDacl(
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    LPBOOL lpbDaclPresent,
    PACL *pDacl,
    LPBOOL lpbDaclDefaulted) {
  TRACE("GetSecurityDescriptorDacl");
  panic("GetSecurityDescriptorDacl not implemented");
  return FALSE;
}

BOOL WINAPI OpenProcessToken(
    HANDLE ProcessHandle,
    DWORD DesiredAccess,
    PHANDLE TokenHandle) {
  TRACE("OpenProcessToken");
  panic("OpenProcessToken not implemented");
  return FALSE;
}

BOOL WINAPI GetTokenInformation(
    HANDLE TokenHandle,
    TOKEN_INFORMATION_CLASS TokenInformationClass,
    LPVOID TokenInformation,
    DWORD TokenInformationLength,
    PDWORD ReturnLength) {
  TRACE("GetTokenInformation");
  panic("GetTokenInformation not implemented");
  return FALSE;
}

BOOL WINAPI InitializeAcl(
    PACL pAcl,
    DWORD nAclLength,
    DWORD dwAclRevision) {
  TRACE("InitializeAcl");
  panic("InitializeAcl not implemented");
  return FALSE;
}

BOOL WINAPI InitializeSecurityDescriptor(
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    DWORD dwRevision) {
  TRACE("InitializeSecurityDescriptor");
  panic("InitializeSecurityDescriptor not implemented");
  return FALSE;
}

BOOL WINAPI SetSecurityDescriptorDacl(
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    BOOL bDaclPresent,
    PACL pDacl,
    BOOL bDaclDefaulted) {
  TRACE("SetSecurityDescriptorDacl");
  panic("SetSecurityDescriptorDacl not implemented");
  return FALSE;
}

int __stdcall DllMain(handle_t hmod, int reason, void *reserved) {
  return TRUE;
}
