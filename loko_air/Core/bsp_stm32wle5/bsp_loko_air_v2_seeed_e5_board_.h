#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*GPIO A*/

#define BUTTON_PIN             LL_GPIO_PIN_0
#define BUTTON_GPIO_PORT       GPIOA
#define BUTTON_PULL_SETTINGS   LL_GPIO_PULL_NO
#define BUTTON_ACTIVE_LEVEL    GPIO_PIN_SET
#define GNSS_WAKEUP_PIN        LL_GPIO_PIN_2
#define GNSS_WAKEUP_GPIO_PORT  GPIOA
#define GNSS_EN_PIN            LL_GPIO_PIN_9
#define GNSS_EN_GPIO_PORT      GPIOA
#define CHARGER_STAT_PIN       LL_GPIO_PIN_10
#define CHARGER_STAT_GPIO_PORT GPIOA
#define TIME_PULSE_PIN         LL_GPIO_PIN_15
#define TIME_PULSE_GPIO_PORT   GPIOA

/*GPIO B*/

#define V_USB_DETECT_PIN          LL_GPIO_PIN_3
#define V_USB_DETECT_GPIO_PORT    GPIOB
#define LED_PIN                   LL_GPIO_PIN_5
#define LED_GPIO_PORT             GPIOB
#define USB_TXD_PIN               LL_GPIO_PIN_6
#define USB_TXD_GPIO_PORT         GPIOB
#define USB_RXD_PIN               LL_GPIO_PIN_7
#define USB_RXD_GPIO_PORT         GPIOB
#define VBAT_MEASURE_PIN          LL_GPIO_PIN_13
#define VBAT_MEASURE_GPIO_PORT    GPIOB
#define VBAT_MEASURE_EN_PIN       LL_GPIO_PIN_15
#define VBAT_MEASURE_EN_GPIO_PORT GPIOB

/*GPIO C*/

#define GPS_RX_PIN       LL_GPIO_PIN_0
#define GPS_RX_GPIO_PORT GPIOC
#define GPS_TX_PIN       LL_GPIO_PIN_1
#define GPS_TX_GPIO_PORT GPIOC

#ifdef __cplusplus
}
#endif
