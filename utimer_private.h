#pragma once

#include "utimer.h"


struct utimer
{
  struct {
    utimer_t *next;
    utimer_t **prev;
  } list;
  ticks_t timeout;
  ticks_t interval;
  bool oneshot;
  utimer_fn callback;
  void *ud;
};

utimer_t* utimer_alloc(void);
void utimer_free(utimer_t* timer);


typedef struct utimer_scheduler {
  utimer_t *active;
  ticks_t now;
} utimer_scheduler_t;

utimer_t* utimer_active_list(void);

utimer_scheduler_t* utimer_scheduler_instance(void);
