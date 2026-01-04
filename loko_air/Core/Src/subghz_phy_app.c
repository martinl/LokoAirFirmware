#include "subghz_phy_app.h"
#include "bsp.h"
#include "bsp_subghz.h"
#include "radio.h"
#include <stdbool.h>

/* -------------------------------------------------------------------------- */

#define _ENTRY_TRACE(...) LOG_DEBUG(LOG_COLOR(LOG_COLOR_RED) "ENTERED TO SUBGHZ Event: %s", __FUNCTION__)

/* -------------------------------------------------------------------------- */

static volatile bool _is_sent = false;
static volatile bool _is_timeout = false;

/* -------------------------------------------------------------------------- */

static void _on_tx_done(void) {
    //    _ENTRY_TRACE();
    _is_sent = true;
}

/* -------------------------------------------------------------------------- */

static void _on_rx_done(uint8_t *payload, uint16_t size, int16_t rssi, int8_t lora_snr_fsk_cfo) {
    _ENTRY_TRACE();
}

/* -------------------------------------------------------------------------- */

static void _on_tx_timeout(void) {
    _ENTRY_TRACE();
    _is_timeout = true;
}

/* -------------------------------------------------------------------------- */

static void _on_rx_timeout(void) {
    _ENTRY_TRACE();
}

/* -------------------------------------------------------------------------- */

static void _on_rx_error(void) {
    _ENTRY_TRACE();
}

/* -------------------------------------------------------------------------- */

void subghz_radio_init(app_lora_params_t *params) {
    static RadioEvents_t _radio_events = {
        .TxDone = _on_tx_done,
        .RxDone = _on_rx_done,
        .TxTimeout = _on_tx_timeout,
        .RxTimeout = _on_rx_timeout,
        .RxError = _on_rx_error,
    };

    Radio.Init(&_radio_events);

    uint32_t fdev = 0;
    uint32_t bandwidth = LORA_BANDWIDTH;
    uint32_t datarate = LORA_SPREADING_FACTOR;
    uint8_t coderate = LORA_CODINGRATE;
    uint16_t preamble_len = LORA_PREAMBLE_LENGTH;
    bool fix_len = LORA_FIX_LENGTH_PAYLOAD_ON;
    bool crc_on = true;
    bool freq_hop_on = false;
    uint8_t hop_period = 0;
    bool iq_inverted = LORA_IQ_INVERSION_ON;
    uint32_t timeout = 5000;

    Radio.SetChannel(params->freq);
    Radio.SetTxConfig(MODEM_LORA,
                      params->tx_power,
                      fdev,
                      bandwidth,
                      datarate,
                      coderate,
                      preamble_len,
                      fix_len,
                      crc_on,
                      freq_hop_on,
                      hop_period,
                      iq_inverted,
                      timeout);
}

/* -------------------------------------------------------------------------- */

void subghz_radio_deinit(void) {
    subghz_deinit();
}

/* -------------------------------------------------------------------------- */

app_lora_result_t subghz_radio_send_block(const uint8_t *data, size_t size) {
    _is_sent = false;
    _is_timeout = false;
    Radio.Send((uint8_t *)data, size);

    size_t ts = bsp_get_ticks();

    while ((_is_sent == false) && (_is_timeout == false)) {
        bsp_lp_sleep();

        if ((bsp_get_ticks() - ts) > 7000) {
            break;
        }
    }

    return _is_sent == false ? SUBGHZ_APP_RESULT_ERROR : SUBGHZ_APP_RESULT_OK;
}

/* -------------------------------------------------------------------------- */
