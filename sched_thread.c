#include "utimer_private.h"

#include <stddef.h>
#include <stdlib.h>
#include <threads.h>


static thread_local utimer_scheduler_t *thread_scheduler = NULL;


static void scheduler_destructor(void *ud)
{
  if (thread_scheduler != NULL)
    free(thread_scheduler);

  thread_scheduler = NULL;
}


utimer_scheduler_t* utimer_scheduler_instance(void)
{
  if (thread_scheduler != NULL)
    return thread_scheduler;

  utimer_scheduler_t *scheduler = calloc(sizeof(utimer_scheduler_t), 1);

  if (scheduler == NULL)
    goto error_alloc;

  tss_t key;
  int r = tss_create(&key, scheduler_destructor);

  if (r != thrd_success)
    goto error_tss;

  r = tss_set(key, scheduler);

  if (r != thrd_success)
    goto error_tss;

  thread_scheduler = scheduler;

  return thread_scheduler;

error_tss:
  free(scheduler);
error_alloc:
  return NULL;
}



