#include "bsp.h"

#include "stm32wlxx_hal.h"

#include "stm32wlxx_ll_bus.h"
#include "stm32wlxx_ll_cortex.h"
#include "stm32wlxx_ll_dma.h"
#include "stm32wlxx_ll_exti.h"
#include "stm32wlxx_ll_gpio.h"
#include "stm32wlxx_ll_lpuart.h"
#include "stm32wlxx_ll_pwr.h"
#include "stm32wlxx_ll_rcc.h"
#include "stm32wlxx_ll_system.h"
#include "stm32wlxx_ll_usart.h"
#include "stm32wlxx_ll_utils.h"

#include <bsp_flash_layout.h>

/* -------------------------------------------------------------------------- */

typedef void(application_t)(void);

typedef struct vector_s {
    uint32_t stack_addr;
    application_t *entry_point;
} vector_t;

/* -------------------------------------------------------------------------- */

void bsp_launch_app(void) {
    const uint32_t ADDR = FLASH_APP_PAGE_ADDR;
    const vector_t *vector_p = (vector_t *)ADDR;

    if (vector_p->stack_addr == 0U) {
        return;
    }

    if ((vector_p->stack_addr & 0xFF000000) != 0x20000000) {
        return;
    }

    if ((uint32_t)vector_p->entry_point < ADDR) {
        return;
    }

    for (IRQn_Type irq = 0; irq <= DMAMUX1_OVR_IRQn; irq++) {
        NVIC_DisableIRQ(irq);
    }

    SCB->VTOR = ADDR;
#if defined(__CC_ARM)
    __asm("msr msp, vector_p->stack_addr;");
    vector_p->entry_point();
#elif defined(__ICCARM__) || defined(__GNUC__)
    asm("\n"
        "msr msp, %0; \n"
        "bx %1;"
        :
        : "r"(vector_p->stack_addr), "r"(vector_p->entry_point));
#else
#    warning need implementation
#endif /* __ICCARM__ */
}

/* -------------------------------------------------------------------------- */

void bsp_disable_irq(void) {
    __disable_irq();
}

/* -------------------------------------------------------------------------- */

void bsp_enable_irq(void) {
    __enable_irq();
}

/* -------------------------------------------------------------------------- */

uint32_t bsp_get_ticks(void) {
    return HAL_GetTick();
}

/* -------------------------------------------------------------------------- */

void bsp_delay_ms(uint32_t ms) {
    HAL_Delay(ms);
}

/* -------------------------------------------------------------------------- */

void bsp_soft_breakpoint(void) {
    if (bsp_is_debug_session()) {
        __ASM volatile("BKPT 0");
    }
}

/* -------------------------------------------------------------------------- */

bool bsp_is_debug_session(void) {
    return (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) == 0U ? false : true;
}

/* -------------------------------------------------------------------------- */

void bsp_system_reset(void) {
    NVIC_SystemReset();
}

/* -------------------------------------------------------------------------- */

uint64_t bsp_get_uid64(void) {
    static struct {
        union {
            uint64_t word;
            uint8_t bytes[8];
        };
    } _uid64 = { 0 };

    if (_uid64.word == 0) {
        uint32_t val = LL_FLASH_GetUDN();
        _uid64.bytes[7] = val & 0xFF;
        _uid64.bytes[6] = (val >> 8) & 0xFF;
        _uid64.bytes[5] = (val >> 16) & 0xFF;
        _uid64.bytes[4] = (val >> 24) & 0xFF;

        _uid64.bytes[3] = LL_FLASH_GetDeviceID() & 0xFF;

        val = LL_FLASH_GetSTCompanyID();
        _uid64.bytes[2] = val & 0xFF;
        _uid64.bytes[1] = (val >> 8) & 0xFF;
        _uid64.bytes[0] = (val >> 16) & 0xFF;
    }

    return _uid64.word;
}

/* -------------------------------------------------------------------------- */

void bsp_fatal_error(size_t code, char *note, char *file, size_t line) {
    LOG_ERROR("\r\nFatal error! %s:%d - %d, %s", file, line, code, note == NULL ? "" : note);
    bsp_soft_breakpoint();
    LOG_ERROR("Reset system");
    bsp_system_reset();
}

/* -------------------------------------------------------------------------- */

bsp_start_reason_t bsp_get_start_reason(void) {

    static bsp_start_reason_t _power_on_reason = BSP_START_REASON_COUNT;

    if (_power_on_reason != BSP_START_REASON_COUNT) {
        return _power_on_reason;
    }

    /* Workaround PWR_SR1 do not keep PWR_SR1_WUF1 or PWR_SR1_WUF3 state, SR1 is zero when it happened, bug when debug
     * session it's work correctly, I don't know the reason so use  BSP_START_REASON_BUTTON as default state*/
    uint32_t pwr_sr1 = PWR->SR1;

    _power_on_reason = BSP_START_REASON_BUTTON;

    if (bsp_gpio_is_usb_charger_connect()) {
        _power_on_reason = BSP_START_REASON_CHARGER;
        LOG_DEBUG("PWR_SR1_WUF3");
    } else if ((PWR->CR4 & PWR_WAKEUP_PIN3_LOW) == PWR_WAKEUP_PIN3_LOW) {
        _power_on_reason = BSP_START_REASON_CHARGER;
        LOG_DEBUG("PWR_WAKEUP_PIN3_LOW");
    }

    if (pwr_sr1 & PWR_SR1_WUFI) {
        _power_on_reason = BSP_START_REASON_TIMER_ALARM;
        LOG_DEBUG("PWR_SR1_WUFI");
    }

#if 0

    if (rcc_csr & RCC_CSR_WWDGRSTF) {
        _power_on_reason = BSP_START_REASON_RESET;
        LOG_DEBUG("RCC_CSR_WWDGRSTF");
    }

    if (rcc_csr & RCC_CSR_IWDGRSTF) {
        _power_on_reason = BSP_START_REASON_RESET;
        LOG_DEBUG("RCC_CSR_IWDGRSTF");
    }

    if (rcc_csr & RCC_CSR_SFTRSTF) {
        _power_on_reason = BSP_START_REASON_RESET;
        LOG_DEBUG("RCC_CSR_SFTRSTF");
    }

    if (rcc_csr & RCC_CSR_BORRSTF) {
        _power_on_reason = BSP_START_REASON_POWER_ON;
        LOG_DEBUG("RCC_CSR_BORRSTF");
    }

    if (rcc_csr & RCC_CSR_PINRSTF) {
        _power_on_reason = BSP_START_REASON_POWER_ON;
        LOG_DEBUG("RCC_CSR_PINRSTF");
    }

    if (rcc_csr & RCC_CSR_OBLRSTF) {
        _power_on_reason = BSP_START_REASON_RESET;
        LOG_DEBUG("RCC_CSR_OBLRSTF");
    }

    if (pwr_sr1 & PWR_SR1_WUF1) {
        _power_on_reason = BSP_START_REASON_BUTTON;
        LOG_DEBUG("PWR_SR1_WUF1");
    }

    if (pwr_sr1 & PWR_SR1_WUF3) {
        _power_on_reason = BSP_START_REASON_CHARGER;
        LOG_DEBUG("PWR_SR1_WUF3");
    }

    if (pwr_sr1 & PWR_SR1_WUFI) {
        _power_on_reason = BSP_START_REASON_TIMER_ALARM;
        LOG_DEBUG("PWR_SR1_WUFI");
    }
    if (rcc_csr & RCC_CSR_LPWRRSTF) {
        return BSP_RESET_REASON_LOW_POWER;
    }

    if (pwr_sr1 & PWR_SR1_WUF2) {
        return BSP_RESET_REASON_WAKEUP_PIN_2;
    }

    if (rcc_csr & RCC_CSR_RFILARSTF) {
        return BSP_RESET_REASON_RADIO_ILLEGAL;
    }

    if (pwr_sr1 & PWR_SR1_WPVDF) {
        return BSP_RESET_REASON_WAKEUP_PVD;
    }

    if (pwr_sr1 & PWR_SR1_WRFBUSYF) {
        return BSP_RESET_REASON_WAKEUP_RADIO_BUSY;
    }
#endif

    return _power_on_reason;
}

/* -------------------------------------------------------------------------- */

void bsp_init(void) {
    __HAL_RCC_DMAMUX1_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();

    SCB->VTOR = TARGET_BASE_ADDR;

    /* DMA interrupt init */
    /* DMA1_Channel1_IRQn interrupt configuration */
    NVIC_SetPriority(DMA1_Channel1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);
    /* DMA1_Channel2_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
    /* DMA1_Channel3_IRQn interrupt configuration */
    NVIC_SetPriority(DMA1_Channel3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(DMA1_Channel3_IRQn);

    HAL_Init();

    __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);
}

/* -------------------------------------------------------------------------- */
