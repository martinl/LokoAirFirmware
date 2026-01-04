
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

#include "bsp.h"
#include "bsp_board.h"

/* -------------------------------------------------------------------------- */

void bsp_uart_gnss_init(void) {
    LL_RCC_SetLPUARTClockSource(LL_RCC_LPUART1_CLKSOURCE_SYSCLK);  // LSE allow max baudrate=9600

    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_LPUART1);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);

    LL_GPIO_InitTypeDef gpio_init = {
        .Mode = LL_GPIO_MODE_ALTERNATE,
        .Speed = LL_GPIO_SPEED_FREQ_LOW,
        .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
        .Pull = LL_GPIO_PULL_NO,
        .Alternate = LL_GPIO_AF_8,
    };
    gpio_init.Pin = GPS_RX_PIN;
    LL_GPIO_Init(GPS_RX_GPIO_PORT, &gpio_init);
    gpio_init.Pin = GPS_TX_PIN;
    LL_GPIO_Init(GPS_TX_GPIO_PORT, &gpio_init);

#if ENMABLE_LPUART_DMA_TX == 1
    /* LPUART1_TX Init */
    LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_3, LL_DMAMUX_REQ_LPUART1_TX);
    LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_3, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MDATAALIGN_BYTE);
#endif

    LL_LPUART_InitTypeDef uart_init = {
        .PrescalerValue = LL_LPUART_PRESCALER_DIV1,
        .BaudRate = 9600,
        .DataWidth = LL_LPUART_DATAWIDTH_8B,
        .StopBits = LL_LPUART_STOPBITS_1,
        .Parity = LL_LPUART_PARITY_NONE,
        .TransferDirection = LL_LPUART_DIRECTION_TX_RX,
        .HardwareFlowControl = LL_LPUART_HWCONTROL_NONE,
    };
    LL_LPUART_Init(LPUART1, &uart_init);
    LL_LPUART_SetTXFIFOThreshold(LPUART1, LL_LPUART_FIFOTHRESHOLD_1_8);
    LL_LPUART_SetRXFIFOThreshold(LPUART1, LL_LPUART_FIFOTHRESHOLD_1_8);
    LL_LPUART_Enable(LPUART1);

    /* Polling LPUART1 initialisation */
    while ((!(LL_LPUART_IsActiveFlag_TEACK(LPUART1))) || (!(LL_LPUART_IsActiveFlag_REACK(LPUART1)))) {
        // Wait for
    }

    NVIC_SetPriority(LPUART1_IRQn, 0);
    NVIC_EnableIRQ(LPUART1_IRQn);
    LL_LPUART_EnableIT_RXNE(LPUART1);
    LL_LPUART_EnableIT_ERROR(LPUART1);
}

/* -------------------------------------------------------------------------- */

void bsp_uart_debug_init(void) {
    LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK2);

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);

    LL_GPIO_InitTypeDef gpio_init = {
        .Pin = USB_RXD_PIN | USB_TXD_PIN,
        .Mode = LL_GPIO_MODE_ALTERNATE,
        .Speed = LL_GPIO_SPEED_FREQ_LOW,
        .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
        .Pull = LL_GPIO_PULL_NO,
        .Alternate = LL_GPIO_AF_7,
    };
    gpio_init.Pin = USB_RXD_PIN;
    LL_GPIO_Init(USB_RXD_GPIO_PORT, &gpio_init);

    gpio_init.Pin = USB_TXD_PIN;
    LL_GPIO_Init(USB_TXD_GPIO_PORT, &gpio_init);

#if ENMABLE_USART1_DMA_TX == 1
    /* USART1_TX Init */
    LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_1, LL_DMAMUX_REQ_USART1_TX);
    LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_1, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MDATAALIGN_BYTE);
#endif

    LL_USART_InitTypeDef uart_init = {
        .PrescalerValue = LL_USART_PRESCALER_DIV1,
        .BaudRate = 115200,
        .DataWidth = LL_USART_DATAWIDTH_8B,
        .StopBits = LL_USART_STOPBITS_1,
        .Parity = LL_USART_PARITY_NONE,
        .TransferDirection = LL_USART_DIRECTION_TX_RX,
        .HardwareFlowControl = LL_USART_HWCONTROL_NONE,
        .OverSampling = LL_USART_OVERSAMPLING_16,
    };
    LL_USART_Init(USART1, &uart_init);
    LL_USART_SetTXFIFOThreshold(USART1, LL_USART_FIFOTHRESHOLD_1_8);
    LL_USART_SetRXFIFOThreshold(USART1, LL_USART_FIFOTHRESHOLD_1_8);
    LL_USART_DisableFIFO(USART1);
    LL_USART_ConfigAsyncMode(USART1);

    LL_USART_Enable(USART1);

    /* Polling USART1 initialisation */
    while ((!(LL_USART_IsActiveFlag_TEACK(USART1))) || (!(LL_USART_IsActiveFlag_REACK(USART1)))) {
        // Wait for ...
    }

    NVIC_SetPriority(USART1_IRQn, 0);
    NVIC_EnableIRQ(USART1_IRQn);
    LL_USART_EnableIT_RXNE(USART1);
    LL_USART_EnableIT_ERROR(USART1);
}

/* -------------------------------------------------------------------------- */

void bsp_uart_debug_write(uint8_t const *data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        LL_USART_TransmitData8(USART1, data[i]);
        while (LL_USART_IsActiveFlag_TXE(USART1) == 0) {
            // Wait for transfer reg is empty
        }
    }
    while (LL_USART_IsActiveFlag_TC(USART1) == 0) {
        // Wait for transfer reg is empty
    }
}

/* -------------------------------------------------------------------------- */

void bsp_uart_gnss_write(uint8_t const *data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        LL_LPUART_TransmitData8(LPUART1, data[i]);

        while (LL_LPUART_IsActiveFlag_TC(LPUART1) == 0) {
            // Wait for transfer complete
        }
    }
}

/* -------------------------------------------------------------------------- */

__weak void bsp_uart_gnss_byte_received(uint8_t byte) {
    (void)byte;
    // Copy this function in application to start receive data
}

/* -------------------------------------------------------------------------- */

__weak void bsp_uart_debug_byte_received(uint8_t byte) {
    (void)byte;
    // Copy this function in application to start receive data
}

/* -------------------------------------------------------------------------- */
