# adds a custom target for git version (always built)
add_custom_target(git_version ALL)

# creates git_version.h using cmake script
add_custom_command(TARGET git_version
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -DOVERRIDE_GIT_BRANCH=${OVERRIDE_GIT_BRANCH}
        -DFLATPAK_BUILD=${FLATPAK_BUILD}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/git_version_update.cmake)

# attempt to force reconfigure for the build AFTER git status changes
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/generated-include/git_version.h")
