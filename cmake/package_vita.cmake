
if(!VITA_APP_NAME)
        message("NO VITA APP NAME")
        error("ERROR NO VITA APP NAME")
endif()

message("Configuring Packaging for PS Vita")
message("THEXTECH_EXECUTABLE_NAME = ${THEXTECH_EXECUTABLE_NAME}")
message("VITA_APP_NAME = ${VITA_APP_NAME}")
message("VITA_TITLEID = ${VITA_TITLEID}")
message("VITA_VERSION = ${VITA_VERSION}")
install(TARGETS thextech DESTINATION .)
vita_create_self(${THEXTECH_EXECUTABLE_NAME}.self output/bin/thextech UNSAFE)
vita_create_vpk(${THEXTECH_EXECUTABLE_NAME}.vpk ${VITA_TITLEID} ${THEXTECH_EXECUTABLE_NAME}.self
    VERSION ${VITA_VERSION}
    NAME ${VITA_APP_NAME}
    FILE    ${CMAKE_SOURCE_DIR}/resources/vita/sce_sys/icon0.png sce_sys/icon0.png
            ${CMAKE_SOURCE_DIR}/resources/vita/sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
            ${CMAKE_SOURCE_DIR}/resources/vita/sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
            ${CMAKE_SOURCE_DIR}/resources/vita/sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
            # ${CMAKE_SOURCE_DIR}/resources/vita/vert.cgv vert.cgv
            # ${CMAKE_SOURCE_DIR}/resources/vita/frag.cgf frag.cgf
)

add_custom_target(copy 
    COMMAND cp ${THEXTECH_EXECUTABLE_NAME}.self eboot.bin
    DEPENDS ${THEXTECH_EXECUTABLE_NAME}.self
)
