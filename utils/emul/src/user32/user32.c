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
  panic("MessageBoxA not implemented");
  return 0;
}

int __stdcall DllMain(handle_t hmod, int reason, void *reserved)
{
  return TRUE;
}
