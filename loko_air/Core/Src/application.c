/* Includes ------------------------------------------------------------------*/
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "subghz_phy_app.h"
#include <bsp.h>
#include <cayenne_lpp_c.h>
#include <cmd_line/cmd_line.h>
#include <encrypt_p2p_payload/encrypt_p2p_payload.h>
#include <gnss_trace/gnss_trace.h>
#include <log_io.h>
#include <lorawan_app/lora_app.h>
#include <lwgps.h>
#include <math.h>
#include <queue/queue.h>
#include <settings/settings.h>
#include <settings_io.h>
#include <utils.h>
#include <version.h>

#include "stm32_timer.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct send_gnss_data_s {
    double lat;
    double lon;
    uint16_t alt;
    uint16_t speed_mps;
} send_gnss_data_t;

typedef enum {
    SYSTEM_STATE_INIT,
    SYSTEM_STATE_TICKLE_CHARGE_MODE,
    SYSTEM_STATE_WAIT_FOR_GPS_FIX,
    SYSTEM_STATE_SEND_DATA,
    SYSTEM_STATE_SHUTDOWN,
    SYSTEM_STATE_SHUTDOWN_CHARGING,
} system_state_t;

typedef struct PACKED {
    uint32_t id1;
    uint32_t id2;
    uint8_t vbat    : 4; /* (n + 27) * 0.1 */
    uint8_t version : 4;
    uint8_t lat_32bit[4];
    uint8_t lon_32bit[4];
} __packed packet_data_short_t;

typedef struct PACKED {
    uint32_t id1;
    uint32_t id2;
    uint8_t vbat    : 4; /* (n + 27) * 0.1 */
    uint8_t version : 4;
    uint8_t lat_32bit[4];
    uint8_t lon_32bit[4];
    uint8_t speed_mps;
    uint16_t alt;
} __packed packet_data_extended_t;

typedef struct PACKED {
    uint32_t id1;
    uint32_t id2;
    uint8_t reserved : 4;
    uint8_t version  : 4;
    enc_p2p_payload_t payload;
} __packed packet_data_encrypted_t;

#define PACKET_DATA_VERSION_SHORT     (3) /* Step is 3*/
#define PACKET_DATA_VERSION_EXTENDED  (PACKET_DATA_VERSION_SHORT + 1)
#define PACKET_DATA_VERSION_ENCRYPTED (PACKET_DATA_VERSION_SHORT + 2)

/* -------------------------------------------------------------------------- */

/* Private define ------------------------------------------------------------*/

#define BATTERY_CHECK_PERIOD_MS       (10 * 1000UL) /*<! Battery voltage checking period */
#define BATTERY_MIN_LEVEL_FOR_GNSS_MV (3400)        /*<! GNSS Enable battery level */
#define BATTERY_MIN_LEVEL_POWER_ON_MV (3100)        /*<! Minimal power on battery level */
#define BSP_RTC_STORE_REG_WAKEUP      (0)           /*<! */
#define DEBUG_PRINT_NMEA_DATA         (0)           /*<! Set 1 to see data from GNSS module */
#define BUTTON_HOLD_TIMEOUT_MS        (3000UL)      /*<! Button hold timeout, milliseconds*/
#define NO_FIX_TIMEOUT_MS                                                                                           \
    (5 * 60 * 1000UL) /*<! When GPS can't catch satellites during NO_FIX_TIMEOUT_MS time(milliseconds), go to sleep \
                         for SLEEP_NO_FIX_PERIOD_S */
#define QUEUE_DEBUG_RX_SIZE     (1024) /*<! */
#define QUEUE_RX_GNSS_SIZE      (4096) /*<! */
#define SHUTDOWN_NO_AUTO_WAKEUP (0)    /*<! */
#define SHUTDOWN_KEEP_WAKEUP    (-1U)  /*<! */

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

static gtrace_t _gtrace; /* GNSS Trace context */
static lwgps_t _gnss;    /* GNSS parser handle, contain all information about navigation */
static cayenne_lpp_t _cayenne_lpp;
static QUEUE(_gnss_rx_queue, QUEUE_RX_GNSS_SIZE, uint8_t);
static QUEUE(_debug_rx_queue, QUEUE_DEBUG_RX_SIZE, uint8_t);
static system_state_t _system_state = SYSTEM_STATE_INIT;
static bool _is_enable_by_button = false;
static bool _is_power_off_request = false;

#if LOG_ENABLED == 1U
static const char *const DEBUG_START_INFO_STR[BSP_START_REASON_COUNT] = {
    [BSP_START_REASON_POWER_ON] = "BSP_START_REASON_POWER_ON",
    [BSP_START_REASON_RESET] = "BSP_START_REASON_RESET",
    [BSP_START_REASON_BUTTON] = "BSP_START_REASON_BUTTON",
    [BSP_START_REASON_CHARGER] = "BSP_START_REASON_CHARGER",
    [BSP_START_REASON_TIMER_ALARM] = "BSP_START_REASON_TIMER_ALARM",
};
#endif  // LOG_ENABLED == 1U
/* Private function prototypes -----------------------------------------------*/

gtrace_t *app_get_gtrace_context(void);
static bool _gnss_trace_wakeup_counter_is_need_save(void);
static bool _is_battery_voltage_critical_low(void);
static bool _is_battery_voltage_low(void);
static void _background_loop(void);
static void _detect_wakeup_reason(void);
static void _gnss_trace_save(lwgps_t const *gnss);
static void _gnss_trace_wakeup_counter_inc(void);
static void _gnss_trace_wakeup_counter_reset(void);
static void _gtrace_init(void);
static void _gtrace_load(send_gnss_data_t *data);
static void _led_blink_1x(void);
static void _led_blink_3x(void);
static void _lora_init(void);
static void _prepare_to_sleep(void);
static void _send_gnss_data(send_gnss_data_t const *gnss_data);
static void _shutdown_button_holding_indication(void);
static void _power_on_button_holding_indication(void);
static void _shutdown(uint32_t auto_wakeup_timeout_s);
static void _status_print(bool is_blink_enable);
static void _switch_mode(system_state_t mode);
static void _uart_data_proccess(void);
static void _send_gnss_data_by_p2p_non_text_data(send_gnss_data_t const *gnss_data);

/* Private user code ---------------------------------------------------------*/

/* Called from Interrupt handler, byte receiving */
void bsp_uart_gnss_byte_received(uint8_t byte) {
    static bool _is_queue_full_dbg_display = false;

    if (queue_enqueue(QHEAD(_gnss_rx_queue), &byte, enqueue8) == false) {
        if (_is_queue_full_dbg_display == false) {
            LOG_ERROR("_gnss_rx_queue is full");
        }

        _is_queue_full_dbg_display = true;
    } else {
        _is_queue_full_dbg_display = false;
    }
}

/* -------------------------------------------------------------------------- */

/* Called from Interrupt handler, byte receiving */
void bsp_uart_debug_byte_received(uint8_t byte) {
    static bool _is_queue_full_dbg_display = false;

    if (queue_enqueue(QHEAD(_debug_rx_queue), &byte, enqueue8) == false) {
        if (_is_queue_full_dbg_display == false) {
            LOG_ERROR("_debug_rx_queue is full");
        }

        _is_queue_full_dbg_display = true;
    } else {
        _is_queue_full_dbg_display = false;
    }
}

/* -------------------------------------------------------------------------- */

static void _uart_data_proccess(void) {
    uint8_t queue_item = 0;

    /* Parse debug stream */
    while (queue_dequeue(QHEAD(_debug_rx_queue), &queue_item, dequeue8)) {
        cmd_line_receive(queue_item);
    }

    /* Parse NMEA stream */
    while (queue_dequeue(QHEAD(_gnss_rx_queue), &queue_item, dequeue8)) {
#if (DEBUG_PRINT_NMEA_DATA == 1U)
        bsp_uart_debug_write(&queue_item, sizeof(queue_item));
#endif
        lwgps_process(&_gnss, &queue_item, sizeof(queue_item));
    }
}

/* -------------------------------------------------------------------------- */

static void _background_loop(void) {

    _status_print(_is_enable_by_button ? true : bsp_gpio_is_usb_charger_connect());
    _uart_data_proccess();

    if (settings_get_is_lorawan_mode()) {
        lorawan_process();
    }

    /* Button handling */
    static bool _is_button_pressed_last_state = false;
    if (bsp_gpio_is_button_pressed() != _is_button_pressed_last_state) {
        _is_button_pressed_last_state = bsp_gpio_is_button_pressed();
        LOG_INFO("Button state changed - %d", _is_button_pressed_last_state);

        if (_is_button_pressed_last_state) {
            if (_is_power_off_request == true) {
                _power_on_button_holding_indication();
            } else {
                _shutdown_button_holding_indication();
            }

            if (bsp_gpio_is_button_pressed() == true) {
                _is_power_off_request = !_is_power_off_request;
                _switch_mode(_is_power_off_request == true ? SYSTEM_STATE_SHUTDOWN : SYSTEM_STATE_INIT);
            }
        }
    }
}

/* -------------------------------------------------------------------------- */

static void _switch_mode(system_state_t mode) {
#if LOG_ENABLED == 1
    static char const *const MODE_LIST[] = {
        [SYSTEM_STATE_INIT] = "SYSTEM_STATE_INIT",
        [SYSTEM_STATE_TICKLE_CHARGE_MODE] = "SYSTEM_STATE_TICKLE_CHARGE_MODE",
        [SYSTEM_STATE_WAIT_FOR_GPS_FIX] = "SYSTEM_STATE_WAIT_FOR_GPS_FIX",
        [SYSTEM_STATE_SEND_DATA] = "SYSTEM_STATE_SEND_DATA",
        [SYSTEM_STATE_SHUTDOWN] = "SYSTEM_STATE_SHUTDOWN",
        [SYSTEM_STATE_SHUTDOWN_CHARGING] = "SYSTEM_STATE_SHUTDOWN_CHARGING",
    };
    LOG_DEBUG(LOG_COLOR(LOG_COLOR_CYAN) "Mode [%s] >>> [%s]", MODE_LIST[_system_state], MODE_LIST[mode]);
#endif
    _system_state = mode;
}

/* -------------------------------------------------------------------------- */

static bool _wait_for_gnss_response(char const *str, uint32_t timeout_ms) {
    uint32_t start_ts = bsp_get_ticks();
    size_t str_len = strlen(str);
    uint8_t queue_item = 0;
    size_t str_idx = 0;

    while ((bsp_get_ticks() - start_ts) < timeout_ms) {
        if (queue_dequeue(QHEAD(_gnss_rx_queue), &queue_item, dequeue8) == false) {
            bsp_lp_sleep();
            continue;
        }
#if (DEBUG_PRINT_NMEA_DATA == 1U)
        bsp_uart_debug_write(&queue_item, sizeof(queue_item));
#endif
        lwgps_process(&_gnss, &queue_item, sizeof(queue_item));
        if (queue_item == str[str_idx]) {
            str_idx++;
            if (str_idx == str_len) {
                return true;
            }
        } else {
            str_idx = 0;
        }
    }

    return false;
}

/* -------------------------------------------------------------------------- */

static void _gnss_init(void) {
    LOG_INFO("GNSS power on...");
    static const uint32_t GNSS_POWER_ON_DELAY_MS = 5;

    /* Reduce MCU clock to avoid deep power drop */
    bsp_clock_switch(BSP_CLOCK_CORE_100_KHZ);
    bsp_delay_ms(GNSS_POWER_ON_DELAY_MS);
    bsp_gpio_gnss_on();
    bsp_delay_ms(GNSS_POWER_ON_DELAY_MS);
    bsp_gpio_gnss_wakeup_leave();
    bsp_delay_ms(GNSS_POWER_ON_DELAY_MS);
    bsp_clock_switch(DEFAULT_FREQ);

    bsp_uart_gnss_init();

    /* Example:
       $PMTK886,0*28<CR><LF> :Enter normal mode.
       $PMTK886,1*29<CR><LF> :Enter fitness mode.
       $PMTK886,2*2A<CR><LF> :Enter aviation mode.
       $PMTK886,3*2B<CR><LF> :Enter balloon mode.
       $PMTK886,4*2C<CR><LF> :Enter stationary mode.
   */
    static char const *const MODES[SETTINGS_GNSS_MODE_COUNT] = {
        [SETTINGS_GNSS_MODE_NORMAL] = "$PMTK886,0*28\r\n",     [SETTINGS_GNSS_MODE_FITNESS] = "$PMTK886,1*29\r\n",
        [SETTINGS_GNSS_MODE_AVIATION] = "$PMTK886,2*2A\r\n",   [SETTINGS_GNSS_MODE_BALLOON] = "$PMTK886,3*2B\r\n",
        [SETTINGS_GNSS_MODE_STATIONARY] = "$PMTK886,4*2C\r\n",
    };

    static const size_t BLIND_SEND_ATTEMPT = 3;
    settings_gnss_mode_t mode = settings_get_gnss_mode();
    if (mode >= SETTINGS_GNSS_MODE_COUNT) {
        return;
    }

    LOG_INFO("GNSS mode set to %d", mode);
    char const *const MODE = MODES[mode];
    size_t str_len = strlen(MODE);
    const uint32_t SEND_TIMEOUT_MS = 100;
    for (size_t i = 0; i < BLIND_SEND_ATTEMPT; ++i) {
        bsp_uart_gnss_write((const uint8_t *)MODE, str_len);
        if (_wait_for_gnss_response("PMTK001,886,", SEND_TIMEOUT_MS) == true) {
            return;
        }
    }
    LOG_ERROR("GNSS mode set timeout");
}

/* -------------------------------------------------------------------------- */

gtrace_t *app_get_gtrace_context(void) {
    return &_gtrace;
}

/* -------------------------------------------------------------------------- */

static void _led_blink_3x(void) {
    for (size_t i = 0; i < 3; i++) {
        bsp_delay_ms(300);
        bsp_battery_measure();
        bsp_gpio_led_on();
        bsp_delay_ms(300);
        bsp_gpio_led_off();
    }
}

/* -------------------------------------------------------------------------- */

static void _led_blink_1x(void) {
    bsp_gpio_led_on();
    bsp_delay_ms(100);
    bsp_gpio_led_off();
}

/* -------------------------------------------------------------------------- */

static void _prepare_to_sleep(void) {
    bsp_gpio_led_off();
    if (settings_is_debug_output() == false) {
        bsp_gpio_gnss_wakeup_enter();
        bsp_gpio_gnss_off();
    }
    subghz_radio_deinit();
}

/* -------------------------------------------------------------------------- */

static void _shutdown(uint32_t auto_wakeup_timeout_s) {
    bsp_wakeup_source_mask_list_t source = WAKEUP_SOURCE_BUTTON_1_MASK | WAKEUP_SOURCE_5V_DETECT_MASK;

    if (SHUTDOWN_NO_AUTO_WAKEUP == auto_wakeup_timeout_s) {
        LOG_DEBUG("Shutdown");
        bsp_rtc_wakeup_deactivate();
    } else if (SHUTDOWN_KEEP_WAKEUP == auto_wakeup_timeout_s) {
        LOG_DEBUG("Shutdown with keep wakeup");
    } else {
        LOG_DEBUG("Shutdown with auto wakeup after %lu sec", auto_wakeup_timeout_s);
        source |= WAKEUP_SOURCE_TIMER_MASK;
    }

    _prepare_to_sleep();

    while (bsp_gpio_is_button_pressed()) {
        // Wait for release button to avoid wake-up
    }

    bsp_lp_shutdown(source, auto_wakeup_timeout_s);
    BSP_FATAL(0, "Can't enter to shutdown mode");
}

/* -------------------------------------------------------------------------- */

static void _power_on_button_holding_indication(void) {
    static const uint32_t DEBOUNCE_DELAY_MS = 50;

    bsp_gpio_led_on();
    uint32_t ts = bsp_get_ticks();

    while (bsp_gpio_is_button_pressed()) {
        bsp_battery_measure();
        if ((bsp_get_ticks() - ts) >= BUTTON_HOLD_TIMEOUT_MS) {
            break;
        }

        bsp_delay_ms(DEBOUNCE_DELAY_MS);
    }

    bsp_gpio_led_off();
}

/* -------------------------------------------------------------------------- */

static void _shutdown_button_holding_indication(void) {
    bsp_gpio_led_on();
    uint32_t ts = bsp_get_ticks();

    while (bsp_gpio_is_button_pressed()) {
        _uart_data_proccess();
        if ((bsp_get_ticks() - ts) >= BUTTON_HOLD_TIMEOUT_MS) {
            break;
        }

        bsp_delay_ms(50);
        bsp_gpio_led_toggle();
    }

    bsp_gpio_led_off();
}

/* -------------------------------------------------------------------------- */

static void _detect_wakeup_reason(void) {
    bsp_start_reason_t reason = bsp_get_start_reason();

    bool is_shutdown = false;
    /* Wait a bit to smooth noise on button or charger */
    bsp_delay_ms(100);

    switch (reason) {
        case BSP_START_REASON_RESET:
            LOG_DEBUG("Wakeup reason: Reset");
            is_shutdown = true;
            break;
        case BSP_START_REASON_POWER_ON:
            LOG_DEBUG("Wakeup reason: Power On");
            break;
        case BSP_START_REASON_BUTTON:
            LOG_DEBUG("Wakeup reason: Button");
            if (bsp_rtc_is_wakeup_activated() == false) {
                _power_on_button_holding_indication();
            } else {
                _shutdown_button_holding_indication();

                if (bsp_gpio_is_button_pressed() == true) {
                    _shutdown(SHUTDOWN_NO_AUTO_WAKEUP);
                }
            }
            is_shutdown = (bsp_gpio_is_button_pressed() == false);
            break;
        case BSP_START_REASON_TIMER_ALARM:
            LOG_DEBUG("Wakeup reason: Timer");
            break;
        case BSP_START_REASON_CHARGER:
            LOG_DEBUG("Wakeup reason: Charger");
            /* do not power on when it wakeup reason was charger and charger disconnected now  */
            is_shutdown = (bsp_gpio_is_usb_charger_connect() == false);
            break;
        default:
            LOG_DEBUG("Wakeup reason: Unknown reason");
            break;
    }

    if (is_shutdown == true) {
        _shutdown(SHUTDOWN_KEEP_WAKEUP);
    }
}

/* -------------------------------------------------------------------------- */

static void _lora_init(void) {
    LOG_INFO("LoRa power on...");
    app_lora_params_t params = {
        .freq = settings_get_lora_frequency_hz(),
        .tx_power = settings_get_tx_power(),
    };
    subghz_radio_init(&params);
}

/* -------------------------------------------------------------------------- */

static void _gtrace_init(void) {
    gtrace_init(&_gtrace);
    LOG_INFO("Loaded GNSS Trace info: record found %u, Page %u, Index %u, Record per page %u",
             _gtrace.written_records,
             _gtrace.current_page,
             _gtrace.current_index,
             _gtrace.rec_per_page);
}

/* -------------------------------------------------------------------------- */

static void _gnss_trace_save(lwgps_t const *gnss) {
    uint32_t speed_mps = (uint8_t)lwgps_to_speed(gnss->speed, lwgps_speed_mps);
    if (speed_mps > UINT8_MAX) {
        speed_mps = UINT8_MAX;
    }
    gtrace_record_t record = {
        .latitude = gnss->latitude,
        .longitude = gnss->longitude,
        .hours = gnss->hours,
        .minutes = gnss->minutes,
        .seconds = gnss->seconds,
        .date = gnss->date,
        .month = gnss->month,
        .year = gnss->year,
        .alt = (uint16_t)gnss->altitude,
        .speed_mps = (uint8_t)speed_mps,
        .checksum = 0,  // Filled inside gtrace_add
    };

    LOG_DEBUG("Save GNSS Trace: %" PRIu32 "/%" PRIu32 "/%" PRIu32 " %" PRIu32 ":%" PRIu32 ":%" PRIu32 " %f, %f",
              (uint32_t)record.year,
              (uint32_t)record.month,
              (uint32_t)record.date,
              (uint32_t)record.hours,
              (uint32_t)record.minutes,
              (uint32_t)record.seconds,
              record.latitude,
              record.longitude);
    gtrace_add(&_gtrace, &record);
}

/* -------------------------------------------------------------------------- */

static bool _gnss_trace_wakeup_counter_is_need_save(void) {
    return (bsp_rtc_store_read_reg(BSP_RTC_STORE_REG_WAKEUP) >= settings_get_gnss_trace_save_mult());
}

/* -------------------------------------------------------------------------- */

static void _gnss_trace_wakeup_counter_reset(void) {
    bsp_rtc_store_write_reg(BSP_RTC_STORE_REG_WAKEUP, 0);
}

/* -------------------------------------------------------------------------- */

static void _gnss_trace_wakeup_counter_inc(void) {
    uint32_t wake_up_counter = bsp_rtc_store_read_reg(BSP_RTC_STORE_REG_WAKEUP) + 1U;
    bsp_rtc_store_write_reg(BSP_RTC_STORE_REG_WAKEUP, wake_up_counter);
    LOG_DEBUG("GTrace Wake-up counter %" PRIu32 ", Threshold %" PRIu32,
              wake_up_counter,
              settings_get_gnss_trace_save_mult());
}

/* -------------------------------------------------------------------------- */

static bool _is_battery_voltage_low(void) {
    uint32_t vbat = bsp_battery_get_voltage();

    return (vbat < BATTERY_MIN_LEVEL_FOR_GNSS_MV);
}

/* -------------------------------------------------------------------------- */

static bool _is_battery_voltage_critical_low(void) {
    uint32_t vbat = bsp_battery_get_voltage();

    return (vbat < BATTERY_MIN_LEVEL_POWER_ON_MV);
}

/* -------------------------------------------------------------------------- */

static uint8_t _pack_vbat(uint32_t vbat_mv) {
    static const uint32_t VBAT_OFFSET = 2700;
    static const uint32_t VBAT_MAX = 4200;
    uint32_t vbat = MIN(vbat_mv, VBAT_OFFSET);
    vbat = MAX(vbat, VBAT_MAX);
    return (uint8_t)((vbat / 100) - (VBAT_OFFSET / 100));
}

/* -------------------------------------------------------------------------- */

static void _pack_lat_lon_32(double lat_lon, uint8_t packed_data[4]) {
    double scale = 1000000.0;
    int32_t lat_lon_scaled = (int32_t)round(lat_lon * scale);

    packed_data[0] = (uint8_t)(lat_lon_scaled >> 24);
    packed_data[1] = (uint8_t)(lat_lon_scaled >> 16);
    packed_data[2] = (uint8_t)(lat_lon_scaled >> 8);
    packed_data[3] = (uint8_t)lat_lon_scaled;
}

/* -------------------------------------------------------------------------- */

static void _send_gnss_data_by_lorawan(send_gnss_data_t const *gnss_data) {
    LOG_DEBUG("Ready to send, wait for join complete...");

    uint32_t join_timeout_ts = bsp_get_ticks();
    while (lorawan_is_joined() == false) {
        if ((bsp_get_ticks() - join_timeout_ts) > 30000) {
            LOG_ERROR("Join timeout");
            break;
        }
        _background_loop();
        bsp_lp_sleep();
    }

    if (lorawan_is_joined() == true) {
        LOG_DEBUG("Send LoRaWAN data...");
        cayenne_lpp_reset(&_cayenne_lpp);
        cayenne_lpp_add_gps(&_cayenne_lpp, 1, (float)gnss_data->lat, (float)gnss_data->lon, (float)gnss_data->alt);

        lorawan_send(_cayenne_lpp.buffer, _cayenne_lpp.cursor);

        while (lorawan_is_tx_complete() == false) {
            _background_loop();
            bsp_lp_sleep();
        }
        LOG_DEBUG("Send done");
    }

#if DELAY_AFTER_SEND == 1
    uint32_t wait_ts = bsp_get_ticks();
    while ((bsp_get_ticks() - wait_ts) < 10000) {
        _background_loop();
        bsp_lp_sleep();
    }
#endif  // DELAY_AFTER_SEND == 1

    LOG_DEBUG("LoraWan deinit");
    while (lorawan_deinit() == false) {
        _background_loop();
        bsp_lp_sleep();
    }
}

/* -------------------------------------------------------------------------- */

static void _send_gnss_data_by_p2p_non_text_data(send_gnss_data_t const *gnss_data) {

    uint8_t tx_payload[MAX(sizeof(packet_data_encrypted_t), sizeof(packet_data_extended_t))] = { 0 };
    size_t tx_size = 0;

    _lora_init();

    if (settings_get_is_p2p_encrypted() == true) {
        packet_data_encrypted_t packet_data_encrypted = {
            .id1 = settings_get_id_1(),
            .id2 = settings_get_id_2(),
            .version = PACKET_DATA_VERSION_ENCRYPTED,
            .reserved = 0,
            .payload = {
                .vbat = _pack_vbat(bsp_battery_get_voltage()),
                .reserved0 = 0,
                .lat_32bit = {0},
                .lon_32bit = {0},
                .speed_mps = (uint8_t)gnss_data->speed_mps,
                .alt_meters = gnss_data->alt,
                .reserved1 = {0},
                .integrity = 0,
            },
        };
        _pack_lat_lon_32(gnss_data->lat, packet_data_encrypted.payload.lat_32bit);
        _pack_lat_lon_32(gnss_data->lon, packet_data_encrypted.payload.lon_32bit);
        packet_data_encrypted.payload.integrity = enc_p2p_get_integrity_value(&packet_data_encrypted.payload);
        uint8_t encrypted_buffer[ENC_BLOCK_SIZE] = { 0 };
        enc_p2p_payload_bin(encrypted_buffer, &packet_data_encrypted.payload);
        memcpy(packet_data_encrypted.payload.raw, encrypted_buffer, ENC_BLOCK_SIZE);

        tx_size = sizeof(packet_data_encrypted);
        memcpy(tx_payload, &packet_data_encrypted, tx_size);
    } else if (settings_get_is_extended_packet() == true) {
        packet_data_extended_t packet_data_extended = {
            .id1 = settings_get_id_1(),
            .id2 = settings_get_id_2(),
            .lat_32bit = { 0 },
            .lon_32bit = { 0 },
            .version = PACKET_DATA_VERSION_EXTENDED,
            .vbat = _pack_vbat(bsp_battery_get_voltage()),
            .speed_mps = (uint8_t)gnss_data->speed_mps,
            .alt = gnss_data->alt,
        };
        _pack_lat_lon_32(gnss_data->lat, packet_data_extended.lat_32bit);
        _pack_lat_lon_32(gnss_data->lon, packet_data_extended.lon_32bit);
        tx_size = sizeof(packet_data_extended);
        memcpy(tx_payload, &packet_data_extended, tx_size);
    } else {
        packet_data_short_t packet_data_short = {
            .id1 = settings_get_id_1(),
            .id2 = settings_get_id_2(),
            .lat_32bit = { 0 },
            .lon_32bit = { 0 },
            .version = PACKET_DATA_VERSION_SHORT,
            .vbat = _pack_vbat(bsp_battery_get_voltage()),
        };
        _pack_lat_lon_32(gnss_data->lat, packet_data_short.lat_32bit);
        _pack_lat_lon_32(gnss_data->lon, packet_data_short.lon_32bit);
        tx_size = sizeof(packet_data_short);
        memcpy(tx_payload, &packet_data_short, tx_size);
    }

    LOG_DEBUG_ARRAY_BLUE("Send data", tx_payload, tx_size);
    if (subghz_radio_send_block(tx_payload, tx_size) == SUBGHZ_APP_RESULT_ERROR) {
        LOG_ERROR("Failed to send LoRa message");
    }
}

/* -------------------------------------------------------------------------- */

static void _send_gnss_data(send_gnss_data_t const *gnss_data) {

    LOG_DEBUG(LOG_COLOR(LOG_COLOR_BLUE) "Data to send: %02lu,%03lu,%lf,%lf,%lu,%lu,%lu",
              settings_get_id_1(),
              settings_get_id_2(),
              gnss_data->lat,
              gnss_data->lon,
              (uint32_t)gnss_data->alt,
              (uint32_t)gnss_data->speed_mps,
              bsp_battery_get_voltage());

    if (settings_get_is_lorawan_mode()) {
        _send_gnss_data_by_lorawan(gnss_data);
    } else {
        _send_gnss_data_by_p2p_non_text_data(gnss_data);
    }
}

/* -------------------------------------------------------------------------- */

static void _status_print(bool is_blink_enable) {

    static uint32_t _blink_ts = 0;
    static uint32_t _print_ts = 0;
    static uint32_t _vbat_measure_ts = 0;

    if (bsp_gpio_is_usb_charger_connect() == true) {
        if (bsp_gpio_is_charger_high() == true) {
            /* Charging complete indication*/
            bsp_gpio_led_on();
            is_blink_enable = false;
        } else {
            bsp_gpio_led_off();
        }
    }

    if ((bsp_get_ticks() - _vbat_measure_ts) >= BATTERY_CHECK_PERIOD_MS) {
        _vbat_measure_ts = bsp_get_ticks();
        bsp_battery_measure();
    }

    if (_is_power_off_request == true) {
        return;
    }

    /* Debug blink period 5s */
    if ((bsp_get_ticks() - _blink_ts) >= (5 * 1000ULL)) {
        _blink_ts = bsp_get_ticks();
        if (is_blink_enable == true) {
            _led_blink_1x();
        }
    }

    /* Print period 1s or 5s for debug */
    if ((bsp_get_ticks() - _print_ts) >= (1 * 1000ULL + (DEBUG_BUILD * 4000ULL))) {
        _print_ts = bsp_get_ticks();

        if (settings_is_debug_output() == true) {
            LOG(LOG_MASK_USER4,
                "5V=%d,VBAT=%lu,Fix=%d,LAT=%f,LON=%f,ALT=%.2f,MPS=%.2f,PDOP=%0.2f,HDOP=%0.2f,VDOP=%0.2f",
                bsp_gpio_is_usb_charger_connect(),
                bsp_battery_get_voltage(),
                _gnss.fix_mode,
                _gnss.latitude,
                _gnss.longitude,
                _gnss.altitude,
                lwgps_to_speed(_gnss.speed, lwgps_speed_mps),
                _gnss.dop_p,
                _gnss.dop_h,
                _gnss.dop_v);
        }
    }
}

/* -------------------------------------------------------------------------- */

static void _gtrace_load(send_gnss_data_t *data) {
    _gtrace_init();
    _gnss_trace_wakeup_counter_inc();

    /* Get latest coords from storage to send it if GNSS can't catch FIX or battery to low */
    size_t records_count = gtrace_get_record_count(&_gtrace);
    if (records_count > 0) {
        gtrace_record_t record = { 0 };
        gtrace_get_record(&_gtrace, records_count - 1, &record);

        data->lat = record.latitude;
        data->lon = record.longitude;
        data->alt = record.alt;
        data->speed_mps = record.speed_mps;
    }
}

/* -------------------------------------------------------------------------- */

int main(void) {

    bsp_init();
    bsp_clock_init();
    bsp_gpio_init();
    bsp_uart_debug_init();
    bsp_rtc_init();

#if LOG_ENABLED == 1U
    const log_mask_t LOG_MASK = DEBUG_BUILD == 1 ? LOG_MASK_ALL : LOG_MASK_OFF;
    log_init(LOG_MASK, &LOG_IO_INTERFACE);
#endif  // LOG_ENABLED == 1U

    LOG_INFO("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
    LOG_INFO("LokoAir V2 Started, %s %s Build %d.%d",
             __TIME__,
             __DATE__,
             FIRMWARE_VERSION_MAJOR,
             FIRMWARE_VERSION_MINOR);
    LOG_INFO("Branch: " GIT_BRANCH_NAME);
    LOG_INFO("Reset reason: %s", DEBUG_START_INFO_STR[bsp_get_start_reason()]);
    LOG_INFO("MCU Frequency: %ldMHz", bsp_clock_cpu_core_freq_hz() / 1000000UL);
    LOG_INFO("Is 5V Connected: %d", bsp_gpio_is_usb_charger_connect());

    mcu_flash_print_info();

    settings_init(&SETTINGS_IO);

    queue_init(QHEAD(_debug_rx_queue), QUEUE_DEBUG_RX_SIZE);
    queue_init(QHEAD(_gnss_rx_queue), QUEUE_RX_GNSS_SIZE);
    lwgps_init(&_gnss);

    UTIL_TIMER_Init();

    _detect_wakeup_reason();
    _is_enable_by_button = bsp_get_start_reason() == BSP_START_REASON_BUTTON;
    if (_is_enable_by_button == true) {
        _led_blink_3x();
    }

#if LOG_ENABLED == 1U
    if (settings_is_debug_output() == true) {
        log_set_output_mask(LOG_MASK_ALL);
    }
#endif  // LOG_ENABLED == 1U

    uint32_t no_fix_ts = bsp_get_ticks();

    send_gnss_data_t gnss_data = {
        .lat = 0.f,
        .lon = 0.f,
        .alt = 0.f,
        .speed_mps = 0,
    };

    LOG_DEBUG("Application started...");
    for (;;) {

        switch (_system_state) {
            case SYSTEM_STATE_INIT: {
                LOG_DEBUG("System init, VBAT %d", bsp_battery_get_voltage());
                /* Do not enable GNSS module when battery low, just send latest coordinates */
                /* First battery measurement occur when button pressed */
                if (_is_battery_voltage_critical_low() == true) {
                    LOG_DEBUG("Battery voltage %" PRIu32, bsp_battery_get_voltage());
                    if (bsp_gpio_is_usb_charger_connect() == true) {
                        LOG_DEBUG("Battery critical low, switch to tickle charge mode");
                        _switch_mode(SYSTEM_STATE_TICKLE_CHARGE_MODE);
                    } else {
                        LOG_DEBUG("Battery critical low, shutdown");
                        _switch_mode(SYSTEM_STATE_SHUTDOWN);
                    }
                } else if ((_is_battery_voltage_low() == true) && (bsp_gpio_is_usb_charger_connect() == false)) {
                    LOG_INFO("Battery low, do not enable GNSS module, just send lates data");

                    if (settings_get_is_lorawan_mode()) {
                        lorawan_init();
                    }
                    _gtrace_load(&gnss_data);
                    _switch_mode(SYSTEM_STATE_SEND_DATA);
                } else {
                    _gnss_init();

                    if (settings_get_is_lorawan_mode()) {
                        lorawan_init();
                    }
                    _gtrace_load(&gnss_data);
                    _switch_mode(SYSTEM_STATE_WAIT_FOR_GPS_FIX);
                }
            } break;
            case SYSTEM_STATE_WAIT_FOR_GPS_FIX: {
                bool is_switch_next_state = false;
                if ((_gnss.fix_mode >= 3) && (_gnss.is_valid == 1)) {

                    LOG_INFO("GNSS data captured!!!");
                    /* Copy normal gnss coords */
                    gnss_data.lat = _gnss.latitude;
                    gnss_data.lon = _gnss.longitude;
                    gnss_data.alt = (uint16_t)_gnss.altitude;
                    gnss_data.speed_mps = (uint16_t)lwgps_to_speed(_gnss.speed, lwgps_speed_mps);

                    if (_gnss_trace_wakeup_counter_is_need_save() || (gtrace_get_record_count(&_gtrace) == 0)) {
                        _gnss_trace_save(&_gnss);
                        _gnss_trace_wakeup_counter_reset();
                    }

                    is_switch_next_state = true;
                } else if ((bsp_gpio_is_usb_charger_connect() == false) &&
                           ((bsp_get_ticks() - no_fix_ts) >= NO_FIX_TIMEOUT_MS)) {
                    is_switch_next_state = true;
                }

                /*Disable GNSS module when battery low, just send latest coordinates */
                if (_is_battery_voltage_low()) {
                    LOG_INFO("Battery low, disable GNSS module, just send lates data");
                    is_switch_next_state = true;
                }

                if (is_switch_next_state == true) {
                    if (settings_is_debug_output() == false) {
                        LOG_INFO("GNSS goes to sleep mode");
                        bsp_gpio_gnss_wakeup_enter();
                    }
                    _switch_mode(SYSTEM_STATE_SEND_DATA);
                }
            } break;
            case SYSTEM_STATE_SEND_DATA: {
                _send_gnss_data(&gnss_data);
                _switch_mode(SYSTEM_STATE_SHUTDOWN);
            } break;
            case SYSTEM_STATE_SHUTDOWN: {
                if (bsp_gpio_is_usb_charger_connect() == true) {
                    LOG_INFO("Switch to Charging mode");
                    _prepare_to_sleep();
                    _switch_mode(SYSTEM_STATE_SHUTDOWN_CHARGING);
                } else {
                    uint32_t time_off =
                        (_is_power_off_request == true) ? SHUTDOWN_NO_AUTO_WAKEUP : settings_get_auto_wakeup_period_s();
                    if (bsp_get_start_reason() == BSP_START_REASON_CHARGER) {
                        time_off = SHUTDOWN_KEEP_WAKEUP;
                    }
                    _shutdown(time_off);
                    LOG_ERROR("If you see it, shutdown is broken");
                }
            } break;
            case SYSTEM_STATE_SHUTDOWN_CHARGING: {
                if (bsp_gpio_is_usb_charger_connect() == false) {
                    _switch_mode(SYSTEM_STATE_SHUTDOWN);
                }
                /* Allow uart receive some data, anyway we will sleep all time */
                bsp_delay_ms(100);

            } break;
            case SYSTEM_STATE_TICKLE_CHARGE_MODE: {
                while (_is_battery_voltage_critical_low() == true) {
                    /* Hangs here until the battery level is critically low and the USB connected */
                    if (bsp_gpio_is_usb_charger_connect() == false) {
                        break;
                    }

                    bsp_clock_switch(BSP_CLOCK_CORE_100_KHZ);
                    bsp_lp_sleep();
                    bsp_clock_switch(DEFAULT_FREQ);
                }
                _switch_mode(SYSTEM_STATE_INIT);
            } break;
            default:
                LOG_ERROR("Unknown system state");
                _switch_mode(SYSTEM_STATE_SHUTDOWN);
                break;
        }

        _background_loop();

        /* Reduce consumption in SYSTEM_STATE_SHUTDOWN_CHARGING */
        if (_system_state == SYSTEM_STATE_SHUTDOWN_CHARGING) {
            bsp_clock_switch(BSP_CLOCK_CORE_100_KHZ);
        }

        //"Wait for interrupt" enter MCU core to sleep mode until any interrupt occur
        bsp_lp_sleep();

        if (_system_state == SYSTEM_STATE_SHUTDOWN_CHARGING) {
            bsp_clock_switch(DEFAULT_FREQ);
        }
    }
}
