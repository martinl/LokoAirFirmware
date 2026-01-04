function(project_create_app TARGET LIBS)
    set(BSP_NAME bsp_${TARGET})

    message(STATUS "Created build target: " ${TARGET})

    add_executable(${TARGET}
        ${APP_SOURCES}
    )

    target_link_libraries(${TARGET}
        ${LAYOUT_NAME}
        ${BSP_NAME}
        ${LIBS}
    )

    target_link_options(${TARGET}
        PRIVATE
            -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.map
    )

    add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex ${EXECUTABLE_OUTPUT_PATH}/${TARGET} ${EXECUTABLE_OUTPUT_PATH}/${TARGET}.hex
        COMMAND ${CMAKE_OBJCOPY} -Obinary ${EXECUTABLE_OUTPUT_PATH}/${TARGET} ${EXECUTABLE_OUTPUT_PATH}/${TARGET}.bin
        COMMENT "Create ${TARGET}.hex and ${TARGET}.bin"
    )

endfunction()

function(project_create_bldr TARGET LIBS)
    set(BSP_NAME bsp_${TARGET})

    message(STATUS "Created build target: " ${TARGET})

    add_executable(${TARGET}
        ${BLDR_SOURCES}
    )

    target_link_libraries(${TARGET}
        ${LAYOUT_NAME}
        ${BSP_NAME}
        ${LIBS}
    )

    target_link_options(${TARGET}
        PRIVATE
            -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.map
    )

    add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex ${EXECUTABLE_OUTPUT_PATH}/${TARGET} ${EXECUTABLE_OUTPUT_PATH}/${TARGET}.hex
        COMMAND ${CMAKE_OBJCOPY} -Obinary ${EXECUTABLE_OUTPUT_PATH}/${TARGET} ${EXECUTABLE_OUTPUT_PATH}/${TARGET}.bin
        COMMENT "Create ${TARGET}.hex and ${TARGET}.bin"
    )

endfunction()

function(project_create_mcu_iface TARGET DEFINITIONS COMPILER_OPTIONS)

    add_library(${TARGET} INTERFACE)

    target_include_directories(${TARGET}
        INTERFACE
            ${CURRENT_SOURCE_DIR}
    )

    target_compile_definitions(${TARGET}
        INTERFACE
            ${DEFINITIONS}
    )

    target_compile_options(${TARGET}
        INTERFACE
            ${COMPILER_OPTIONS}
    )

    target_link_options(${TARGET}
        INTERFACE
            ${COMPILER_OPTIONS}
    )
    endfunction()

function(project_create_bsp_app_lib PROJECT_NAME TARGET_INTERFACE SOURCE LIBS)

    set(LINKER_FILE ${CMAKE_CURRENT_SOURCE_DIR}/app_stm32wle5jcix_flash.ld) #todo
    set(LINKER_TEMPLATE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/app_stm32wle5jcix_flash.ld)
    set(LINKER_H_FILE ${CMAKE_CURRENT_SOURCE_DIR}/bsp_flash_layout.h)

    set(TARGET bsp_${PROJECT_NAME}_app)

    set(AUTOCONF_H ${EXECUTABLE_OUTPUT_PATH}/generated/include/autoconf_${PROJECT_NAME}_app.h)
    # process_kconfig(${TARGET} ${PROJECT_ROOT} ${PROJECT_ROOT}/configs/${TARGET}.conf)

    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${LINKER_H_FILE})
    file(READ "${LINKER_H_FILE}" layout)
    utils_get_define_int_value("BSP_CONF_APP_STACK_SIZE" STACK_SIZE ${layout})
    message("${TARGET} stack size: ${STACK_SIZE}")

    add_library(${TARGET} STATIC)

    target_precompile_headers(${TARGET}
        PUBLIC
            ${AUTOCONF_H}
            ${PROJECT_SOURCES_ROOT}/Core/Inc/build_conf.h
    )

    target_sources(${TARGET}
        PRIVATE
            ${SOURCE}
    )

    target_compile_options(${TARGET}
        PUBLIC
            -Ofast
            -Wstack-usage=${STACK_SIZE}
    )

    target_link_options(${TARGET}
        PUBLIC
            -T${LINKER_FILE}
    )

    set_target_properties(${TARGET}
        PROPERTIES
            LINK_DEPENDS "${LINKER_FILE};${LINKER_TEMPLATE_FILE};${LINKER_H_FILE}"
    )

    target_link_libraries(${TARGET}
        PUBLIC
            ${TARGET_INTERFACE}
        PRIVATE
            ${LIBS} #Private to hide HAL for application level
    )

    # add_custom_command(TARGET ${TARGET}
    #     PRE_BUILD
    #     COMMAND arm-none-eabi-cpp -P ${LINKER_TEMPLATE_FILE} -o ${LINKER_FILE}
    #     DEPENDS "${LINKER_FILE};${LINKER_TEMPLATE_FILE};${LINKER_H_FILE};"
    #     COMMENT "Re-Generate linker file: ${LINKER_FILE} from:\r\n${LINKER_H_FILE}"
    # )

endfunction()

function(project_create_bsp_bldr_lib PROJECT_NAME TARGET_INTERFACE SOURCE LIBS)

    set(LINKER_FILE ${CMAKE_CURRENT_SOURCE_DIR}/bldr_stm32wle5jcix_flash.ld)
    set(LINKER_TEMPLATE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/bldr_stm32wle5jcix_flash.ld)
    set(LINKER_H_FILE ${CMAKE_CURRENT_SOURCE_DIR}/bsp_flash_layout.h)

    set(TARGET bsp_${PROJECT_NAME}_bldr)

    string(TOUPPER BUILD_${PROJECT_NAME}_CONF BUILD_XXX_CONF)

    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${LINKER_H_FILE})
    file(READ "${LINKER_H_FILE}" layout)
    utils_get_define_int_value("BSP_CONF_BLDR_STACK_SIZE" STACK_SIZE ${layout})
    message("${TARGET} stack size: ${STACK_SIZE}")

    add_library(${TARGET} STATIC)

    set(AUTOCONF_H ${EXECUTABLE_OUTPUT_PATH}/generated/include/autoconf_${PROJECT_NAME}_bldr.h)
    target_precompile_headers(${TARGET}
        PUBLIC
            ${AUTOCONF_H}
            ${PROJECT_SOURCES_ROOT}/Core/Inc/build_conf.h
    )

    target_sources(${TARGET}
        PRIVATE
            ${SOURCE}
    )

    target_compile_options(${TARGET}
        PUBLIC
            -Os
            -Wstack-usage=${STACK_SIZE}
    )

    target_link_options(${TARGET}
        PUBLIC
            -Xlinker --defsym=CONFIG_BUILD_TYPE_BOOTLOADER=1
            -T${LINKER_FILE}
    )

    set_target_properties(${TARGET}
        PROPERTIES
            LINK_DEPENDS "${LINKER_FILE};${LINKER_TEMPLATE_FILE};${LINKER_H_FILE};"
    )

    target_link_libraries(${TARGET}
        PUBLIC
            ${TARGET_INTERFACE}
        PRIVATE
            ${LIBS} #Private to hide HAL for application level
    )

    # add_custom_command(TARGET ${TARGET}
    #     PRE_BUILD
    #     COMMAND arm-none-eabi-cpp -P ${LINKER_TEMPLATE_FILE} -o ${LINKER_FILE}
    #     DEPENDS "${LINKER_FILE};${LINKER_TEMPLATE_FILE};${LINKER_H_FILE};"
    #     COMMENT "Re-Generate linker file: ${LINKER_FILE} from:\r\n${LINKER_H_FILE}"
    # )

endfunction()
