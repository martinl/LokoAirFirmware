project(bsp_cpputest)

add_library(${PROJECT_NAME} STATIC)

if(DEFINED ENV{CPPUTEST_HOME})
    message(STATUS "CPPUTEST_HOME:" $ENV{CPPUTEST_HOME})
else()
    message(FATAL_ERROR "CPPUTEST_HOME not defined" )
endif()

set(CPPUTEST_HOME $ENV{CPPUTEST_HOME})

set(COMPILE_OPTIONS_C
    -m32
    -g
    -pedantic-errors
    -Wall
    -Wconversion
    -Werror
    -Wextra
    -Wno-long-long
    -Wno-pedantic
    -Wshadow
    -Wsign-conversion
    -Wswitch-default
    -Wswitch-enum
    -fprofile-arcs  #generate coverage informarion
    -ftest-coverage  #generate coverage informarion
    -O0
)

set(COMPILE_OPTIONS_CXX
    ${COMPILE_OPTIONS_C}
    -Woverloaded-virtual
)

target_compile_options(${PROJECT_NAME}
    PUBLIC
        $<$<COMPILE_LANGUAGE:C>:${COMPILE_OPTIONS_C}>
        $<$<COMPILE_LANGUAGE:CXX>:${COMPILE_OPTIONS_CXX}>
)

target_link_options(${PROJECT_NAME}
    PUBLIC
        ${COMPILE_OPTIONS_CXX}
)

target_link_directories( ${PROJECT_NAME}
    PUBLIC
        ${CPPUTEST_HOME}/lib
)

target_sources(${PROJECT_NAME}
    PRIVATE
        bsp_fake.c
        bsp_flash_fake.c
        bsp_uart_fake.c
)

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        -DBUILD_CPPUTEST_CONF=1
)

target_precompile_headers(${PROJECT_NAME}
    PUBLIC
        ${PROJECT_ROOT}/build/out/generated/include/autoconf_test_runner.h
        ${PROJECT_SOURCES_ROOT}/Core/Inc/build_conf.h
        ${CPPUTEST_HOME}/include/CppUTest/MemoryLeakDetectorMallocMacros.h
        ${CPPUTEST_HOME}/include/CppUTest/MemoryLeakDetectorNewMacros.h
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        .
        ${CPPUTEST_HOME}/include
)
