# Get the current working branch
set(OVERRIDE_GIT_BRANCH "" CACHE STRING "Override name of GIT branch")

if(OVERRIDE_GIT_BRANCH)
    set(GIT_BRANCH ${OVERRIDE_GIT_BRANCH})
elseif(DEFINED ENV{APPVEYOR_REPO_BRANCH})
    set(GIT_BRANCH $ENV{APPVEYOR_REPO_BRANCH})
elseif(DEFINED ENV{TRAVIS_BRANCH})
    set(GIT_BRANCH $ENV{TRAVIS_BRANCH})
elseif(DEFINED ENV{BRANCH_NAME})
    set(GIT_BRANCH $ENV{BRANCH_NAME})
else()
    execute_process(
            COMMAND git rev-parse --abbrev-ref HEAD
            WORKING_DIRECTORY ${SOURCE_DIR}
            OUTPUT_VARIABLE GIT_BRANCH
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

# Get the latest abbreviated commit hash of the working branch
execute_process(
        COMMAND git log -1 --format=%h
        WORKING_DIRECTORY ${SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Check if the git index is dirty
execute_process(
        COMMAND git diff-index HEAD --
        WORKING_DIRECTORY ${SOURCE_DIR}
        OUTPUT_VARIABLE GIT_DIRTY_STRING
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

# flatpak-builder breaks any commands that check the git tree
if(NOT "${GIT_DIRTY_STRING}" STREQUAL "" AND NOT FLATPAK_BUILD)
    # on CI, display the git diff in the log
    if(OVERRIDE_GIT_BRANCH OR DEFINED ENV{CI})
        message("=== git index dirty, see diff:")
        message("${GIT_DIRTY_STRING}")
    endif()

    set(GIT_COMMIT_HASH "${GIT_COMMIT_HASH}-dirty")
endif()

if(NOT GIT_COMMIT_HASH)
    set(THEXTECH_GIT_VERSION "CMakeUnknown")
else()
    set(THEXTECH_GIT_VERSION "${GIT_COMMIT_HASH}")
endif()

if(NOT GIT_BRANCH)
    set(THEXTECH_GIT_BRANCH "unknown")
else()
    set(THEXTECH_GIT_BRANCH "${GIT_BRANCH}")
endif()

message("=== git info: commit ${THEXTECH_GIT_VERSION}, branch ${THEXTECH_GIT_BRANCH}")

file(MAKE_DIRECTORY generated-include)
file(WRITE generated-include/git_version.h.txt "#define GIT_VERSION \"${THEXTECH_GIT_VERSION}\"\n#define GIT_BRANCH \"${THEXTECH_GIT_BRANCH}\"\n")

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different generated-include/git_version.h.txt generated-include/git_version.h)
