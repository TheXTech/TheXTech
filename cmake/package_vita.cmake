
set(VITA_GLES_MODULES)
if(THEXTECH_BUILD_GL_ES_MODERN)
    set(VITA_GLES_MODULES
        "$ENV{VITASDK}/arm-vita-eabi/modules/libGLESv2.suprx module/libGLESv2.suprx"
        "$ENV{VITASDK}/arm-vita-eabi/modules/libIMGEGL.suprx module/libIMGEGL.suprx"
        "$ENV{VITASDK}/arm-vita-eabi/modules/libgpu_es4_ext.suprx module/libgpu_es4_ext.suprx"
    )
endif()

set(VITA_APP_NAME "TheXTech ${THEXTECH_VERSION_STRING}")
set(VITA_TITLEID "THEXTECH0")

set(VITA_VERSION "${THEXTECH_VERSION_1}${THEXTECH_VERSION_2}.${THEXTECH_VERSION_3}${THEXTECH_VERSION_4}")

# the next two variables are used by template.xml.in
set(XTECH_VITA_AUTHORS "By Wohlstand and ds-sloth. Ported by Axiom.")

if(NOT "${THEXTECH_VERSION_REL}" STREQUAL "")
    set(XTECH_VITA_SHORT_DESC "git ${GIT_BRANCH} #${GIT_COMMIT_HASH}")
else()
    set(XTECH_VITA_SHORT_DESC "Based on SMBX 1.3 by Redigit")
endif()

message("Configuring Packaging for PS Vita")
message("THEXTECH_EXECUTABLE_NAME = ${THEXTECH_EXECUTABLE_NAME}")
message("VITA_APP_NAME = ${VITA_APP_NAME}")
message("VITA_TITLEID = ${VITA_TITLEID}")
message("VITA_VERSION = ${VITA_VERSION}")
message("VITA_AUTHORS = ${XTECH_VITA_AUTHORS}")
message("VITA_DESC = ${XTECH_VITA_SHORT_DESC}")
install(TARGETS thextech DESTINATION .)

configure_file(${CMAKE_SOURCE_DIR}/resources/vita/sce_sys/livearea/contents/template.xml.in ${CMAKE_BINARY_DIR}/template.xml)

vita_create_self(${THEXTECH_EXECUTABLE_NAME}.self output/bin/thextech UNSAFE)
vita_create_vpk(${THEXTECH_EXECUTABLE_NAME}.vpk ${VITA_TITLEID} ${THEXTECH_EXECUTABLE_NAME}.self
    VERSION ${VITA_VERSION}
    NAME ${VITA_APP_NAME}
    FILE    ${CMAKE_SOURCE_DIR}/resources/vita/sce_sys/icon0.png sce_sys/icon0.png
            ${CMAKE_SOURCE_DIR}/resources/vita/sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
            ${CMAKE_SOURCE_DIR}/resources/vita/sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
            ${CMAKE_BINARY_DIR}/template.xml sce_sys/livearea/contents/template.xml
            # ${CMAKE_SOURCE_DIR}/resources/vita/vert.cgv vert.cgv
            # ${CMAKE_SOURCE_DIR}/resources/vita/frag.cgf frag.cgf
            ${VITA_GLES_MODULES}
)

add_custom_target(copy 
    COMMAND cp ${THEXTECH_EXECUTABLE_NAME}.self eboot.bin
    DEPENDS ${THEXTECH_EXECUTABLE_NAME}.self
)
