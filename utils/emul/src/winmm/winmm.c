#include <os.h>
#include <win32.h>

MMRESULT WINAPI timeBeginPeriod
(
  UINT uPeriod  
)
{
  syslog(LOG_DEBUG, "timeBeginPeriod: timer resolution is %d\n", uPeriod);
  return 0;
}

MMRESULT WINAPI timeEndPeriod
(
  UINT uPeriod  
)
{
  syslog(LOG_DEBUG, "timeEndPeriod: timer resolution is %d\n", uPeriod);
  return 0;
}

DWORD timeGetTime(VOID)
{
  return get_tick_count();
}

int __stdcall DllMain(handle_t hmod, int reason, void *reserved)
{
  return TRUE;
}
