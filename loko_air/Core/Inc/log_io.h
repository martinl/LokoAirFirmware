
#if defined(__LOGGER_INTERFACE__)
#    error This file provide interface for log_.c and this file must be include only once in main.c
#else
#    define __LOGGER_INTERFACE__ 1U
#endif

#if LOG_ENABLED == 1U

/* -------------------------------------------------------------------------- */

static inline void _log_write(const uint8_t *data, size_t size) {
    bsp_uart_debug_write(data, size);
}

/* -------------------------------------------------------------------------- */

#    if LOG_THREADSAFE_ENABLED == 1U

static void _lock(void) {
    // There is template, just add your implementation
}

/* -------------------------------------------------------------------------- */

static void _unlock(void) {
    // There is template, just add your implementation
}

#    endif  // LOG_THREADSAFE_ENABLED == 1U

/* -------------------------------------------------------------------------- */

#    if LOG_TIMESTAMP_ENABLED == 1U

static inline log_timestamp_t _get_ts(void) {
    // There is template, just add your implementation
    return (log_timestamp_t)bsp_get_ticks();
}

#    endif  // LOG_TIMESTAMP_ENABLED == 1U

/* -------------------------------------------------------------------------- */

#    if LOG_ISR_QUEUE == 1U

static bool _is_isr(void) {
    // There is template, just add your implementation
    return false;
}

#    endif  // LOG_ISR_QUEUE == 1U

/* -------------------------------------------------------------------------- */

const log_io_t LOG_IO_INTERFACE = {
    .write = _log_write,
#    if LOG_THREADSAFE_ENABLED == 1U
    .lock = _lock,
    .unlock = _unlock,
#    endif  // LOG_THREADSAFE_ENABLED == 1U
#    if LOG_TIMESTAMP_ENABLED == 1U
    .get_ts = _get_ts,
#    endif  // LOG_TIMESTAMP_ENABLED == 1U
#    if LOG_ISR_QUEUE == 1U
    .is_isr = _is_isr,
#    endif  // LOG_ISR_QUEUE == 1U
};

/* -------------------------------------------------------------------------- */

#endif  // LOG_ENABLED == 1U
