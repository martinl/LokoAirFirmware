#Folloing code works only for global scope, not for target scope
# macro(utils_add_subdir_ifdef varname subdir)
#     if(DEFINED ${varname})
#         add_subdirectory(${subdir})
#     endif()
# endmacro()

# macro(utils_add_var_ifdef varname var value)
#     if(DEFINED ${varname})
#         set(${var} "${value};${${var}}" )
#     endif()
# endmacro()


macro(utils_target_add_lib_ifdef varname target_name value)

    set(TARGET_GLOBAL_DEFINES_VAR config_enabled_vars_${target_name})
    set(TARGET_GLOBAL_DEFINES_VAR ${${TARGET_GLOBAL_DEFINES_VAR}})

    foreach(define_name ${TARGET_GLOBAL_DEFINES_VAR})
        if("${define_name}" STREQUAL "${varname}")
            target_link_libraries(${target_name} ${value})
        endif()
    endforeach()
endmacro()

macro(utils_bsp_add_lib_ifdef varname target_name value)

    set(TARGET_GLOBAL_DEFINES_VAR config_enabled_vars_${target_name})
    set(TARGET_GLOBAL_DEFINES_VAR ${${TARGET_GLOBAL_DEFINES_VAR}})

    foreach(define_name ${TARGET_GLOBAL_DEFINES_VAR})
        if("${define_name}" STREQUAL "${varname}")
            target_link_libraries(bsp_${target_name} PRIVATE ${value})
        endif()
    endforeach()
endmacro()

macro(utils_get_define_int_value DEFINE VALUE HEADER_FILE_CONTEXT)
    # "(\n|^)# *define ..." it doesnt works without () so need move match to CMAKE_MATCH_2
    string(REGEX MATCH "(\n|^)# *define +${DEFINE} +([0-9 \*+\-\/\(\)x]*)" _ ${HEADER_FILE_CONTEXT})
    # message("${TARGET} ${DEFINE}= ${CMAKE_MATCH_2}")
    string(LENGTH "${CMAKE_MATCH_2}" str_length)
    if(str_length EQUAL 0)
        message(WARNING "Can't find ${DEFINE} value")
    else()
        math(EXPR ${VALUE} ${CMAKE_MATCH_2})
    endif()
endmacro()

function(utils_print_lib_list TARGET)
    get_target_property(LIBRARIES_LIST ${TARGET} LINK_LIBRARIES)
    message(STATUS "Libs: ${TARGET}: ${LIBRARIES_LIST}")
endfunction()
