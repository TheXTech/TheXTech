#ifndef SCREEN_PAUSE_H

#define SCREEN_PAUSE_H

namespace PauseScreen
{

void Init(bool LegacyPause);

bool Logic(int plr);
void Render();

} // namespace PauseScreen

#endif // #ifndef SCREEN_PAUSE_H