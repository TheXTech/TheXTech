#ifndef PSEUDO_VB_H
#define PSEUDO_VB_H

#include <SDL2/SDL_scancode.h>

#define False false
#define True true
#define If if(
#define Then ) {
#define End }
#define And &&
#define Not !
#define Or ||
#define Len(x) std::strlen(x)

const int vbKeyEscape = SDL_SCANCODE_ESCAPE;
const int vbKeySpace = SDL_SCANCODE_SPACE;

#endif // PSEUDO_VB_H
