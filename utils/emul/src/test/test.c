#include <os.h>

hmodule_t module;

void print_banner();

__declspec(dllimport) int add(int a, int b);
__declspec(dllimport) int printf(const char *fmt, ...);

int __stdcall main(hmodule_t hmod, char *cmdline, int reserved)
{
  handle_t f;
  int i;

  module = hmod;

  print_banner();
  printf("2+3=%d\n", add(2, 3));
  printf("cmdline: %s\n", cmdline);

  f = open_file("test.txt", O_CREAT | O_WRONLY);
  for (i = 0; i < 10; i++)
  {
    write_file(f, "hello world\n", 12);
  }
  close_handle(f);

  return 0;
}
