project(stm32wlxx_hal DESCRIPTION "STM32 HAL Lib")

add_library(${PROJECT_NAME} INTERFACE)

set(HAL_SRC_PATH "Src")
get_filename_component(HAL_SRC_PATH ${HAL_SRC_PATH} ABSOLUTE)
set(HAL_INC_PATH "Inc")
get_filename_component(HAL_INC_PATH ${HAL_INC_PATH} ABSOLUTE)
# set(STM32L4XX_HAL_CMSIS_PATH "../CMSIS")
# get_filename_component(STM32L4XX_HAL_CMSIS_PATH ${STM32L4XX_HAL_CMSIS_PATH} ABSOLUTE)

target_sources(${PROJECT_NAME}
    INTERFACE
        ${HAL_SRC_PATH}/stm32wlxx_hal_adc_ex.c
        ${HAL_SRC_PATH}/stm32wlxx_hal_adc.c
        ${HAL_SRC_PATH}/stm32wlxx_hal_cortex.c
        ${HAL_SRC_PATH}/stm32wlxx_hal_dma_ex.c
        ${HAL_SRC_PATH}/stm32wlxx_hal_dma.c
        ${HAL_SRC_PATH}/stm32wlxx_hal_exti.c
        ${HAL_SRC_PATH}/stm32wlxx_hal_flash_ex.c
        ${HAL_SRC_PATH}/stm32wlxx_hal_flash.c
        ${HAL_SRC_PATH}/stm32wlxx_hal_gpio.c
        ${HAL_SRC_PATH}/stm32wlxx_hal_pwr_ex.c
        ${HAL_SRC_PATH}/stm32wlxx_hal_pwr.c
        ${HAL_SRC_PATH}/stm32wlxx_hal_rcc_ex.c
        ${HAL_SRC_PATH}/stm32wlxx_hal_rcc.c
        ${HAL_SRC_PATH}/stm32wlxx_hal_rtc_ex.c
        ${HAL_SRC_PATH}/stm32wlxx_hal_rtc.c
        ${HAL_SRC_PATH}/stm32wlxx_hal_subghz.c
        ${HAL_SRC_PATH}/stm32wlxx_hal_tim_ex.c
        ${HAL_SRC_PATH}/stm32wlxx_hal_tim.c
        ${HAL_SRC_PATH}/stm32wlxx_hal.c
        ${HAL_SRC_PATH}/stm32wlxx_ll_adc.c
        ${HAL_SRC_PATH}/stm32wlxx_ll_dma.c
        ${HAL_SRC_PATH}/stm32wlxx_ll_exti.c
        ${HAL_SRC_PATH}/stm32wlxx_ll_gpio.c
        ${HAL_SRC_PATH}/stm32wlxx_ll_lpuart.c
        ${HAL_SRC_PATH}/stm32wlxx_ll_pwr.c
        ${HAL_SRC_PATH}/stm32wlxx_ll_rcc.c
        ${HAL_SRC_PATH}/stm32wlxx_ll_usart.c
        ${HAL_SRC_PATH}/stm32wlxx_ll_utils.c
)

target_compile_definitions(${PROJECT_NAME}
    INTERFACE
        -DUSE_HAL_DRIVER
        -DUSE_FULL_LL_DRIVER
)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        ${HAL_INC_PATH}
        ${HAL_INC_PATH}/Legacy
)
