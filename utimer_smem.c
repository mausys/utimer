#include "utimer_private.h"

#include <stddef.h>

#ifndef UTIMER_MAX
#define UTIMER_MAX 16
#endif

static utimer_t m_timers[UTIMER_MAX];

utimer_t* utimer_alloc(void)
{
  for (int i = 0; i < UTIMER_MAX; i++) {
    utimer_t *timer = &m_timers[i];
    if (timer->callback == NULL) {
      return timer;
    }
  }
  return NULL;
}


void utimer_free(utimer_t* timer)
{
  timer->callback = NULL;
}
