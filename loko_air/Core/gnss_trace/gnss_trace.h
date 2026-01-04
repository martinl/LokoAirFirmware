#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <lwgps.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t current_page;
    size_t current_index;
    size_t written_records;
    size_t rec_per_page;
    size_t max_records;
} gtrace_t;

typedef struct PACKED {
    lwgps_float_t latitude;  /*!< Latitude in units of degrees */
    lwgps_float_t longitude; /*!< Longitude in units of degrees */
    uint32_t seconds : 6;    /*!< Seconds in UTC (0-59) */
    uint32_t minutes : 6;    /*!< Minutes in UTC (0-59) */
    uint32_t hours   : 5;    /*!< Hours in UTC (0-23) */
    uint32_t date    : 5;    /*!< Fix date (1-31) */
    uint32_t month   : 4;    /*!< Fix month (1-12) */
    uint32_t year    : 6;    /*!< Fix year (0-63, representing 2000-2063) */
    uint16_t alt;            /*!< Altitude in units of meters */
    uint8_t speed_mps;       /*!< Speed in meters per second */
    uint8_t checksum;        /* for internal purposes */
} __packed gtrace_record_t;

typedef enum {
    GTRACE_RESULT_OK,
    GTRACE_RESULT_ERROR,
} gtrace_result_t;

gtrace_result_t gtrace_init(gtrace_t *context);
gtrace_result_t gtrace_add(gtrace_t *context, gtrace_record_t *record);
size_t gtrace_get_record_count(gtrace_t const *context);
gtrace_result_t gtrace_get_record(gtrace_t const *context, size_t index, gtrace_record_t *record);
void gtrace_erase_all(gtrace_t *context);

#ifdef __cplusplus
}
#endif /* __cplusplus */
