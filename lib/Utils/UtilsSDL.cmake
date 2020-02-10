# message("Path to UtilsSDL is [${CMAKE_CURRENT_LIST_DIR}]")
include_directories(${CMAKE_CURRENT_LIST_DIR}/../)

set(UTILS_SDL_SRCS)

list(APPEND UTILS_SDL_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/sdl_file.cpp
)

