# Major
set(THEXTECH_VERSION_1 1)
# Minor
set(THEXTECH_VERSION_2 3)
# Revision
set(THEXTECH_VERSION_3 7)
# Patch
set(THEXTECH_VERSION_4 0)
# Type of version: "-alpha","-beta","-dev", or "" aka "release"
set(THEXTECH_VERSION_REL "-dev")

# Static version values for F-Droid to parse (PLEASE KEEP IT IN SYNC WITH VERSION NUMBER PARTS FROM ABOVE)
set(THEXTECH_ANDROID_VERSION_NAME "1.3.7-dev")
set(THEXTECH_ANDROID_VERSION_CODE "1030700")

# Defining global macros
add_definitions(-DTHEXTECH_VERSION_1=${THEXTECH_VERSION_1})
add_definitions(-DTHEXTECH_VERSION_2=${THEXTECH_VERSION_2})
add_definitions(-DTHEXTECH_VERSION_3=${THEXTECH_VERSION_3})
add_definitions(-DTHEXTECH_VERSION_4=${THEXTECH_VERSION_4})
add_definitions(-DTHEXTECH_VERSION_REL=${THEXTECH_VERSION_REL})

# Buildin the version name
set(THEXTECH_VERSION_STRING "${THEXTECH_VERSION_1}.${THEXTECH_VERSION_2}")

if(NOT ${THEXTECH_VERSION_3} EQUAL 0 OR NOT ${THEXTECH_VERSION_4} EQUAL 0)
    string(CONCAT THEXTECH_VERSION_STRING "${THEXTECH_VERSION_STRING}" ".${THEXTECH_VERSION_3}")
endif()

if(NOT ${THEXTECH_VERSION_4} EQUAL 0)
    string(CONCAT THEXTECH_VERSION_STRING "${THEXTECH_VERSION_STRING}" ".${THEXTECH_VERSION_4}")
endif()

if(NOT "${THEXTECH_VERSION_REL}" STREQUAL "")
    string(CONCAT THEXTECH_VERSION_STRING "${THEXTECH_VERSION_STRING}" "${THEXTECH_VERSION_REL}")
endif()

# Building the version code (for Android)
math(EXPR THEXTECH_VERSION_CODE "1000000 * ${THEXTECH_VERSION_1} + 10000 * ${THEXTECH_VERSION_2} + 100 * ${THEXTECH_VERSION_3} + ${THEXTECH_VERSION_4}")

message("== TheXTech version ${THEXTECH_VERSION_STRING} (${THEXTECH_VERSION_CODE}) ==")

# A reminder that will compare dynamically computed values with a static value, and will fail a configure if mismatch:
# ==================================================

if(NOT "${THEXTECH_ANDROID_VERSION_NAME}" STREQUAL "${THEXTECH_VERSION_STRING}")
    message(FATAL_ERROR "Android version name value doesn't matching to the primary version name\n(expected ${THEXTECH_VERSION_STRING}, ${THEXTECH_ANDROID_VERSION_NAME} actually)")
endif()

if(NOT "${THEXTECH_ANDROID_VERSION_CODE}" STREQUAL "${THEXTECH_VERSION_CODE}")
    message(FATAL_ERROR "Android version code doesn't matching to the primary version code\n(expected ${THEXTECH_VERSION_CODE}, ${THEXTECH_ANDROID_VERSION_CODE} actually)")
endif()
# ==================================================
