#include "utimer_private.h"

#include <stdlib.h>

utimer_t* utimer_alloc(void)
{
  return malloc(sizeof(utimer_t));
}


void utimer_free(utimer_t* timer)
{
  free(timer);
}
