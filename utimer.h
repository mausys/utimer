#pragma once

#include <stdbool.h>
#include "ticks.h"

typedef struct utimer utimer_t;

typedef void (*utimer_fn)(void*);


ticks_t utimer_schedule(ticks_t now);

utimer_t* utimer_new(utimer_fn callback, void *ud);
void utimer_delete(utimer_t* timer);

bool utimer_active(const utimer_t* timer);
void utimer_start(utimer_t* timer, ticks_t countdown);
void utimer_start_periodic(utimer_t* timer, ticks_t interval);
void utimer_stop(utimer_t* timer);
int utimer_oneshot(ticks_t countdown, utimer_fn callback, void *ud);

