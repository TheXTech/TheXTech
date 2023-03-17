find_program(NACPTOOL nacptool REQUIRED)
message("-- napctool found: ${NACPTOOL}")

find_program(ELF2NRO elf2nro REQUIRED)
message("-- elf2nro found: ${ELF2NRO}")

add_custom_target(thextech_nacp ALL
    "${NACPTOOL}" --create
    "${THEXTECH_PACKAGE_NAME}"
    "${CPACK_PACKAGE_VENDOR}"
    "${THEXTECH_VERSION_1}.${THEXTECH_VERSION_2}.${THEXTECH_VERSION_3}"
    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/thextech.nacp"
    DEPENDS thextech
    BYPRODUCTS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/thextech.nacp"
)

add_custom_target(thextech_nro ALL
    "${ELF2NRO}"
    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/thextech.elf"
    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/thextech.nro"
    "--icon=${TheXTech_SOURCE_DIR}/resources/switch/thextech-logo.jpg"
    "--nacp=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/thextech.nacp"
    DEPENDS thextech thextech_nacp
)
