project(test_runner)

file(GLOB TEST_SOURCES
     "src/*_test.cpp"
)

#Debug purposes to debug only one test
# set(TEST_SOURCES "src/dummy_test.cpp")

add_executable(${PROJECT_NAME}
    ${TEST_SOURCES}
    test_runner.cpp
    common_fake.c
    spy/settings_io.cpp
    ../Middlewares/Third_Party/LoRaWAN/Crypto/lorawan_aes.c
)

target_link_libraries(${PROJECT_NAME}
    bsp_cpputest
    cayenne_lpp_c
    cmd_line
    CppUTest
    CppUTestExt
    crc16
    encrypt_p2p_payload
    gnss_trace
    log_
    lwgps
    queue
    settings
    stm32_bootloader_host_protocol
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        .
        ./../Middlewares/Third_Party/LoRaWAN
        ./../Middlewares/Third_Party/LoRaWAN/Mac
        ./../Middlewares/Third_Party/LoRaWAN/Mac/Region/
        ./../Middlewares/Third_Party/LoRaWAN/Utilities/
)

add_custom_target(generate-coverage DEPENDS ${PROJECT_NAME}
    COMMAND ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}
    COMMAND rm -rf coverage_report
    COMMAND mkdir coverage_report
    COMMAND lcov --rc lcov_branch_coverage=1 --capture --directory . -o ./coverage_report/tests.info
    COMMAND lcov --rc lcov_branch_coverage=1 --remove ./coverage_report/tests.info '${PROJECT_SOURCES_ROOT}/Core/Inc/*' '${PROJECT_SOURCES_ROOT}/tests/*' '${PROJECT_SOURCES_ROOT}/Core/log_/*' '${PROJECT_SOURCES_ROOT}/Middlewares/Third_Party/*' '/opt/cpputest/*' '/usr/include/*' -o ./coverage_report/tests.info
    # COMMAND lcov --rc lcov_branch_coverage=1 --remove ./coverage_report/tests.info '${PROJECT_SOURCES_ROOT}/Core/Inc/*' '${PROJECT_SOURCES_ROOT}/tests/*' '${PROJECT_SOURCES_ROOT}/Core/log_/*' '/opt/cpputest/*' '/usr/include/*' -o ./coverage_report/tests.info
    COMMAND genhtml ./coverage_report/tests.info --output-directory ./coverage_report/ --branch-coverage
)

message("CMAKE_C_COMPILER " ${CMAKE_C_COMPILER})
message("CMAKE_CXX_COMPILER " ${CMAKE_CXX_COMPILER})
message("CMAKE_ASM_COMPILER " ${CMAKE_ASM_COMPILER})
