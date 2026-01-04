

#include <stdbool.h>

#include "stm32wlxx_hal.h"
#include "stm32wlxx_ll_dma.h"
#include "stm32wlxx_ll_gpio.h"

#include "bsp.h"
#include "bsp_adc.h"
#include "bsp_board.h"

/* -------------------------------------------------------------------------- */
#if CONFIG_LOKO_AIR_SEEED_E5 == 1
#    define DIV_RES_N1_KOMH 1U
#    define DIV_RES_N2_KOMH 1U
#else
#    define DIV_RES_N1_KOMH 30U
#    define DIV_RES_N2_KOMH 10U
#endif
#define ADC_RANGE (4096U)
const uint32_t DIV_MUL_KOEF = ((DIV_RES_N1_KOMH + DIV_RES_N2_KOMH) / DIV_RES_N2_KOMH);

/* -------------------------------------------------------------------------- */

enum {
    ADC_VAL_VBAT_INDEX,
    ADC_VAL_VREF_INDEX,

    ADC_VAL_COUNT
};

static uint32_t _values[ADC_VAL_COUNT];
static volatile bool _adc_done = false;

/* -------------------------------------------------------------------------- */

static ADC_HandleTypeDef _adc = {
    .Instance = ADC,
    .Init = {
        .ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2,
        .Resolution = ADC_RESOLUTION_12B,
        .DataAlign = ADC_DATAALIGN_RIGHT,
        .ScanConvMode = ADC_SCAN_ENABLE,
        .EOCSelection = ADC_EOC_SINGLE_CONV,
        .LowPowerAutoWait = DISABLE,
        .LowPowerAutoPowerOff = DISABLE,
        .ContinuousConvMode = DISABLE,
        .NbrOfConversion = 2,
        .DiscontinuousConvMode = DISABLE,
        .ExternalTrigConv = ADC_SOFTWARE_START,
        .ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE,
        .DMAContinuousRequests = DISABLE,
        .Overrun = ADC_OVR_DATA_PRESERVED,
        .SamplingTimeCommon1 = ADC_SAMPLETIME_79CYCLES_5,
        .SamplingTimeCommon2 = ADC_SAMPLETIME_79CYCLES_5,
        .OversamplingMode = DISABLE,
        .TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH,
    },
};
static DMA_HandleTypeDef _hdma_adc;

/* -------------------------------------------------------------------------- */

static void _adc_init(void) {

    HAL_StatusTypeDef ret = HAL_ADC_Init(&_adc);

    if (ret != HAL_OK) {
        BSP_FATAL(ret, NULL);
    }

    ADC_ChannelConfTypeDef adc_conf = {
        .Channel = ADC_CHANNEL_0,
        .Rank = ADC_REGULAR_RANK_1,
        .SamplingTime = ADC_SAMPLINGTIME_COMMON_1,
    };
    ret = HAL_ADC_ConfigChannel(&_adc, &adc_conf);

    if (ret != HAL_OK) {
        BSP_FATAL(ret, NULL);
    }

    adc_conf.Channel = ADC_CHANNEL_VREFINT;
    adc_conf.Rank = ADC_REGULAR_RANK_2;
    ret = HAL_ADC_ConfigChannel(&_adc, &adc_conf);

    if (ret != HAL_OK) {
        BSP_FATAL(ret, NULL);
    }
}

/* -------------------------------------------------------------------------- */

static void _adc_deinit(void) {
    HAL_ADC_DeInit(&_adc);
    __HAL_RCC_ADC_CLK_DISABLE();
    HAL_DMA_DeInit(_adc.DMA_Handle);
}

/* -------------------------------------------------------------------------- */

void HAL_ADC_MspInit(ADC_HandleTypeDef *adc_handle) {
    if (adc_handle->Instance == ADC) {
        __HAL_RCC_ADC_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        LL_GPIO_InitTypeDef gpio_param = {
            .Pin = VBAT_MEASURE_PIN,
            .Mode = LL_GPIO_MODE_ANALOG,
            .Pull = LL_GPIO_PULL_NO,
        };
        LL_GPIO_Init(VBAT_MEASURE_GPIO_PORT, &gpio_param);

        _hdma_adc.Instance = DMA1_Channel2;
        _hdma_adc.Init.Request = DMA_REQUEST_ADC;
        _hdma_adc.Init.Direction = DMA_PERIPH_TO_MEMORY;
        _hdma_adc.Init.PeriphInc = DMA_PINC_DISABLE;
        _hdma_adc.Init.MemInc = DMA_MINC_ENABLE;
        _hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
        _hdma_adc.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
        _hdma_adc.Init.Mode = DMA_NORMAL;
        _hdma_adc.Init.Priority = DMA_PRIORITY_LOW;
        HAL_StatusTypeDef ret = HAL_DMA_Init(&_hdma_adc);
        BSP_ASSERT(ret == HAL_OK);

        __HAL_LINKDMA(adc_handle, DMA_Handle, _hdma_adc);
    }
}

/* -------------------------------------------------------------------------- */

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *adc_handle) {
    (void)adc_handle;
    /* Report to main program that ADC sequencer has reached its end */
    _adc_done = true;
}

/* -------------------------------------------------------------------------- */

uint32_t bsp_adc_get_vbat_mv(void) {
    bsp_gpio_vbat_measure_on();

    _adc_init();

    /* Do couple measurement to skip rising time */
    static const size_t ADC_MEASUREMENT_COUNT = 2;
    for (size_t i = 0; i < ADC_MEASUREMENT_COUNT; i++) {
        _adc_done = false;
        HAL_StatusTypeDef ret = HAL_ADC_Start_DMA(&_adc, (uint32_t *)&_values, ADC_VAL_COUNT);

        if (ret != HAL_OK) {
            BSP_FATAL(ret, NULL);
        }

        uint32_t timeout_ticks = 10000;

        while (_adc_done == false) {
            timeout_ticks--;

            if (timeout_ticks == 0) {
                for (size_t i = 0; i < ADC_VAL_COUNT; i++) {
                    _values[i] = 1;
                }

                LOG_ERROR("No ADC interrupt");
                break;
            }
        }
    }

    bsp_gpio_vbat_measure_off();

    _adc_deinit();

    uint32_t vdda = VREFINT_CAL_VREF * (*VREFINT_CAL_ADDR) / _values[ADC_VAL_VREF_INDEX];

    uint32_t vbat = _values[ADC_VAL_VBAT_INDEX] * vdda / ADC_RANGE * DIV_MUL_KOEF;

    return (uint32_t)vbat;
}

/* -------------------------------------------------------------------------- */

void bsp_adc_dma_handler(void) {
    HAL_DMA_IRQHandler(&_hdma_adc);
}

/* -------------------------------------------------------------------------- */
