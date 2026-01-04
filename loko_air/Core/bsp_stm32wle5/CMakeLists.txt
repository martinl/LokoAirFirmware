set(CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
get_filename_component(CURRENT_SOURCE_DIR ${CURRENT_SOURCE_DIR} ABSOLUTE)

set(BSP_COMPILE_OPTIONS_C
    -g
    -mthumb
    -mabi=aapcs
    -mcpu=cortex-m4
    -lm
    -mfloat-abi=soft
    -Wall
    -ffunction-sections
    -fdata-sections
    -Wl,--print-memory-usage
    -Wl,--gc-sections
    -specs=nano.specs
    -specs=nosys.specs
    -u _printf_float
)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(BSP_COMPILE_OPTIONS_C
        ${BSP_COMPILE_OPTIONS_C}
        -finline-small-functions
        -O0
    )
else()
    set(BSP_COMPILE_OPTIONS_C
        ${BSP_COMPILE_OPTIONS_C}
        -flto
        -ffat-lto-objects
        -fmerge-all-constants
        -finline-small-functions
        -findirect-inlining
        -ffunction-sections
        -fdata-sections
        -Os
    )
endif()

file(GLOB BSP_APP_SOURCES
    "*.c"
    "SubGHz_Phy/*.c"
)

set(BSP_APP_SOURCES
    ${BSP_APP_SOURCES}
    ${CURRENT_SOURCE_DIR}/startup_stm32wle5jcix.s
)

set(BSP_BLDR_SOURCES
    ${CURRENT_SOURCE_DIR}/bsp_battery.c
    ${CURRENT_SOURCE_DIR}/bsp_clocks.c
    ${CURRENT_SOURCE_DIR}/bsp_flash.c
    ${CURRENT_SOURCE_DIR}/bsp_gpio.c
    ${CURRENT_SOURCE_DIR}/bsp_low_power.c
    ${CURRENT_SOURCE_DIR}/bsp_rtc.c
    ${CURRENT_SOURCE_DIR}/bsp_stm32wlxx_it.c
    ${CURRENT_SOURCE_DIR}/bsp_uart.c
    ${CURRENT_SOURCE_DIR}/bsp.c
    ${CURRENT_SOURCE_DIR}/startup_stm32wle5jcix.s
    ${CURRENT_SOURCE_DIR}/system_stm32wlxx.c
)


set (BSP_DEFINE_LIST
    -DSTM32WLE5xx=1
    -DCORE_CM4=1
)

project_create_mcu_iface(
    mcu_stm32wle5xx_interface
    "${BSP_DEFINE_LIST}"
    "${BSP_COMPILE_OPTIONS_C}"
)

set (BSP_LIB_LIST
    stm32wlxx_hal
)

project_create_bsp_app_lib(loko
    mcu_stm32wle5xx_interface
    "${BSP_APP_SOURCES}"
    "${BSP_LIB_LIST}"
)
target_include_directories(bsp_loko_app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/SubGHz_Phy
)


project_create_bsp_app_lib(loko_e5
    mcu_stm32wle5xx_interface
    "${BSP_APP_SOURCES}"
    "${BSP_LIB_LIST}"
)
target_include_directories(bsp_loko_e5_app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/SubGHz_Phy
)

project_create_bsp_bldr_lib(loko
    mcu_stm32wle5xx_interface
    "${BSP_BLDR_SOURCES}"
    "${BSP_LIB_LIST}"
)

project_create_bsp_bldr_lib(loko_e5
    mcu_stm32wle5xx_interface
    "${BSP_BLDR_SOURCES}"
    "${BSP_LIB_LIST}"
)
