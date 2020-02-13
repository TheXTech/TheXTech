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
const SDL_Scancode vbKeyDown = SDL_SCANCODE_DOWN;
const SDL_Scancode vbKeyLeft = SDL_SCANCODE_LEFT;
const SDL_Scancode vbKeyUp = SDL_SCANCODE_UP;
const SDL_Scancode vbKeyRight = SDL_SCANCODE_RIGHT;
const SDL_Scancode vbKeyShift = SDL_SCANCODE_LSHIFT;
const SDL_Scancode vbKeyZ = SDL_SCANCODE_Z;
const SDL_Scancode vbKeyX = SDL_SCANCODE_X;
const SDL_Scancode vbKeyA = SDL_SCANCODE_A;
const SDL_Scancode vbKeyS = SDL_SCANCODE_S;

#endif // PSEUDO_VB_H
