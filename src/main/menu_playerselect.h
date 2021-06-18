#ifndef MENU_PLAYERSELECT_H

#define MENU_PLAYERSELECT_H

extern int g_charSelect[maxLocalPlayers];

void menuPlayerSelect_Start();
void menuPlayerSelect_Resume();
void menuPlayerSelect_Render(int minPlayers);

// return values:
// -1 back
// 0 continue
// 1 next
int menuPlayerSelect_Logic(int minPlayers);

#endif // MENU_PLAYERSELECT_H