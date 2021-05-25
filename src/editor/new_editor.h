#include "std_picture.h"
#include <string>
#include <vector>
#include <DirManager/dirman.h>

class EditorScreen
{
private:
    typedef enum
    {
        SPECIAL_PAGE_NONE,
        SPECIAL_PAGE_BROWSER,
        SPECIAL_PAGE_BROWSER_CONFIRM,
        SPECIAL_PAGE_EVENTS,
        SPECIAL_PAGE_EVENT_SETTINGS,
        SPECIAL_PAGE_EVENT_DELETION,
        SPECIAL_PAGE_EVENT_LAYERS,
        SPECIAL_PAGE_EVENT_TRIGGER,
        SPECIAL_PAGE_EVENT_MUSIC,
        SPECIAL_PAGE_EVENT_BACKGROUND,
        SPECIAL_PAGE_EVENT_BOUNDS,
        SPECIAL_PAGE_EVENT_CONTROLS,
        SPECIAL_PAGE_EVENT_SOUND,
        SPECIAL_PAGE_SECTION_MUSIC,
        SPECIAL_PAGE_SECTION_BACKGROUND,
        SPECIAL_PAGE_LAYERS,
        SPECIAL_PAGE_LAYER_DELETION,
        SPECIAL_PAGE_OBJ_LAYER,
        SPECIAL_PAGE_OBJ_TRIGGERS,
        SPECIAL_PAGE_BLOCK_CONTENTS,
        SPECIAL_PAGE_LEVEL_EXIT,
        SPECIAL_PAGE_WORLD_SETTINGS,
        SPECIAL_PAGE_FILE,
        SPECIAL_PAGE_FILE_CONFIRM,
    } SpecialPage_t;
    typedef enum
    {
        BROWSER_MODE_NONE,
        BROWSER_MODE_SAVE_NEW,
        BROWSER_MODE_SAVE,
        BROWSER_MODE_OPEN,
    } BrowserMode_t;
    typedef enum
    {
        BROWSER_CALLBACK_NONE,
        BROWSER_CALLBACK_NEW_LEVEL,
        BROWSER_CALLBACK_SAVE_LEVEL,
        BROWSER_CALLBACK_OPEN_LEVEL,
        BROWSER_CALLBACK_NEW_WORLD,
        BROWSER_CALLBACK_SAVE_WORLD,
        BROWSER_CALLBACK_OPEN_WORLD,
        BROWSER_CALLBACK_CUSTOM_MUSIC,
    } BrowserCallback_t;

    // current pages
    SpecialPage_t m_special_page = SPECIAL_PAGE_NONE;
    int m_special_subpage = 0;
    int m_last_mode;
    int m_NPC_page = 0;
    int m_Block_page = 0;
    int m_BGO_page = 0;
    int m_layers_page = 0;
    int m_events_page = 0;
    int m_sounds_page = 0;
    int m_tile_page = 0;
    int m_music_page = 0;
    int m_background_page = 0;
    int m_current_event = 0;

    // file browser info
    std::string* m_file_target = nullptr;
    std::string m_root_path;
    std::vector<std::string> m_target_exts;
    std::string m_cur_path;
    std::string m_cur_file;
    std::vector<std::string> m_cur_path_dirs;
    std::vector<std::string> m_cur_path_files;
    DirMan m_dirman;
    bool m_path_synced = false;
    BrowserMode_t m_browser_mode = BROWSER_MODE_NONE;
    BrowserCallback_t m_browser_callback = BROWSER_CALLBACK_NONE;
    SpecialPage_t m_last_special_page = SPECIAL_PAGE_NONE;

    void EnsureWorld();
    void EnsureLevel();

    void SyncPath();
    void GoToSuper();
    void ValidateExt(std::string& cur_file);
    bool FileExists(const std::string& cur_file);
    void StartFileBrowser(std::string* file_target, const std::string root_path, const std::string current_path, const std::vector<std::string> target_exts, BrowserMode_t browser_mode = BROWSER_MODE_OPEN, BrowserCallback_t browser_callback = BROWSER_CALLBACK_NONE);
    void FileBrowserSuccess();
    void FileBrowserFailure();
    void FileBrowserCleanup();

    bool UpdateButton(int x, int y, StdPicture &im, bool sel,
        int src_x = 0, int src_y = 0, int src_w = 32, int src_h = 32);

    void UpdateNPC(int x, int y, int type);
    void UpdateNPCGrid(int x, int y, const int* types, int n_npcs, int n_cols);
    void UpdateNPCScreen();

    void UpdateBlock(int x, int y, int type);
    void UpdateBlockGrid(int x, int y, const int* types, int n_blocks, int n_cols);
    void UpdateBlockScreen();

    void UpdateBGO(int x, int y, int type);
    void UpdateBGOGrid(int x, int y, const int* types, int n_bgos, int n_cols);
    void UpdateBGOScreen();

    void UpdateWaterScreen();
    void UpdateWarpScreen();

    void UpdateSectionsScreen();

    void UpdateWorldSettingsScreen();

    void UpdateLayersScreen();

    void UpdateEventsScreen();
    void UpdateEventSettingsScreen();
    void UpdateEventsSubScreen();

    void UpdateTile(int x, int y, int type);
    void UpdateTileGrid(int x, int y, const int* types, int n_tiles, int n_cols);
    void UpdateTileScreen();

    void UpdateScene(int x, int y, int type);
    void UpdateSceneGrid(int x, int y, const int* types, int n_scenes, int n_cols);
    void UpdateSceneScreen();

    void UpdateLevel(int x, int y, int type);
    void UpdateLevelGrid(int x, int y, const int* types, int n_levels, int n_cols);
    void UpdateLevelScreen();

    void UpdatePath(int x, int y, int type);
    void UpdatePathGrid(int x, int y, const int* types, int n_paths, int n_cols);
    void UpdatePathScreen();

    void UpdateSelectListScreen();

    void UpdateFileScreen();
    void UpdateBrowserScreen();

public:
    bool active = true;
    void UpdateEditorScreen();
    void UpdateSelectorBar(bool level_screen = false);
    void ResetCursor();
    // set m_NPC_page based on the editor cursor
    void FocusNPC();
    // set m_Block_page based on the editor cursor
    void FocusBlock();
    // set m_BGO_page based on the editor cursor
    void FocusBGO();
    // set m_tile_page based on the editor cursor
    void FocusTile();
};

extern EditorScreen editorScreen;
