
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

macro(add_git_version_defs FILETARGET)
    set_property(
        SOURCE ${FILETARGET}
        APPEND PROPERTY COMPILE_DEFINITIONS GIT_VERSION="${THEXTECH_GIT_VERSION}"
    )
    set_property(
        SOURCE ${FILETARGET}
        APPEND PROPERTY COMPILE_DEFINITIONS GIT_BRANCH="${THEXTECH_GIT_BRANCH}"
    )
    message("-- Set GIT definitions to file ${FILETARGET}")
endmacro()
