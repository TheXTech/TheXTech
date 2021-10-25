namespace TextEntryScreen
{

extern std::string Text;

void Init(const std::string& Prompt, const std::string Value = "");
void Render();
bool Logic();

void CursorLeft();
void CursorRight();
void Insert(const char* text);
void Backspace();
void Commit();
};
