project(LokoAir)

cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_VERBOSE_MAKEFILE ON)

set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(PROJECT_SOURCES_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/out")
set(CMAKE_BINARY_DIR ${OUT_DIR})
set(CMAKE_CURRENT_BINARY_DIR ${OUT_DIR})
set(EXECUTABLE_OUTPUT_PATH ${OUT_DIR})
set(ARCHIVE_OUTPUT_DIRECTORY ${OUT_DIR})
set(RUNTIME_OUTPUT_DIRECTORY ${OUT_DIR})
set(LIBRARY_OUTPUT_PATH ${OUT_DIR})

#-Wundef
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall -Werror ")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Werror ")

include(cmake/utils.cmake)
include(cmake/kconfig.cmake)
include(cmake/create_target.cmake)
include(cmake/git-info.cmake)
include(cmake/define-app-version.cmake)

load_git_info()
print_git_info()
define_version(${GIT_LAST_TAG})

# Retrieve the current Git branch name
execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    OUTPUT_VARIABLE GIT_BRANCH_NAME
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_definitions(-DGIT_BRANCH_NAME="${GIT_BRANCH_NAME}")

if(DEBUG_BUILD)
    message(STATUS "DEBUG_BUILD=1")
    add_definitions(-DDEBUG_BUILD=1)
else()
    message(STATUS "DEBUG_BUILD=0")
    add_definitions(-DDEBUG_BUILD=0)
endif()


message(STATUS "CMAKE_TOOLCHAIN_FILE:${CMAKE_TOOLCHAIN_FILE}")

include_directories(
    ${PROJECT_SOURCES_ROOT}/Core
    ${PROJECT_SOURCES_ROOT}/Core/Inc
    ${PROJECT_SOURCES_ROOT}/Core/Src
    ${PROJECT_SOURCES_ROOT}/Core/Src/lorawan_app
    ${PROJECT_SOURCES_ROOT}/Drivers/CMSIS/Device/ST/STM32WLxx/Include
    ${PROJECT_SOURCES_ROOT}/Drivers/CMSIS/Include
    ${PROJECT_SOURCES_ROOT}/Drivers/STM32WLxx_HAL_Driver/Inc
    ${PROJECT_SOURCES_ROOT}/Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver
)

enable_language(ASM)

add_subdirectory(Core)
add_subdirectory(Middlewares)
add_subdirectory(Drivers)

set(APP_COMMON_LIB_LIST
    cayenne_lpp_c
    cmd_line
    encrypt_p2p_payload
    gnss_trace
    log_
    LoRaWAN
    lwgps
    queue
    settings
    SubGHz_Phy
)

set(BLDR_COMMON_LIB_LIST
    queue
    crc16
    stm32_bootloader_host_protocol
)


macro(target_add_lib_ifdef TARGET_NAME)

    # utils_target_add_lib_ifdef(CONFIG_DEBUG_PRINT_CALL_STACK ${TARGET_NAME} cm_backtrace)

endmacro()

macro(bsp_add_lib_ifdef TARGET_NAME)

    # utils_bsp_add_lib_ifdef(CONFIG_BSP_DRIVER_SEGGER_RTT ${TARGET_NAME} bsp_segger_rtt_driver)

endmacro()

file(GLOB_RECURSE  LORA_APP_SOURCES
     "${PROJECT_SOURCES_ROOT}/Core/Src/lorawan_app/*.c"
)

set(APP_SOURCES
    "${LORA_APP_SOURCES}"
    "${PROJECT_SOURCES_ROOT}/Core/Src/application.c"
    "${PROJECT_SOURCES_ROOT}/Core/Src/subghz_phy_app.c"
    "${PROJECT_SOURCES_ROOT}/Core/Src/sysmem.c"
    "${PROJECT_SOURCES_ROOT}/Core/Src/version.c"
)

set(BLDR_SOURCES
    "${PROJECT_SOURCES_ROOT}/Core/Src/bootloader.c"
    "${PROJECT_SOURCES_ROOT}/Core/Src/version.c"
)

if(BUILD_CONFIG_UNIT_TESTS)
    process_kconfig(test_runner ${PROJECT_ROOT} ${PROJECT_ROOT}/configs/cpputest.conf)
    add_subdirectory(tests)
elseif(BUILD_CONFIG_WIN_SIMULATOR)
    add_subdirectory(Simulator)
else()
    # Get all *.conf and create target from it
    file(GLOB CONFIGS_LIST "${PROJECT_ROOT}/configs/*.conf")
    foreach(CONF_FILE_PATH ${CONFIGS_LIST})
        # Remove path, leave file name
        get_filename_component(CONF_FILE_NAME ${CONF_FILE_PATH} NAME)
        # Remove the extension from the filename
        string(REGEX REPLACE "\\..*$" "" TARGET_NAME ${CONF_FILE_NAME})

        process_kconfig(${TARGET_NAME} ${PROJECT_ROOT} ${CONF_FILE_PATH})

        if(CONF_FILE_NAME MATCHES "_bldr.conf$")
            bsp_add_lib_ifdef(${TARGET_NAME})
            project_create_bldr(${TARGET_NAME} "${BLDR_COMMON_LIB_LIST}")
            target_add_lib_ifdef(${TARGET_NAME})
            utils_print_lib_list(${TARGET_NAME})
        endif()

        if(CONF_FILE_NAME MATCHES "_app.conf$")
            bsp_add_lib_ifdef(${TARGET_NAME})
            project_create_app(${TARGET_NAME} "${APP_COMMON_LIB_LIST}")
            target_add_lib_ifdef(${TARGET_NAME})
            utils_print_lib_list(${TARGET_NAME})
        endif()
    endforeach()
endif()
