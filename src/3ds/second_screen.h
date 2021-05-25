#include <string>

extern uint64_t perf_times[10];
extern uint64_t graphics_times[17];
extern bool debugMode;
extern unsigned int num_textures_loaded;
extern unsigned int num_big_textures_loaded;
extern unsigned int num_waves_loaded;

void writeMessage(std::string message);

void drawSecondScreen();

void drawSecondScreen2();
