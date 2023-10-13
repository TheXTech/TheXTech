# Get the current working branch
set(OVERRIDE_GIT_BRANCH "" CACHE STRING "Override name of GIT branch")

# Identify the Pull-Request
if((DEFINED ENV{TRAVIS_PULL_REQUEST} AND NOT "$ENV{TRAVIS_PULL_REQUEST}" STREQUAL "false")
    OR (DEFINED ENV{APPVEYOR_PULL_REQUEST_NUMBER})
    OR (DEFINED ENV{PULL_REQUEST_NUMBER}))
    set(CI_PULL_REQUEST "-pr-check")
else()
    set(CI_PULL_REQUEST "")
endif()

if(OVERRIDE_GIT_BRANCH)
    set(GIT_BRANCH ${OVERRIDE_GIT_BRANCH})
elseif(DEFINED ENV{APPVEYOR_REPO_BRANCH})
    set(GIT_BRANCH $ENV{APPVEYOR_REPO_BRANCH})
elseif(DEFINED ENV{TRAVIS_BRANCH})
    set(GIT_BRANCH $ENV{TRAVIS_BRANCH})
else()
    execute_process(
            COMMAND git rev-parse --abbrev-ref HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_BRANCH
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

# Get the latest abbreviated commit hash of the working branch
execute_process(
        COMMAND git log -1 --format=%h
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Check if the git index is dirty
execute_process(
        COMMAND git diff-index HEAD --
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_DIRTY_STRING
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT "${GIT_DIRTY_STRING}" STREQUAL "")
    set(GIT_COMMIT_HASH "${GIT_COMMIT_HASH}-dirty")
endif()

set(PACKAGE_SUFFIX ${GIT_BRANCH}${CI_PULL_REQUEST})
