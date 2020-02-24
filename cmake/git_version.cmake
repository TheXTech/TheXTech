
if(NOT GIT_COMMIT_HASH)
    add_definitions(-DGIT_VERSION="CMakeUnknown")
else()
    add_definitions(-DGIT_VERSION="${GIT_COMMIT_HASH}")
endif()

if(NOT GIT_BRANCH)
    add_definitions(-DGIT_BRANCH="unknown")
else()
    add_definitions(-DGIT_BRANCH="${GIT_BRANCH}")
endif()

