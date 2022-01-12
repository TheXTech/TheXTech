#include "lunacounter_record.h"
#include <SDL2/SDL_endian.h>

void DeathRecord::Save(FILE *openfile)
{
    // Write character count
    uint32_t tempint = (uint32_t)LevelName.size();

    tempint = SDL_SwapLE32(tempint);
    std::fwrite(&tempint, 1, sizeof(uint32_t), openfile);

    // Write string data
    int16_t nullt = 0;
    std::fwrite(LevelName.data(), 1, tempint, openfile);
    std::fwrite(&nullt, 1, sizeof(int16_t), openfile);

    // Write death count
    int32_t tempsint = SDL_SwapLE32(Deaths);
    std::fwrite(&tempsint, 1, sizeof(int32_t), openfile);
}

void DeathRecord::Load(FILE *openfile)
{
    uint32_t tempint;
    int32_t tempsint;
    char buf[151];
    SDL_memset(buf, 0, 151);

    // Read string length
    uint32_t length;
    uint32_t skip = 0;
    std::fread(&tempint, 1, sizeof(uint32_t), openfile);
    length = SDL_SwapLE32(tempint);

    if(length > 150)
    {
        skip = length - 150;
        length = 150;
    }

    // Read string data
    std::fread(buf, 1, length, openfile);
    if(skip > 0)
    std::fseek(openfile, skip, SEEK_CUR);

    LevelName = std::string(buf);
    std::fseek(openfile, 2, SEEK_CUR);

    // Read death count
    std::fread(&tempsint, 1, sizeof(int32_t), openfile);
    Deaths = SDL_SwapLE32(tempsint);
}
