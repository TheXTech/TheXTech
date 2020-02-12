#ifndef PSEUDO_VB_H
#define PSEUDO_VB_H

#include <SDL2/SDL_scancode.h>

#define False false
#define True true
#define If if(
#define Then ) {
#define End }
#define And &&
#define Or ||
#define Len(x) std::strlen(x)

const SDL_Scancode vbKeyEscape = SDL_SCANCODE_ESCAPE;
const SDL_Scancode vbKeySpace = SDL_SCANCODE_SPACE;
const SDL_Scancode vbKeyReturn = SDL_SCANCODE_RETURN;

#endif // PSEUDO_VB_H
