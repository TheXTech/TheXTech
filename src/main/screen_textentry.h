#ifndef SCREEN_TEXTENTRY_H
#define SCREEN_TEXTENTRY_H

namespace TextEntryScreen
{

const std::string& Run(const std::string& Prompt, const std::string Value = "");

void Render();
bool Logic();

void CursorLeft();
void CursorRight();
void Insert(const char* text);
void Backspace();
void Commit();

} // namespace TextEntryScreen

#endif // #ifndef SCREEN_TEXTENTRY_H