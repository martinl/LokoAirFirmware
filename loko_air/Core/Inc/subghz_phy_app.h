#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#define LORA_BANDWIDTH             0  /* [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved] */
#define LORA_SPREADING_FACTOR      12 /* [SF7..SF12] */
#define LORA_CODINGRATE            1  /* [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8] */
#define LORA_PREAMBLE_LENGTH       15 /* Same for Tx and Rx */
#define LORA_SYMBOL_TIMEOUT        10 /* Symbols */
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON       false

typedef struct {
    uint32_t freq;
    int8_t tx_power;
} app_lora_params_t;

typedef enum {
    SUBGHZ_APP_RESULT_OK,
    SUBGHZ_APP_RESULT_ERROR,
} app_lora_result_t;

void subghz_radio_init(app_lora_params_t *params);
void subghz_radio_deinit(void);
app_lora_result_t subghz_radio_send_block(const uint8_t *data, size_t size);

#ifdef __cplusplus
}
#endif
