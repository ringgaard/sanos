#include "msvcrt.h"

time_t time(time_t *timer)
{
  return (time_t) get_time();
}
