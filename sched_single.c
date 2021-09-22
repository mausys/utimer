#include "utimer_private.h"

static utimer_scheduler_t m_scheduler;


utimer_scheduler_t* utimer_scheduler_instance(void)
{
  return &m_scheduler;
}


