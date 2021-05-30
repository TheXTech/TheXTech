#include "globals.h"
#include "sound.h"
#include "write_common.h"

void SaveWorld(std::string FilePath)   // Saves the world!
{
    int A = 0;
    int B = 0;

    FILE* f = fopen(FilePath.c_str(), "wb");
    if (!f)
        return;
    fwritenum(f, curRelease);
    fwritestr(f, WorldName);
    for (A = 1; A <= 5; A++)
    {
        fwritebool(f, blockCharacter[A]);
    }
    fwritestr(f, StartLevel);
    fwritebool(f, NoMap);
    fwritebool(f, RestartLevel);
    fwritenum(f, MaxWorldStars);
    for (A = 1; A <= 5; A++)
    {
        fwritestr(f, WorldCredits[A]);
    }
    for (A = 1; A <= numTiles; A++)
    {
        fwritenum(f, Tile[A].Location.X);
        fwritenum(f, Tile[A].Location.Y);
        fwritenum(f, Tile[A].Type);
    }
    fprintf(f, "next\r\n");
    for (A = 1; A <= numScenes; A++)
    {
        fwritenum(f, Scene[A].Location.X);
        fwritenum(f, Scene[A].Location.Y);
        fwritenum(f, Scene[A].Type);
    }
    fprintf(f, "next\r\n");
    for (A = 1; A <= numWorldPaths; A++)
    {
        fwritenum(f, WorldPath[A].Location.X);
        fwritenum(f, WorldPath[A].Location.Y);
        fwritenum(f, WorldPath[A].Type);
    }
    fprintf(f, "next\r\n");
    for (A = 1; A <= numWorldLevels; A++)
    {
        fwritenum(f, WorldLevel[A].Location.X);
        fwritenum(f, WorldLevel[A].Location.Y);
        fwritenum(f, WorldLevel[A].Type);
        fwritestr(f, WorldLevel[A].FileName);
        fwritestr(f, WorldLevel[A].LevelName);
        for (B = 1; B <= 4; B++)
            fwritenum(f, WorldLevel[A].LevelExit[B]);
        fwritenum(f, WorldLevel[A].StartWarp);
        fwritebool(f, WorldLevel[A].Visible);
        fwritebool(f, WorldLevel[A].Path);
        fwritebool(f, WorldLevel[A].Start);
        fwritenum(f, WorldLevel[A].WarpX);
        fwritenum(f, WorldLevel[A].WarpY);
        fwritebool(f, WorldLevel[A].Path2);
    }
    fprintf(f, "next\r\n");
    for (A = 1; A <= numWorldMusic; A++)
    {
        fwritenum(f, WorldMusic[A].Location.X);
        fwritenum(f, WorldMusic[A].Location.Y);
        fwritenum(f, WorldMusic[A].Type);
    }
    fprintf(f, "next\r\n");
    fclose(f);
    PlaySound(12);
}
