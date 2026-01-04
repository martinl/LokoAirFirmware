project(SubGHz_Phy)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        .
)

target_sources(${PROJECT_NAME}
    INTERFACE
        stm32_radio_driver/radio.c
        stm32_radio_driver/radio_driver.c
        stm32_radio_driver/radio_fw.c
)
