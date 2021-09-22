#pragma once

#include <stdint.h>

#ifndef TICKS_WIDTH
#define TICKS_WIDTH 32
#endif

#if TICKS_WIDTH == 64
  typedef uint64_t ticks_t;
  typedef int64_t dticks_t;
  #define TICKS_MAX INT64_MAX
#elif TICKS_WIDTH == 32
  typedef uint32_t ticks_t;
  typedef int32_t dticks_t;
  #define TICKS_MAX INT32_MAX
#elif TICKS_WIDTH == 16
  typedef uint16_t ticks_t;
  typedef int16_t dticks_t;
  #define TICKS_MAX INT16_MAX
#elif TICKS_WIDTH == 8
  typedef uint8_t ticks_t;
  typedef int8_t dticks_t;
  #define TICKS_MAX INT8_MAX
#else
  #error "unknown ticks width"
#endif

#if !defined(TICKS_PER_MS) && !defined(MS_PER_TICK)
#define TICKS_PER_MS 1
#define MS_PER_TICK 1
#endif

#if defined(TICKS_PER_MS)
#define MS_TO_TICKS(ms) ((ms) * (TICKS_PER_MS))
#elif defined(MS_PER_TICK)
#define MS_TO_TICKS(ms) (((ms) + ((MS_PER_TICK) / 2)) / (MS_PER_TICK))
#else
#error "TICKS_PER_MS or MS_PER_TICK not defined"
#endif


#if defined(MS_PER_TICK)
#define TICKS_TO_MS(ticks) ((ticks) * (MS_PER_TICK))
#elif defined(TICKS_PER_MS)
#define TICKS_TO_MS(ticks) (((ticks) + ((TICKS_PER_MS) / 2)) / (TICKS_PER_MS))
#else
#error "TICKS_PER_MS or MS_PER_TICK not defined"
#endif


ticks_t ticks_now(void);

static inline ticks_t ticks_elapsed(ticks_t ts)
{
  return ticks_now() - ts;
}

