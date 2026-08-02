include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

set(MINIZ_SRC)

list(APPEND MINIZ_SRC
    ${CMAKE_CURRENT_LIST_DIR}/src/miniz.c
    ${CMAKE_CURRENT_LIST_DIR}/src/miniz_tdef.c
    ${CMAKE_CURRENT_LIST_DIR}/src/miniz_tinfl.c
    ${CMAKE_CURRENT_LIST_DIR}/src/miniz_zip.c
    ${CMAKE_CURRENT_LIST_DIR}/include/miniz.h
    ${CMAKE_CURRENT_LIST_DIR}/include/miniz_common.h
    ${CMAKE_CURRENT_LIST_DIR}/include/miniz_export.h
    ${CMAKE_CURRENT_LIST_DIR}/include/miniz_tdef.h
    ${CMAKE_CURRENT_LIST_DIR}/include/miniz_tinfl.h
    ${CMAKE_CURRENT_LIST_DIR}/include/miniz_zip.h
)
