#include "utimer_private.h"

#include <stddef.h>
#include <errno.h>




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



ticks_t utimer_schedule(ticks_t now)
{
  utimer_scheduler_t *scheduler = utimer_scheduler_instance();
  scheduler->now = now;

  utimer_t *timer = scheduler->active;

  while (timer != NULL) {
    dticks_t d = timer->timeout - scheduler->now;

    if (d > 0) // still running
      return d;

    ticks_t actual = timer->timeout;

    utimer_fn callback = timer->callback;
    void *ud = timer->ud;

    deactivate(timer);

    if (timer->oneshot) {
      utimer_free(timer);

      callback(ud);
    } else {
      callback(ud);

      if ((timer->interval != 0) && (timer->list.prev == NULL)) { // check if callback function has already set new periodic timer

      dticks_t adjust = actual + timer->interval - scheduler->now;

      // maybe we already missed an interval,
      // so set countdown to next possible time
      if (adjust <= 0)
        adjust = 1;

      start(scheduler, timer, adjust);
      }
    }

    timer = scheduler->active;
  }
  return 0;
}


utimer_t* utimer_new(utimer_fn callback, void *ud)
{
  if (callback == NULL)
    return NULL;

  utimer_t *timer = utimer_alloc();

  if (timer == NULL)
    return NULL;

  *timer = (utimer_t) {
    .callback = callback,
    .ud = ud,
  };

  return timer;
}


void utimer_delete(utimer_t* timer)
{
  utimer_stop(timer);
  utimer_free(timer);
}


bool utimer_active(const utimer_t* timer)
{
  return timer->list.prev != NULL;
}


void utimer_start(utimer_t* timer, ticks_t countdown)
{
  utimer_stop(timer);

  utimer_scheduler_t *scheduler = utimer_scheduler_instance();
  start(scheduler, timer, countdown);

  timer->interval = 0;
}


void utimer_start_periodic(utimer_t* timer, ticks_t interval)
{
  utimer_stop(timer);

  utimer_scheduler_t *scheduler = utimer_scheduler_instance();
  start(scheduler, timer, interval);

  timer->interval = interval;
}


void utimer_stop(utimer_t* timer)
{
  if (!utimer_active(timer))
    return;

  deactivate(timer);
}


int utimer_oneshot(ticks_t countdown, utimer_fn callback, void *ud)
{
  utimer_t *timer = utimer_new(callback, ud);

  if (timer == NULL)
    return -ENOMEM;

  timer->oneshot = true;
  utimer_start(timer, countdown);
  return 0;
}
