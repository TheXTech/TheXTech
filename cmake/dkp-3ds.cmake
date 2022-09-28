include(${CMAKE_CURRENT_LIST_DIR}/devkitPro.cmake)

if(DEFINED ENV{CTRULIB})
	set(CTRULIB $ENV{CTRULIB})
else()
	set(CTRULIB ${DEVKITPRO}/libctru)
endif()

set(PORTLIBS ${DEVKITPRO}/portlibs/3ds)

set(cross_prefix arm-none-eabi-)
set(arch_flags "-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft -ffunction-sections")
set(inc_flags "-isystem${CTRULIB}/include -isystem${PORTLIBS}/include ${arch_flags} -mword-relocations")
set(link_flags "-L${PORTLIBS}/lib -L${CTRULIB}/lib -lctru -lcitro2d -lcitro3d -specs=3dsx.specs ${arch_flags} -Wl,--gc-sections -Wl,-Map,.map")

set(CMAKE_SYSTEM_PROCESSOR arm CACHE INTERNAL "processor")
set(CMAKE_LIBRARY_ARCHITECTURE arm-none-eabi CACHE INTERNAL "abi")

set(3DS ON)
add_definitions(-D_3DS -D__3DS__ -DARM11)

create_devkit(ARM)

set(CMAKE_FIND_ROOT_PATH ${DEVKITARM}/${CMAKE_LIBRARY_ARCHITECTURE} ${CTRULIB} ${DEVKITPRO}/portlibs/3ds)
