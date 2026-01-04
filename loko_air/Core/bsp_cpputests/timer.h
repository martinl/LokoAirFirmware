#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define TIMERTIME_T_MAX ((uint32_t)~0)

typedef void *TimerTime_t;
typedef void *TimerEvent_t;

#define TimerInit(HANDLE, CB)
#define TimerSetValue(HANDLE, TIMEOUT)
#define TimerStart(HANDLE)
#define TimerStop(HANDLE)
#define TimerGetCurrentTime
#define TimerGetElapsedTime

#ifdef __cplusplus
}
#endif
