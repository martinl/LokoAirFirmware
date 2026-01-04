#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32_timer.h"

#define TIMERTIME_T_MAX ((uint32_t)~0)

typedef UTIL_TIMER_Time_t TimerTime_t;
typedef UTIL_TIMER_Object_t TimerEvent_t;

#define TimerInit(HANDLE, CB)          UTIL_TIMER_Create(HANDLE, TIMERTIME_T_MAX, UTIL_TIMER_ONESHOT, CB, NULL)
#define TimerSetValue(HANDLE, TIMEOUT) UTIL_TIMER_SetPeriod(HANDLE, TIMEOUT)
#define TimerStart(HANDLE)             UTIL_TIMER_Start(HANDLE)
#define TimerStop(HANDLE)              UTIL_TIMER_Stop(HANDLE)
#define TimerGetCurrentTime            UTIL_TIMER_GetCurrentTime
#define TimerGetElapsedTime            UTIL_TIMER_GetElapsedTime

#ifdef __cplusplus
}
#endif
