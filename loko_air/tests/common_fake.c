#include <bsp.h>

#include <gnss_trace.h>

gtrace_t *app_get_gtrace_context(void) {
    static gtrace_t gtrace_fake = { 0 };
    return &gtrace_fake;
}
