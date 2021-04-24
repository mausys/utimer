#include <stddef.h>
#include "utimer.h"


#ifndef UTIMER_MAX_TIMERS
#define UTIMER_MAX_TIMERS 16
#endif

#define MAX_TIMERS UTIMER_MAX_TIMERS


typedef struct utimer
{
  struct {
    utimer_t *next;
    utimer_t **prev;
  } list;
  ticks_t timeout;
  ticks_t interval;
  utimer_fn callback;
  void *ud;
} utimer_t;


typedef struct {
  utimer_t *active;
  ticks_t now;
  utimer_t timers[MAX_TIMERS];
} utimer_scheduler_t;


static utimer_scheduler_t m_scheduler;
static ticks_t *m_ticks = &m_scheduler.now;


static utimer_scheduler_t* utimer_scheduler_get(void)
{
  return &m_scheduler;
}


static void utimer_construct(utimer_t* timer, utimer_fn callback, void *ud)
{
  timer->list.prev = NULL;
  timer->interval = 0;
  timer->callback = callback;
  timer->ud = ud;
}


static void deactivate(utimer_t* timer)
{
  if (timer->list.next != NULL)
    timer->list.next->list.prev = timer->list.prev;

  *timer->list.prev = timer->list.next;
  timer->list.prev = NULL;
}


static void start(utimer_scheduler_t* scheduler, utimer_t* timer, ticks_t countdown)
{
  timer->timeout = scheduler->now + countdown;

  if (scheduler->active == NULL) {
    timer->list.next = NULL;
    scheduler->active = timer;
    timer->list.prev = &scheduler->active;
    return;
  }

  for (utimer_t *iter = scheduler->active; iter != NULL; iter = iter->list.next) {
    dticks_t d = iter->timeout - scheduler->now;

    if (d < 0)
      d = 0;

    if (d > (dticks_t)countdown) {
      timer->list.prev = iter->list.prev;
      timer->list.next = iter;
      *iter->list.prev = timer;
      iter->list.prev = &timer->list.next;
      break;
    } else if (iter->list.next == NULL) {
      timer->list.next = NULL;
      iter->list.next = timer;
      timer->list.prev = &iter->list.next;
      break;
    }
  }
}


ticks_t ticks_now(void)
{
  return *m_ticks;
}


ticks_t utimer_schedule(ticks_t now)
{
  utimer_scheduler_t *scheduler = utimer_scheduler_get();
  scheduler->now = now;

  utimer_t *timer = scheduler->active;

  while (timer != NULL) {
    dticks_t d = timer->timeout - scheduler->now;

    if (d > 0) // still running
      return d;

    ticks_t actual = timer->timeout;

    deactivate(timer);

    timer->callback(timer->ud);

    // check if callback function has already set new periodic timer
    if ((timer->interval != 0) && (timer->list.prev == NULL)) {
      dticks_t adjust = actual + timer->interval - scheduler->now;

      // maybe we already missed an interval,
      // so set countdown to next possible time
      if (adjust <= 0)
        adjust = 1;

      start(scheduler, timer, adjust);
    }

    timer = scheduler->active;
  }
  return 0;
}


utimer_t* utimer_new(utimer_fn callback, void *ud)
{
  if (callback == NULL)
    return NULL;

  utimer_scheduler_t *scheduler =  utimer_scheduler_get();

  for (int i = 0; i < MAX_TIMERS; i++) {
    utimer_t *timer = &scheduler->timers[i];
    if (timer->callback == NULL) {
      utimer_construct(timer, callback, ud);
      return timer;
    }
  }
  return NULL;
}


void utimer_delete(utimer_t* timer)
{
  utimer_stop(timer);
  timer->callback = NULL;
}


bool utimer_active(const utimer_t* timer)
{
  return timer->list.prev != NULL;
}


void utimer_start(utimer_t* timer, ticks_t countdown)
{
  utimer_stop(timer);

  utimer_scheduler_t *scheduler = utimer_scheduler_get();
  start(scheduler, timer, countdown);

  timer->interval = 0;
}


void utimer_start_periodic(utimer_t* timer, ticks_t interval)
{
  utimer_stop(timer);

  utimer_scheduler_t *scheduler = utimer_scheduler_get();
  start(scheduler, timer, interval);

  timer->interval = interval;
}


void utimer_stop(utimer_t* timer)
{
  if (!utimer_active(timer))
    return;

  deactivate(timer);
}
