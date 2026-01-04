#include "gnss_trace.h"
#include <bsp.h>
#include <string.h>

/* -------------------------------------------------------------------------- */

static uint8_t _checksum(uint8_t const *data, size_t size) {
    uint8_t checksum = (uint8_t)size;

    while (size-- != 0) {
        checksum -= *data++;
    }

    return checksum;
}

/* -------------------------------------------------------------------------- */

static void _add_checksum(gtrace_record_t *record) {
    record->checksum = _checksum((uint8_t *const)record, sizeof(gtrace_record_t) - 1 /* checksum */);
}

/* -------------------------------------------------------------------------- */

static uint8_t _get_checksum(gtrace_record_t *record) {
    return _checksum((uint8_t *const)record, sizeof(gtrace_record_t) - 1 /* checksum */);
}

/* -------------------------------------------------------------------------- */

static size_t _get_next_page(size_t page) {
    page++;
    page %= bsp_flash_get_gnss_trace_page_count();
    return page;
}

/* -------------------------------------------------------------------------- */

static void _switch_page(gtrace_t *context) {
    context->current_page = _get_next_page(context->current_page);
    bsp_flash_gnss_trace_erase(context->current_page);

    context->current_index = 0;

    if (context->written_records > context->rec_per_page) {
        context->written_records -= context->rec_per_page;
    }
}

/* -------------------------------------------------------------------------- */

static size_t _check_page(gtrace_t const *context, size_t page) {
    gtrace_record_t empty_record;
    memset(&empty_record, 0xFF, sizeof(empty_record));
    size_t page_offset = bsp_flash_get_gnss_trace_page_size() * page;

    size_t empty_records = 0;
    size_t invalid_record = 0;

    for (size_t i = 0; i < context->rec_per_page; i++) {
        gtrace_record_t record;
        bsp_flash_gnss_trace_read(i * sizeof(gtrace_record_t) + page_offset, &record, sizeof(gtrace_record_t));

        if (memcmp(&empty_record, &record, sizeof(record)) == 0) {
            empty_records++;
        } else {
            if (record.checksum != _get_checksum(&record)) {
                invalid_record++;
            }
        }
    }

    if (invalid_record) {
        LOG_WARNING("Erase page %u, Empty records %u, Invalid records %d", page, empty_records, invalid_record);
        bsp_flash_gnss_trace_erase(page);
        return 0;
    }

    return context->rec_per_page - empty_records;
}

/* -------------------------------------------------------------------------- */

gtrace_result_t gtrace_init(gtrace_t *context) {
    size_t page_count = bsp_flash_get_gnss_trace_page_count();

    context->rec_per_page = bsp_flash_get_gnss_trace_page_size() / sizeof(gtrace_record_t);
    context->max_records = context->rec_per_page * bsp_flash_get_gnss_trace_page_count();
    context->written_records = 0;
    context->current_page = page_count - 1;
    context->current_index = context->rec_per_page;

    bool is_full_filled_page_found = false;
    for (size_t i = 0; i < page_count; i++) {
        size_t rec_count = _check_page(context, i);

        if (is_full_filled_page_found == false) {
            if (rec_count < context->rec_per_page) {
                is_full_filled_page_found = true;
                context->current_page = i;
                context->current_index = rec_count;
            }
        }

        context->written_records += rec_count;
    }

    return GTRACE_RESULT_OK;
}

/* -------------------------------------------------------------------------- */

gtrace_result_t gtrace_add(gtrace_t *context, gtrace_record_t *record) {
    if (context->current_index >= context->rec_per_page) {
        _switch_page(context);
    }

    _add_checksum(record);

    size_t page_offset = bsp_flash_get_gnss_trace_page_size() * context->current_page;

    bsp_flash_gnss_trace_write(context->current_index * sizeof(gtrace_record_t) + page_offset,
                               record,
                               sizeof(gtrace_record_t));
    context->current_index++;
    context->written_records++;

    return GTRACE_RESULT_OK;
}

/* -------------------------------------------------------------------------- */

size_t gtrace_get_record_count(gtrace_t const *context) {
    return context->written_records;
}

/* -------------------------------------------------------------------------- */

gtrace_result_t gtrace_get_record(gtrace_t const *context, size_t index, gtrace_record_t *record) {
    if (index > context->written_records) {
        return GTRACE_RESULT_ERROR;
    }

    size_t page_offset = bsp_flash_get_gnss_trace_page_size() * context->current_page;
    size_t correct_index = index % context->rec_per_page;

    if (context->written_records >= context->rec_per_page) {
        if (index < context->rec_per_page) {
            page_offset = bsp_flash_get_gnss_trace_page_size() * _get_next_page(context->current_page);
        }
    }

    bsp_flash_gnss_trace_read(correct_index * sizeof(gtrace_record_t) + page_offset, record, sizeof(gtrace_record_t));

    return (record->checksum == _get_checksum(record)) ? GTRACE_RESULT_OK : GTRACE_RESULT_ERROR;
}

/* -------------------------------------------------------------------------- */

void gtrace_erase_all(gtrace_t *context) {
    for (size_t i = 0; i < bsp_flash_get_gnss_trace_page_count(); i++) {
        bsp_flash_gnss_trace_erase(i);
    }

    context->current_index = 0;
    context->current_page = 0;
    context->written_records = 0;
}

/* -------------------------------------------------------------------------- */
