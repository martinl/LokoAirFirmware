#include "bsp.h"

#include "bsp_board.h"
#include "stm32wlxx_hal.h"
#include "stm32wlxx_ll_gpio.h"

/* -------------------------------------------------------------------------- */

void bsp_gpio_init(void) {
    /* GPIO Ports Clock Enable */
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);

    LL_GPIO_InitTypeDef all_analog = {
        .Pin = LL_GPIO_PIN_ALL,
        .Mode = LL_GPIO_MODE_ANALOG,
        .Speed = LL_GPIO_SPEED_FREQ_LOW,
        .OutputType = LL_GPIO_OUTPUT_OPENDRAIN,
        .Pull = LL_GPIO_PULL_NO,
    };

    if (bsp_is_debug_session() == false) {
        LL_GPIO_Init(GPIOA, &all_analog);
        LL_GPIO_Init(GPIOB, &all_analog);
        LL_GPIO_Init(GPIOC, &all_analog);
    }

    LL_GPIO_ResetOutputPin(VBAT_MEASURE_EN_GPIO_PORT, VBAT_MEASURE_EN_PIN);
    LL_GPIO_SetOutputPin(GNSS_EN_GPIO_PORT, GNSS_EN_PIN);
    LL_GPIO_SetOutputPin(GNSS_WAKEUP_GPIO_PORT, GNSS_WAKEUP_PIN);
    LL_GPIO_SetOutputPin(LED_GPIO_PORT, LED_PIN);

    LL_GPIO_InitTypeDef gpio_config = {
        .Mode = LL_GPIO_MODE_OUTPUT,
        .Speed = LL_GPIO_SPEED_FREQ_LOW,
        .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
        .Pull = LL_GPIO_PULL_NO,
    };

    /* outputs push pull */
    gpio_config.Pin = VBAT_MEASURE_EN_PIN;
    LL_GPIO_Init(VBAT_MEASURE_EN_GPIO_PORT, &gpio_config);

    gpio_config.Pin = GNSS_WAKEUP_PIN;
    LL_GPIO_Init(GNSS_WAKEUP_GPIO_PORT, &gpio_config);

#if CONFIG_LOKO_AIR_SEEED_E5 == 1
    gpio_config.Pin = GNSS_EN_PIN;
    LL_GPIO_Init(GNSS_EN_GPIO_PORT, &gpio_config);
#endif

    /* outputs open drain */
    gpio_config.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;

#if CONFIG_LOKO_AIR == 1
    gpio_config.Pin = GNSS_EN_PIN;
    LL_GPIO_Init(GNSS_EN_GPIO_PORT, &gpio_config);
#endif

    gpio_config.Pin = LED_PIN;
    LL_GPIO_Init(LED_GPIO_PORT, &gpio_config);

    /* input pin config */
    gpio_config.Mode = LL_GPIO_MODE_INPUT;
    gpio_config.Pin = BUTTON_PIN;
    gpio_config.Pull = BUTTON_PULL_SETTINGS;
    LL_GPIO_Init(BUTTON_GPIO_PORT, &gpio_config);

    gpio_config.Pull = LL_GPIO_PULL_NO;
    gpio_config.Pin = V_USB_DETECT_PIN;
    LL_GPIO_Init(V_USB_DETECT_GPIO_PORT, &gpio_config);

    /* Has external pull-up */
    gpio_config.Pin = CHARGER_STAT_PIN;
    LL_GPIO_Init(CHARGER_STAT_GPIO_PORT, &gpio_config);
}

/* -------------------------------------------------------------------------- */

bool bsp_gpio_is_button_pressed(void) {
    return HAL_GPIO_ReadPin(BUTTON_GPIO_PORT, BUTTON_PIN) == BUTTON_ACTIVE_LEVEL;
}

/* -------------------------------------------------------------------------- */

bool bsp_gpio_is_charger_high(void) {
    return HAL_GPIO_ReadPin(CHARGER_STAT_GPIO_PORT, CHARGER_STAT_PIN) == GPIO_PIN_SET;
}

/* -------------------------------------------------------------------------- */

bool bsp_gpio_is_usb_charger_connect(void) {
    return HAL_GPIO_ReadPin(V_USB_DETECT_GPIO_PORT, V_USB_DETECT_PIN) == GPIO_PIN_SET;
}

/* -------------------------------------------------------------------------- */

void bsp_gpio_led_on(void) {
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_PIN, GPIO_PIN_RESET);
}

/* -------------------------------------------------------------------------- */

void bsp_gpio_led_off(void) {
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_PIN, GPIO_PIN_SET);
}

/* -------------------------------------------------------------------------- */

void bsp_gpio_led_toggle(void) {
    HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
}

/* -------------------------------------------------------------------------- */

void bsp_gpio_gnss_on(void) {
#if CONFIG_LOKO_AIR_SEEED_E5 == 1
    HAL_GPIO_WritePin(GNSS_EN_GPIO_PORT, GNSS_EN_PIN, GPIO_PIN_SET);
#else
    HAL_GPIO_WritePin(GNSS_EN_GPIO_PORT, GNSS_EN_PIN, GPIO_PIN_RESET);
#endif  // CONFIG_LOKO_AIR_SEEED_E5 == 1
}

/* -------------------------------------------------------------------------- */

void bsp_gpio_gnss_off(void) {
#if CONFIG_LOKO_AIR_SEEED_E5 == 1
    HAL_GPIO_WritePin(GNSS_EN_GPIO_PORT, GNSS_EN_PIN, GPIO_PIN_RESET);
#else
    HAL_GPIO_WritePin(GNSS_EN_GPIO_PORT, GNSS_EN_PIN, GPIO_PIN_SET);
#endif  // CONFIG_LOKO_AIR_SEEED_E5 == 1
}

/* -------------------------------------------------------------------------- */

void bsp_gpio_gnss_wakeup_enter(void) {
    HAL_GPIO_WritePin(GNSS_WAKEUP_GPIO_PORT, GNSS_WAKEUP_PIN, GPIO_PIN_RESET);
}

/* -------------------------------------------------------------------------- */

void bsp_gpio_gnss_wakeup_leave(void) {
    HAL_GPIO_WritePin(GNSS_WAKEUP_GPIO_PORT, GNSS_WAKEUP_PIN, GPIO_PIN_SET);
}

/* -------------------------------------------------------------------------- */

void bsp_gpio_vbat_measure_on(void) {
    HAL_GPIO_WritePin(VBAT_MEASURE_EN_GPIO_PORT, VBAT_MEASURE_EN_PIN, GPIO_PIN_SET);
}

/* -------------------------------------------------------------------------- */

void bsp_gpio_vbat_measure_off(void) {
    HAL_GPIO_WritePin(VBAT_MEASURE_EN_GPIO_PORT, VBAT_MEASURE_EN_PIN, GPIO_PIN_RESET);
}

/* -------------------------------------------------------------------------- */
