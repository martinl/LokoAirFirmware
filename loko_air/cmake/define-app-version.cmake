function(define_version app_version_string)
    # Use a regular expression to parse the version string
    string(REGEX MATCH "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" _match ${app_version_string})

    # Check if the match was successful
    if (_match)
        # Extract the major, minor, and patch numbers
        string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\1" APP_VERSION_MAJOR ${app_version_string})
        string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\2" APP_VERSION_MINOR ${app_version_string})
        string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\3" APP_VERSION_PATCH ${app_version_string})

        # Set the version variables in the parent scope
        set(APP_VERSION_MAJOR "${APP_VERSION_MAJOR}" PARENT_SCOPE)
        set(APP_VERSION_MINOR "${APP_VERSION_MINOR}" PARENT_SCOPE)
        set(APP_VERSION_PATCH "${APP_VERSION_PATCH}" PARENT_SCOPE)

        # Define global definitions for the version numbers
        add_compile_definitions(
            APP_VERSION_MAJOR=${APP_VERSION_MAJOR}
            APP_VERSION_MINOR=${APP_VERSION_MINOR}
            APP_VERSION_PATCH=${APP_VERSION_PATCH}
        )
    else()
        message(FATAL_ERROR "Invalid version string format [${app_version_string}]. Expected format: X.Y.Z")
    endif()
endfunction()
