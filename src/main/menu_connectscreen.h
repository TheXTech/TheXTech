#ifndef MENU_CONNECTSCREEN_H

#define MENU_CONNECTSCREEN_H

extern int g_charSelect[maxLocalPlayers];

namespace ConnectScreen
{

void MainMenu_Start(int minPlayers);
void Reconnect_Start();

void Render();

// return values:
// -1 back
// 0 continue
// 1 next
int Logic();

} // namespace ConnectScreen

#endif // MENU_CONNECTSCREEN_H