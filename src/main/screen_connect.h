#ifndef SCREEN_CONNECT_H

#define SCREEN_CONNECT_H

// for maxLocalPlayers
#include "../global_constants.h"

extern int g_charSelect[maxLocalPlayers];

namespace ConnectScreen
{

void MainMenu_Start(int minPlayers);
void Reconnect_Start();
void DropAdd_Start();

void Render();

// return values:
// -1 back
// 0 continue
// 1 next
int Logic();

// used to track which characters
//   were present in last case with
//   SwapCharAllowed() true.
void SaveChars();

} // namespace ConnectScreen

#endif // SCREEN_CONNECT_H