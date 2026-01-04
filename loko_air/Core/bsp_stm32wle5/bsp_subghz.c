#include "bsp_subghz.h"

#include "stm32wlxx_hal.h"
#include <bsp.h>

SUBGHZ_HandleTypeDef hsubghz;

void subghz_init(void) {
    hsubghz.Init.BaudratePrescaler = SUBGHZSPI_BAUDRATEPRESCALER_2;
    HAL_StatusTypeDef ret = HAL_SUBGHZ_Init(&hsubghz);

    if (ret != HAL_OK) {
        BSP_FATAL(ret, NULL);
    }
}

void subghz_deinit(void) {
    HAL_StatusTypeDef ret = HAL_SUBGHZ_DeInit(&hsubghz);

    if (ret != HAL_OK) {
        BSP_FATAL(ret, NULL);
    }
}

void subghz_interrupt_handler(void) {
    HAL_SUBGHZ_IRQHandler(&hsubghz);
}

void HAL_SUBGHZ_MspInit(SUBGHZ_HandleTypeDef *subghz_handle) {
    __HAL_RCC_SUBGHZSPI_CLK_ENABLE();

    /* SUBGHZ interrupt Init */
    HAL_NVIC_SetPriority(SUBGHZ_Radio_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(SUBGHZ_Radio_IRQn);
}

void HAL_SUBGHZ_MspDeInit(SUBGHZ_HandleTypeDef *subghz_handle) {
    __HAL_RCC_SUBGHZSPI_CLK_DISABLE();

    /* SUBGHZ interrupt Deinit */
    HAL_NVIC_DisableIRQ(SUBGHZ_Radio_IRQn);
}
