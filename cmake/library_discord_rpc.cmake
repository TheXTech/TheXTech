add_library(PGE_DiscordRPC INTERFACE)

if(WIN32)
    if("${TARGET_PROCESSOR}" STREQUAL "x86" OR "${TARGET_PROCESSOR}" STREQUAL "x86_64")
        set(DISCORD_RPC_SUPPORTED ON)
    endif()
elseif(APPLE)
    if(NOT XTECH_MACOSX_TIGER AND ("${TARGET_PROCESSOR}" STREQUAL "x86_64" OR "${TARGET_PROCESSOR}" STREQUAL "arm64"))
        set(DISCORD_RPC_SUPPORTED ON)
    endif()
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    if("${TARGET_PROCESSOR}" STREQUAL "x86_64")
        set(DISCORD_RPC_SUPPORTED ON)
    endif()
endif()

if(DISCORD_RPC_SUPPORTED)
    option(THEXTECH_ENABLE_DISCORD_RPC "Enable Discord RPC support" OFF)
else()
    set(THEXTECH_ENABLE_DISCORD_RPC OFF CACHE BOOL "" FORCE)
    mark_as_advanced(THEXTECH_ENABLE_DISCORD_RPC)
endif()

if(DISCORD_RPC_SUPPORTED AND THEXTECH_ENABLE_DISCORD_RPC)
    set(libDiscordRpc_Libs "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}discord-rpc${PGE_LIBS_DEBUG_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
    set(THEXTECH_DISCORD_APPID nullptr CACHE STRING "Discord Application ID for the Risch Presence work")

    ExternalProject_Add(
        DiscordPRC_Local
        PREFIX ${CMAKE_BINARY_DIR}/external/DiscordRPC
        DOWNLOAD_COMMAND ""
        SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../3rdparty/thextech-discord-rpc
        CMAKE_ARGS
            "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
            "-DCMAKE_INSTALL_PREFIX=${DEPENDENCIES_INSTALL_DIR}"
            "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
            "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}"
            "-DCMAKE_POSITION_INDEPENDENT_CODE=${FREETYPE_PIC}"
            "-DCMAKE_DEBUG_POSTFIX=${PGE_LIBS_DEBUG_SUFFIX}"
            "-DBUILD_EXAMPLES=OFF"
            ${ANDROID_CMAKE_FLAGS}
            $<$<BOOL:APPLE>:-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}>
            $<$<BOOL:APPLE>:-DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}>
        BUILD_BYPRODUCTS
            "${libDiscordRpc_Libs}"
    )

    message(STATUS "Discord RPC will be built: ${libDiscordRpc_Libs}")

    target_link_libraries(PGE_DiscordRPC INTERFACE "${libDiscordRpc_Libs}")
    target_include_directories(PGE_DiscordRPC INTERFACE "${DEPENDENCIES_INSTALL_DIR}/include")
else()
    message(STATUS "Discord RPC will be disabled: unsupported platform")
endif()
