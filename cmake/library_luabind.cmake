
# Note: You must also include "library_luajit.cmake" too!

add_library(PGE_LuaBind INTERFACE)

if(PGE_USE_LUAJIT)
    set(LUAJIT_USE_CMAKE_FLAG -DUSE_LUAJIT=ON)
    message("***** LuaJIT Lua in use! *****")
else()
    set(LUAJIT_USE_CMAKE_FLAG -DUSE_LUAJIT=OFF)
    message("***** PUC-Rio Lua in use! *****")
endif()

set(libLuaBind_Lib "${DEPENDENCIES_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}luabind${PGE_LIBS_DEBUG_SUFFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")

if(NOT LUABIND_SOURCE_DIR)
    set(LUABIND_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../3rdparty/luabind")
endif()

# LuaBind is a powerful lua binding library for C++
ExternalProject_Add(
    LuaBind_Local
    PREFIX ${CMAKE_BINARY_DIR}/external/luabind
    DOWNLOAD_COMMAND ""
    SOURCE_DIR "${LUABIND_SOURCE_DIR}"
    CMAKE_ARGS
        "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
        "-DCMAKE_INSTALL_PREFIX=${DEPENDENCIES_INSTALL_DIR}"
        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        "-DCMAKE_CONFIGURATION_TYPES=${CMAKE_CONFIGURATION_TYPES}"
        "-DCMAKE_DEBUG_POSTFIX=d"
        "-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
        ${ANDROID_CMAKE_FLAGS}
        ${LUAJIT_USE_CMAKE_FLAG}
        $<$<BOOL:APPLE>:-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}>
    BUILD_BYPRODUCTS
        "${libLuaBind_Lib}"
)

if(PGE_USE_LUAJIT)
    add_dependencies(LuaBind_Local LuaJIT_local)
endif()

target_link_libraries(PGE_LuaBind INTERFACE "${libLuaBind_Lib}" PGE_LuaJIT)
