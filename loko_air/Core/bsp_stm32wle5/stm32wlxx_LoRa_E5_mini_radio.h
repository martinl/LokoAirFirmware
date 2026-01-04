#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32wlxx_LoRa_E5_mini_errno.h"
#include "stm32wlxx_hal.h"

typedef enum {
    RADIO_SWITCH_OFF = 0,
    RADIO_SWITCH_RX = 1,
    RADIO_SWITCH_RFO_LP = 2,
    RADIO_SWITCH_RFO_HP = 3,
} BSP_RADIO_Switch_TypeDef;

#define RADIO_CONF_RFO_LP_HP 0U
#define RADIO_CONF_RFO_LP    1U
#define RADIO_CONF_RFO_HP    2U

#define RADIO_CONF_TCXO_NOT_SUPPORTED 0U
#define RADIO_CONF_TCXO_SUPPORTED     1U

#define RADIO_CONF_DCDC_NOT_SUPPORTED 0U
#define RADIO_CONF_DCDC_SUPPORTED     1U

#define RF_SW_CTRL1_PIN               GPIO_PIN_4
#define RF_SW_CTRL1_GPIO_PORT         GPIOA
#define RF_SW_CTRL1_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define RF_SW_RX_GPIO_CLK_DISABLE()   __HAL_RCC_GPIOA_CLK_DISABLE()

#define RF_SW_CTRL2_PIN                GPIO_PIN_5
#define RF_SW_CTRL2_GPIO_PORT          GPIOA
#define RF_SW_CTRL2_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOA_CLK_ENABLE()
#define RF_SW_CTRL2_GPIO_CLK_DISABLE() __HAL_RCC_GPIOA_CLK_DISABLE()

#define RF_TCXO_VCC_PIN           GPIO_PIN_0
#define RF_TCXO_VCC_GPIO_PORT     GPIOB
#define RF_TCXO_VCC_CLK_ENABLE()  __HAL_RCC_GPIOB_CLK_ENABLE()
#define RF_TCXO_VCC_CLK_DISABLE() __HAL_RCC_GPIOB_CLK_DISABLE()

int32_t bsp_radio_init(void);
int32_t bsp_radio_de_init(void);
int32_t bsp_radio_config_rf_switch(BSP_RADIO_Switch_TypeDef config);
int32_t bsp_radio_get_tx_config(void);
int32_t bsp_radio_is_tcxo(void);
int32_t bsp_radio_is_dcdc(void);

#ifdef __cplusplus
}
#endif
