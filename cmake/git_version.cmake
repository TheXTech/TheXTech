# adds a custom target for git version (always built)
add_custom_target(git_version ALL)

# creates git_version.h using cmake script
add_custom_command(TARGET git_version
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -DOVERRIDE_GIT_BRANCH=${OVERRIDE_GIT_BRANCH}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/git_version_update.cmake)
