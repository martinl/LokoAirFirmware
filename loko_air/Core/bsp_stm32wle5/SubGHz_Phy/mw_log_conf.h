#ifndef __MW_LOG_CONF_H__
#define __MW_LOG_CONF_H__

#ifdef __cplusplus
extern "C" {
#endif

#if LOG_ENABLED == 1U
#    define MW_LOG_ENABLED
#    define MW_LOG(TS, VL, ...) LOG_DEBUG(LOG_COLOR(LOG_COLOR_CYAN) "MW_LOG:"__VA_ARGS__);
#else /* MW_LOG_ENABLED */
#    define MW_LOG(TS, VL, ...)
#endif /* MW_LOG_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /*__MW_LOG_CONF_H__ */
