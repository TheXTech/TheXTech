#include "globals.h"
#include "sound.h"
#include "write_common.h"
#include <PGE_File_Formats/file_formats.h>
#include "Logger/logger.h"

void SaveWorld(std::string FilePath, int format, int version)   // Saves the world!
{
    WorldData out;
    WorldTerrainTile terra;
    WorldScenery scene;
    WorldPathTile path;
    WorldLevelTile level;
    WorldMusicBox musicbox;

    FileFormats::CreateWorldData(out);

    out.EpisodeTitle = WorldName;
    out.IntroLevel_file = StartLevel;
    out.HubStyledWorld = NoMap;
    out.restartlevel = RestartLevel;
    out.stars = MaxWorldStars;

    out.nocharacter.clear();
    for(int i = 1; i <= 5; ++i)
        out.nocharacter.push_back(blockCharacter[i]);
    out.charactersToS64();

    for(int A = 1; A <= maxWorldCredits; ++A)
    {
        if(!out.authors.empty())
            out.authors.push_back('\n');
        out.authors.append(WorldCredits[A]);
    }

    // remove all trailing new-lines and spacings
    while(!out.authors.empty() && (out.authors.back() == '\n' || out.authors.back() == ' '))
        out.authors.pop_back();

    for(int i = 1; i <= numTiles; ++i)
    {
        auto &t = Tile[i];
        terra = FileFormats::CreateWldTile();
        terra.x = t.Location.X;
        terra.y = t.Location.Y;
        terra.id = t.Type;
        terra.meta.array_id = out.tile_array_id++;
        out.tiles.push_back(terra);
    }

    for(int i = 1; i <= numScenes; ++i)
    {
        auto &s = Scene[i];
        scene = FileFormats::CreateWldScenery();
        scene.x = s.Location.X;
        scene.y = s.Location.Y;
        scene.id = s.Type;
        scene.meta.array_id = out.scene_array_id++;
        out.scenery.push_back(scene);
    }

    for(int i = 1; i <= numWorldPaths; ++i)
    {
        auto &p = WorldPath[i];
        path = FileFormats::CreateWldPath();
        path.x = p.Location.X;
        path.y = p.Location.Y;
        path.id = p.Type;
        path.meta.array_id = out.path_array_id++;
        out.paths.push_back(path);
    }

    for(int i = 1; i <= numWorldLevels; ++i)
    {
        auto &s = WorldLevel[i];
        level = FileFormats::CreateWldLevel();
        level.x = s.Location.X;
        level.y = s.Location.Y;
        level.id = s.Type;
        level.lvlfile = s.FileName;
        level.title= s.LevelName;
        level.top_exit = s.LevelExit[1];
        level.left_exit = s.LevelExit[2];
        level.bottom_exit = s.LevelExit[3];
        level.right_exit = s.LevelExit[4];
        level.meta.array_id = out.path_array_id++;
        level.entertowarp = s.StartWarp;
        level.alwaysVisible = s.Visible;
        level.gamestart = s.Start;
        level.gotox = s.WarpX;
        level.gotoy = s.WarpY;
        level.pathbg = s.Path;
        level.bigpathbg = s.Path2;
        out.levels.push_back(level);
    }

    for(int A = 1; A <= numWorldMusic; A++)
    {
        auto &s = WorldMusic[A];
        musicbox = FileFormats::CreateWldMusicbox();
        musicbox.x = s.Location.X;
        musicbox.y = s.Location.Y;
        musicbox.id = s.Type;
        musicbox.music_file = GetS(s.MusicFile);
        musicbox.meta.array_id = out.musicbox_array_id++;
        out.music.push_back(musicbox);
    }

    if(!FileFormats::SaveWorldFile(out, FilePath, (FileFormats::WorldFileFormat)format, version))
    {
        pLogWarning("Error while saving the level file: %s", out.meta.ERROR_info.c_str());
        PlaySound(SFX_Smash);
        return;
    }

    PlaySound(SFX_GotItem);
}
