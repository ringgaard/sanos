#include <os.h>

extern handle_t module;

void print_banner()
{
  syslog(LOG_DEBUG, "hello from test\n");
}
