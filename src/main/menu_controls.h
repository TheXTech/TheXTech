#ifndef MENU_CONTROLS_H

#define MENU_CONTROLS_H

extern bool g_pollingInput;

void menuControls_Render();
// return values:
// -1 back
// 0 continue
int menuControls_Logic();

#endif // MENU_CONTROLS_H