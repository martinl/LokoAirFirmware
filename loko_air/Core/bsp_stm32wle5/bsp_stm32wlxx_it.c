#include "bsp.h"
#include "bsp_subghz.h"

#include "stm32wlxx_hal.h"
#include "stm32wlxx_ll_lpuart.h"
#include "stm32wlxx_ll_usart.h"

#if 1
#    define _ENTRY_TRACE(...)
#else
#    define _ENTRY_TRACE(...) LOG_DEBUG(LOG_COLOR(LOG_COLOR_RED) "ENTERED TO ISR: %s", __FUNCTION__)
#endif

/*---------------------------------------------------------------------------*/

void NMI_Handler(void) {
    BSP_FATAL(0, "NMI_Handler");
}

/*---------------------------------------------------------------------------*/

void HardFault_Handler(void) {
    BSP_FATAL(0, "HardFault_Handler");
}

/*---------------------------------------------------------------------------*/

void MemManage_Handler(void) {
    BSP_FATAL(0, "MemManage_Handler");
}

/*---------------------------------------------------------------------------*/

void BusFault_Handler(void) {
    BSP_FATAL(0, "BusFault_Handler");
}

/*---------------------------------------------------------------------------*/

void UsageFault_Handler(void) {
    BSP_FATAL(0, "UsageFault_Handler");
}

/*---------------------------------------------------------------------------*/

void SVC_Handler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void DebugMon_Handler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void PendSV_Handler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void SysTick_Handler(void) {
    HAL_IncTick();
}

/*---------------------------------------------------------------------------*/

void DMA1_Channel1_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void DMA1_Channel2_IRQHandler(void) {
#if CONFIG_BSP_USE_ADC == 1
    bsp_adc_dma_handler();
#endif /* CONFIG_BSP_USE_ADC == 1 */
}

/*---------------------------------------------------------------------------*/

void DMA1_Channel3_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void SUBGHZ_Radio_IRQHandler(void) {
#if CONFIG_BSP_USE_RADIO == 1
    subghz_interrupt_handler();
#endif /* CONFIG_BSP_USE_RADIO == 1 */
}

/*---------------------------------------------------------------------------*/

void USART1_IRQHandler(void) {
#if CONFIG_BSP_USE_UART == 1

    if (LL_USART_IsActiveFlag_RXNE(USART1)) {
        /* RXNE flag will be cleared by reading of RDR register (done in call) */
        /* Call function in charge of handling Character reception */
        /* Read Received character. RXNE flag is cleared by reading of RDR register */
        uint8_t byte = LL_USART_ReceiveData8(USART1);
        bsp_uart_debug_byte_received(byte);
    } else if (LL_USART_IsActiveFlag_ORE(USART1)) {
        LL_USART_ClearFlag_ORE(USART1);
    } else if (LL_USART_IsActiveFlag_NE(USART1)) {
        LL_USART_ClearFlag_NE(USART1);
    } else if (LL_USART_IsActiveFlag_FE(USART1)) {
        LL_USART_ClearFlag_FE(USART1);
    } else if (LL_USART_IsActiveFlag_PE(USART1)) {
        LL_USART_ClearFlag_PE(USART1);
    }
#endif /* CONFIG_BSP_USE_UART == 1 */
}

/*---------------------------------------------------------------------------*/

void LPUART1_IRQHandler(void) {
#if CONFIG_BSP_USE_UART == 1
    if (LL_LPUART_IsActiveFlag_RXNE(LPUART1)) {
        /* RXNE flag will be cleared by reading of RDR register (done in call) */
        /* Call function in charge of handling Character reception */
        /* Read Received character. RXNE flag is cleared by reading of RDR register */
        uint8_t byte = LL_LPUART_ReceiveData8(LPUART1);
        bsp_uart_gnss_byte_received(byte);
    } else if (LL_LPUART_IsActiveFlag_ORE(LPUART1)) {
        LL_LPUART_ClearFlag_ORE(LPUART1);
    } else if (LL_LPUART_IsActiveFlag_NE(LPUART1)) {
        LL_LPUART_ClearFlag_NE(LPUART1);
    } else if (LL_LPUART_IsActiveFlag_FE(LPUART1)) {
        LL_LPUART_ClearFlag_FE(LPUART1);
    } else if (LL_LPUART_IsActiveFlag_PE(LPUART1)) {
        LL_LPUART_ClearFlag_PE(LPUART1);
    }
#endif /* CONFIG_BSP_USE_UART == 1 */
}

/*---------------------------------------------------------------------------*/

void TAMP_STAMP_LSECSS_SSRU_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void RTC_Alarm_IRQHandler(void) {
#if CONFIG_BSP_USE_RTC == 1
    extern RTC_HandleTypeDef _rtc;
    HAL_RTC_AlarmIRQHandler(&_rtc);
#endif /* CONFIG_BSP_USE_RTC */
}

/*---------------------------------------------------------------------------*/

void WWDG_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void PVD_PVM_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void RTC_WKUP_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void FLASH_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void RCC_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void EXTI0_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void EXTI1_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void EXTI2_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void EXTI3_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void EXTI4_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void DMA1_Channel4_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void DMA1_Channel5_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void DMA1_Channel6_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void DMA1_Channel7_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void ADC_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void DAC_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void COMP_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void EXTI9_5_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void TIM1_BRK_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void TIM1_UP_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void TIM1_TRG_COM_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void TIM1_CC_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void TIM2_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void TIM16_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void TIM17_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void I2C1_EV_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void I2C1_ER_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void I2C2_EV_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void I2C2_ER_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void SPI1_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void SPI2_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void USART2_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void LPTIM1_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void LPTIM2_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void EXTI15_10_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void LPTIM3_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void SUBGHZSPI_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void HSEM_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void I2C3_EV_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void I2C3_ER_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void AES_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void RNG_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void PKA_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void DMA2_Channel1_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void DMA2_Channel2_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void DMA2_Channel3_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void DMA2_Channel4_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void DMA2_Channel5_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void DMA2_Channel6_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void DMA2_Channel7_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/

void DMAMUX1_OVR_IRQHandler(void) {
    _ENTRY_TRACE();
}

/*---------------------------------------------------------------------------*/
