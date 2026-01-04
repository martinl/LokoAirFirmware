function(load_git_info)
    # Get the current branch name
    execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        OUTPUT_VARIABLE GIT_BRANCH_NAME
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(GIT_BRANCH_NAME "${GIT_BRANCH_NAME}" PARENT_SCOPE)
    add_compile_definitions(GIT_BRANCH_NAME="${GIT_BRANCH_NAME}")

    # Get the current commit hash
    execute_process(
        COMMAND git rev-parse HEAD
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(GIT_COMMIT_HASH "${GIT_COMMIT_HASH}" PARENT_SCOPE)
    add_compile_definitions(GIT_COMMIT_HASH="${GIT_COMMIT_HASH}")

    # Get the date of the current commit
    execute_process(
        COMMAND git show -s --format=%ci HEAD
        OUTPUT_VARIABLE GIT_COMMIT_DATE
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(GIT_COMMIT_DATE "${GIT_COMMIT_DATE}" PARENT_SCOPE)
    add_compile_definitions(GIT_COMMIT_DATE="${GIT_COMMIT_DATE}")

    # Get the author of the current commit
    execute_process(
        COMMAND git show -s --format=%an HEAD
        OUTPUT_VARIABLE GIT_COMMIT_AUTHOR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(GIT_COMMIT_AUTHOR "${GIT_COMMIT_AUTHOR}" PARENT_SCOPE)
    add_compile_definitions(GIT_COMMIT_AUTHOR="${GIT_COMMIT_AUTHOR}")

    # Get the commit message of the current commit
    execute_process(
        COMMAND git show -s --format=%s HEAD
        OUTPUT_VARIABLE GIT_COMMIT_MESSAGE
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(GIT_COMMIT_MESSAGE "${GIT_COMMIT_MESSAGE}" PARENT_SCOPE)
    add_compile_definitions(GIT_COMMIT_MESSAGE="${GIT_COMMIT_MESSAGE}")

    # Get the last tag in x.y.z format, if not find set to 0.0.0
    execute_process(
        COMMAND git describe --tags  --abbrev=0  --match [0-9]*.*
        OUTPUT_VARIABLE GIT_LAST_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if ("${GIT_LAST_TAG}" STREQUAL "")
        message(WARNING "Git version not found, set to 0.0.0")
        set(GIT_LAST_TAG "0.0.0")
    endif()
    set(GIT_LAST_TAG "${GIT_LAST_TAG}" PARENT_SCOPE)
    add_compile_definitions(GIT_LAST_TAG="${GIT_LAST_TAG}")

endfunction()


function(print_git_info)
    message(STATUS "GIT_BRANCH_NAME = ${GIT_BRANCH_NAME}")
    message(STATUS "GIT_COMMIT_HASH = ${GIT_COMMIT_HASH}")
    message(STATUS "GIT_COMMIT_DATE = ${GIT_COMMIT_DATE}")
    message(STATUS "GIT_COMMIT_AUTHOR = ${GIT_COMMIT_AUTHOR}")
    message(STATUS "GIT_COMMIT_MESSAGE = ${GIT_COMMIT_MESSAGE}")
    message(STATUS "GIT_LAST_TAG = ${GIT_LAST_TAG}")
endfunction()
