#include <os.h>
[!if USE_CLIB]
#include <stdio.h>
[!endif]

[!if APP_TYPE_USERDLL]

int __stdcall DllMain(hmodule_t hmod, int reason, void *reserved)
{
  return 1;
}
[!endif]

[!if APP_TYPE_USEREXE]

int main(int argc, char *argv[])
{
	// TODO: Add code here
	return 0;
}
[!endif]
