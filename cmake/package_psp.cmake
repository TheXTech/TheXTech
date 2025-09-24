
set(PSP_APP_NAME "TheXTech ${THEXTECH_VERSION_STRING}")
set(PSP_TITLEID "THEXTECH0")

set(PSP_VERSION "${THEXTECH_VERSION_1}${THEXTECH_VERSION_2}.${THEXTECH_VERSION_3}${THEXTECH_VERSION_4}")

# the next two variables are used by template.xml.in
set(XTECH_VITA_AUTHORS "By Wohlstand and ds-sloth. Ported by Axiom.")

if(NOT "${THEXTECH_VERSION_REL}" STREQUAL "")
    set(XTECH_VITA_SHORT_DESC "git ${GIT_BRANCH} #${GIT_COMMIT_HASH}")
else()
    set(XTECH_VITA_SHORT_DESC "Based on SMBX 1.3 by Redigit")
endif()

message("Configuring Packaging for PSP")
message("THEXTECH_EXECUTABLE_NAME = ${THEXTECH_EXECUTABLE_NAME}")
message("PSP_APP_NAME = ${PSP_APP_NAME}")
message("PSP_TITLEID = ${PSP_TITLEID}")
message("PSP_VERSION = ${PSP_VERSION}")
message("PSP_AUTHORS = ${XTECH_PSP_AUTHORS}")
message("PSP_DESC = ${XTECH_PSP_SHORT_DESC}")
install(TARGETS thextech DESTINATION .)

create_pbp_file(
    TARGET thextech
    ICON_PATH "${CMAKE_SOURCE_DIR}/resources/psp/icon.png"
    BACKGROUND_PATH "${CMAKE_SOURCE_DIR}/resources/psp/background.png"
    MUSIC_PATH "${CMAKE_SOURCE_DIR}/resources/psp/game-beat.at3"
    PREVIEW_PATH NULL
    TITLE ${PSP_APP_NAME}
    VERSION ${PSP_VERSION}
    OUTPUT_DIR "${CMAKE_BINARY_DIR}/output/bin"
)
