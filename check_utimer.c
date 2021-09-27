#include "utimer.h"

#include <stdlib.h>
#include <stdint.h>
#include <check.h>

#define MAX_EVENTS 128
#define MAX_ACTIONS 128
#define MAX_TIMERS 16

typedef struct {
  enum {
    ACTION_LAST = 0,
    ACTION_NOOP,
    ACTION_START,
    ACTION_STOP,
  } type;
  utimer_t *timer;
  ticks_t ticks;
  bool next;
} action_t;

typedef struct {
  utimer_t *timer;
  ticks_t ticks;
} event_t;


static ticks_t m_ticks;

static struct {
  event_t events[MAX_EVENTS];
  unsigned index;
} m_event_list;

static struct {
  action_t actions[MAX_ACTIONS];
  unsigned index;
} m_action_list;

static utimer_t *m_timers[MAX_TIMERS];

static void callback(void *ud)
{
  utimer_t *timer = *(utimer_t**)ud;
  m_event_list.events[m_event_list.index++] = (event_t) { .ticks =  m_ticks, .timer = timer};

  bool next = true;
  while (next) {
    action_t *action = &m_action_list.actions[m_action_list.index];

    switch (action->type) {
      case ACTION_LAST:
        return;
      case ACTION_NOOP:
        break;
      case ACTION_START:
        utimer_start(action->timer, action->ticks);
        break;
      case ACTION_STOP:
        utimer_stop(action->timer);
        break;
    }
    next = action->next;
    m_action_list.index++;
  }
}

static void forward(ticks_t ticks)
{
  m_ticks += ticks;
  utimer_schedule(m_ticks);
}

void setup(void)
{
  m_ticks = UINT32_MAX - 20;
  utimer_schedule(m_ticks);

  for (int i = 0; i < MAX_TIMERS; i++) {
    m_timers[i] = utimer_new(callback, &m_timers[i]);
  }
}

void teardown(void)
{
  for (int i = 0; i < MAX_TIMERS; i++) {
    utimer_delete(m_timers[i]);
  }
}

START_TEST(test_utimer_single)
{
  utimer_start(m_timers[0], 10);
  forward(9);
  ck_assert_int_eq(m_event_list.index, 0);
  forward(1);
  ck_assert_int_eq(m_event_list.index, 1);
  forward(100);
  ck_assert_int_eq(m_event_list.index, 1);
  utimer_start(m_timers[0], 10);
  forward(100);
  ck_assert_int_eq(m_event_list.index, 2);
}
END_TEST


START_TEST(test_utimer_stop_1)
{
  m_action_list.actions[0].type = ACTION_STOP;
  m_action_list.actions[0].timer = m_timers[1];
  utimer_start(m_timers[0], 10);
  utimer_start(m_timers[1], 20);
  forward(9);
  ck_assert_int_eq(m_event_list.index, 0);
  forward(100);
  ck_assert_int_eq(m_event_list.index, 1);
  forward(100);
  ck_assert_int_eq(m_event_list.index, 1);
}
END_TEST

START_TEST(test_utimer_stop_2)
{
  m_action_list.actions[0].type = ACTION_STOP;
  m_action_list.actions[0].next = true;
  m_action_list.actions[0].timer = m_timers[1];
  m_action_list.actions[1].type = ACTION_STOP;
  m_action_list.actions[1].timer = m_timers[2];
  utimer_start(m_timers[0], 10);
  utimer_start(m_timers[1], 20);
  utimer_start(m_timers[2], 21);
  utimer_start(m_timers[3], 21);
  forward(9);
  ck_assert_int_eq(m_event_list.index, 0);
  forward(100);
  ck_assert_int_eq(m_event_list.index, 2);
  forward(100);
  ck_assert_int_eq(m_event_list.index, 2);
}
END_TEST


START_TEST(test_utimer_restart_same)
{
  m_action_list.actions[0].type = ACTION_START;
  m_action_list.actions[0].ticks = 1;
  m_action_list.actions[0].timer = m_timers[0];

  utimer_start(m_timers[0], 10);
  forward(10);
  ck_assert_int_eq(m_event_list.index, 1);
  forward(1);
  ck_assert_int_eq(m_event_list.index, 2);
}
END_TEST


START_TEST(test_utimer_restart_other)
{
  m_action_list.actions[0].type = ACTION_START;
  m_action_list.actions[0].ticks = 2;
  m_action_list.actions[0].timer = m_timers[1];

  utimer_start(m_timers[0], 10);
  utimer_start(m_timers[1], 11);
  forward(20);
  ck_assert_int_eq(m_event_list.index, 1);
  forward(1);
  ck_assert_int_eq(m_event_list.index, 1);
  forward(2);
  ck_assert_int_eq(m_event_list.index, 2);
}
END_TEST

START_TEST(test_utimer_immediate)
{
  m_action_list.actions[0].type = ACTION_START;
  m_action_list.actions[0].ticks = 0;
  m_action_list.actions[0].timer = m_timers[1];

  utimer_start(m_timers[0], 10);
  forward(10);
  ck_assert_int_eq(m_event_list.index, 2);
  ck_assert_ptr_eq(m_event_list.events[0].timer, m_timers[0]);
  ck_assert_ptr_eq(m_event_list.events[1].timer, m_timers[1]);
}
END_TEST

START_TEST(test_utimer_immediate2)
{
  m_action_list.actions[0].type = ACTION_START;
  m_action_list.actions[0].ticks = 0;
  m_action_list.actions[0].timer = m_timers[2];

  utimer_start(m_timers[0], 10);
  utimer_start(m_timers[1], 10);
  forward(10);
  ck_assert_int_eq(m_event_list.index, 3);
  ck_assert_ptr_eq(m_event_list.events[0].timer, m_timers[0]);
  ck_assert_ptr_eq(m_event_list.events[1].timer, m_timers[1]);
  ck_assert_ptr_eq(m_event_list.events[2].timer, m_timers[2]);
}
END_TEST

Suite * utimer_suite(void)
{
    Suite *s;
    TCase *tc_single;
    TCase *tc_periodic;
    TCase *tc_oneshot;
    TCase *tc_mixed;

    s = suite_create("utimer");

    /* Core test case */
    tc_single = tcase_create("Single");

    tcase_add_checked_fixture(tc_single, setup, teardown);
    tcase_add_test(tc_single, test_utimer_single);
    tcase_add_test(tc_single, test_utimer_stop_1);
    tcase_add_test(tc_single, test_utimer_stop_2);
    tcase_add_test(tc_single, test_utimer_restart_same);
    tcase_add_test(tc_single, test_utimer_restart_other);
    tcase_add_test(tc_single, test_utimer_immediate);
    tcase_add_test(tc_single, test_utimer_immediate2);
    suite_add_tcase(s, tc_single);


    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = utimer_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
