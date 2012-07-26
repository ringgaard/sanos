#include <os.h>

int mainCRTStartup() {
  write(fdout, "hello world\n", 12);
}
