macro(process_kconfig TARGET_NAME KCONF_ROOT PROJ_CONFIG_FILE)
    message(STATUS "Kconfig: ${TARGET_NAME} ${PROJ_CONFIG_FILE}")

    if(EXISTS ${KCONF_ROOT}/Kconfig)
        set(PROJECT_KCONFIG_ROOT ${KCONF_ROOT}/Kconfig)
    else()
        message((error "Kconfig file not found in ${CMAKE_CURRENT_SOURCE_DIR}"))
    endif()

    set(AUTOCONF_H ${CMAKE_BINARY_DIR}/generated/include/autoconf_${TARGET_NAME}.h)

    get_filename_component(autoconf_dir ${AUTOCONF_H} DIRECTORY)
    file(MAKE_DIRECTORY ${autoconf_dir})

    set(DOT_CONF ${CMAKE_CURRENT_BINARY_DIR}/.config_${TARGET_NAME})

    execute_process(
        COMMAND cmake -E env
        srctree=${PROJECT_ROOT}
        KCONFIG_CONFIG=${DOT_CONF}
        python3 ${PROJECT_ROOT}/tools/kconfig/kconfig.py
        ${PROJECT_KCONFIG_ROOT}
        ${DOT_CONF}
        ${AUTOCONF_H}
        ${CMAKE_BINARY_DIR}/kconfig_files_${TARGET_NAME}.txt
        ${PROJ_CONFIG_FILE}
        RESULT_VARIABLE kconfig_result_${TARGET_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    if(NOT kconfig_result_${TARGET_NAME} EQUAL 0)
        message(FATAL_ERROR "Kconfig failed! ${kconfig_result_${TARGET_NAME}}")
    endif()

    file(STRINGS ${CMAKE_BINARY_DIR}/kconfig_files_${TARGET_NAME}.txt parsed_source_files)

    # Reconfigure project if any of Kconfig files have changed
    foreach(kconfig_file
            ${PROJ_CONFIG_FILE}
            ${DOT_CONF}
            ${AUTOCONF_H}
            ${parsed_source_files}
        )
        # message(STATUS "Kconfig depends on ${kconfig_file}")
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${kconfig_file})
    endforeach()

    # Import .config file to cmake space
    file(
        STRINGS ${DOT_CONF}
        config_option_list
        REGEX "^CONFIG_"
        ENCODING UTF-8
    )

    foreach(config_line ${config_option_list})
        # Match config parameter name
        string(REGEX MATCH "[^=]+" conf_name ${config_line})
        # Match parameter value
        string(REGEX MATCH "=(.+$)" conf_value ${config_line})
        set(conf_value ${CMAKE_MATCH_1})

        # Remove "" around value
        if(${conf_value} MATCHES "^\"(.*)\"$")
            set(conf_value ${CMAKE_MATCH_1})
        endif()

        #Create var in global scope, but we have several tartgets with different configs
        #so this way isn't good
        #set(${conf_name} ${conf_value})

        #Create var for target
        # message("conf: ${conf_name}=${conf_value}")
        if(${conf_value} STREQUAL "y")
            # message("The variable value is 'y'")
            set(config_enabled_vars_${TARGET_NAME} "${conf_name};${config_enabled_vars_${TARGET_NAME}}")
        endif()

    endforeach()

    #message("config_enabled_vars_${TARGET_NAME} = ${config_enabled_vars_${TARGET_NAME}}")

endmacro()
