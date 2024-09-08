#include <fmt_format_ne.h>

#include "core/render.h"

#include "global_constants.h"
#include "controls.h"
#include "sound.h"
#include "globals.h"
#include "gfx.h"
#include "graphics.h"
#include "player.h"
#include "config.h"
#include "npc_id.h"
#include "npc/npc_queues.h"

#include "graphics/gfx_marquee.h"

#include "main/game_info.h"

#include "main/screen_connect.h"
#include "main/screen_quickreconnect.h"
#include "main/menu_main.h"
#include "main/game_strings.h"
#include "main/speedrunner.h"

namespace ConnectScreen
{

enum class PlayerState
{
    Disconnected,   // missing controller for player
    SelectChar,     // left side of main screen
    ControlsMenu,   // right side of main screen
    Reconnecting,   // player requested to reconnect controller
    SelectProfile,  // selecting controls profile
    ConfirmProfile, // checking that player understands controls profile
    ForceDrop,      // player wants to force disconnected other players to drop (to leave drop/add screen)
};

enum class Context
{
    MainMenu,
    LegacyMenu,
    DropAdd,
};

static Context s_context;

struct CharInfo
{
private:
    bool char_present[numCharacters];

public:
    int dead_count = 0;
    int max_players = 0;

    bool accept(int c, int p, uint8_t current_add)
    {
        if(c < 1 || c > numCharacters)
            return false;

        // don't accept a blocked char, unless it has been present before
        if(blockCharacter[c] && !(s_context == Context::DropAdd && this->char_present[c - 1]))
            return false;

        // allow everything if char swap is allowed
        if(SwapCharAllowed())
            return true;

        // automatically recognize novel adds
        if(p >= this->max_players)
            current_add = 2;

        // if not a current add, don't allow changing character
        if(!current_add)
            return false;

        bool novel_add = (current_add >= 2);

        // require the character to have been selected previously if the player is not a novel add
        if(!novel_add && !this->char_present[c - 1])
            return false;

        return true;
    }

    void reset()
    {
        for(int c = 0; c < numCharacters; c++)
            char_present[c] = false;

        max_players = l_screen->player_count;
        dead_count = 0;
    }

    void mark_present()
    {
        for(int p = 0; p < l_screen->player_count; p++)
        {
            int c = Player[l_screen->players[p]].Character - 1;

            if(0 <= c && c < numCharacters)
                char_present[c] = true;
        }

        if(max_players < l_screen->player_count)
            max_players = l_screen->player_count;
    }

    // checks if the active players' characters have been changed by gameplay logic (not drop/add)
    bool chars_changed()
    {
        for(int p = 0; p < l_screen->player_count; p++)
        {
            int c = Player[l_screen->players[p]].Character - 1;

            if(0 <= c && c < numCharacters)
            {
                if(!char_present[c])
                    return true;
            }
        }

        return false;
    }

    // checks if a character has been present
    bool char_was_present(int c)
    {
        if(c < 1 || c > numCharacters)
            return false;

        return this->char_present[c - 1];
    }

    // marks a single character as present
    void mark_char_present(int c)
    {
        if(c < 1 || c > numCharacters)
            return;

        this->char_present[c - 1] = true;
    }
};

struct PlayerBox
{
    PlayerState m_state = PlayerState::Disconnected;
    bool m_input_ready = false;

    // set if the player was added during this invocation of the Drop/Add screen; set to 2 if a novel add (the number of players is greater than it had been previously)
    uint8_t m_current_add = 0;

private:
    // just added a player in multiplayer menu char select: play DropItem if appropriate, and block the start and back keys for non-P1 players
    bool m_just_added = false;

    // progress towards entering the Konami cheat to set g_forceCharacter
    uint8_t m_konami_bits = false;

    //! multiuse tracker for current menu item / action confirm progress
    int m_menu_item = 0;

    // state of hint text marquee
    MarqueeState m_marquee_state;

    Controls::InputMethodProfile* m_last_profile = nullptr;

    // calculates the player index using pointer arithmetic
    int CalcIndex() const;

    // checks if a character is validly available for this player
    bool CharAvailable(int c, bool ghost_mode = false);

    // adds player if not present, and updates player's character
    void UpdatePlayer();

    // makes sure that l_screen->charSelect[p] is valid
    void ValidateChar(bool ghost_mode = false);

    // does logic for an item the mouse is currently over
    bool MouseItem(int i);

    // do all mouse logic and rendering for an item at a certain location
    bool MenuItem_Mouse_Render(int i, const std::string& label, int X, int Y, bool mouse, bool render, int end_X = 0);

    // draws the selected character at a location
    bool DrawChar(int x, int w, int y, int h, bool show_name);

    // do rendering and mouse logic for SinglePlayer char select
    // returns -1 if player is ready to return, +1 if player is ready to proceed
    int Mouse_Render_1P(bool render);

public:
    // initialize the state for the current context
    void Init();

    // do rendering and mouse logic
    // returns -1 if player is ready to return, +1 if player is ready to proceed
    int Mouse_Render(bool render, int x, int y, int w, int h);

    // returns -1 if player is ready to return, +1 if player is ready to proceed
    int Logic();

    // check if it is possible to rotate char
    bool CanChangeChar();

    // rotate character
    void PrevChar();
    void NextChar();

    // registers the appropriate cursor input
    void Left();
    void Right();

    void Up();
    void Down();

    // returns true if this player is ready to return to previous menu
    bool Back();

    // returns true if this player is ready to proceed
    bool Do();

    // returns true if this player is ready to proceed
    bool Ready() const;
};

static bool s_controls_dirty = false;
static int s_minPlayers = 1;

static std::array<PlayerBox, maxLocalPlayers> s_players;
static std::array<uint8_t, maxLocalPlayers> s_recent_char{};

static CharInfo s_char_info;

static void s_logRecentChars()
{
    // reset recent chars to 0
    s_recent_char = {};

    // update
    for(int i = 0; i < maxLocalPlayers; i++)
    {
        if(s_players[i].m_state != PlayerState::Disconnected)
            s_recent_char[i] = l_screen->charSelect[i];
    }

    // if controls dirty, save them
    if(s_controls_dirty)
    {
        Controls::SaveConfig();
        s_controls_dirty = false;
    }
}

static inline void Render_PCursor(int x, int y)
{
    if(GFX.PCursor.inited)
        XRender::renderTexture(x, y, GFX.PCursor);
    else
        XRender::renderTextureFL(x, y, GFX.MCursor[1].w, GFX.MCursor[1].h, GFX.MCursor[1], 0, 0, 90.0, nullptr, X_FLIP_NONE);
}

static inline void Render_PCursorDownwards(int x, int y, XTColor color = XTColor())
{
    if(GFX.PCursor.inited)
        XRender::renderTextureFL(x - GFX.PCursor.h / 2, y - GFX.PCursor.w, GFX.PCursor.w, GFX.PCursor.h, GFX.PCursor, 0, 0, 90.0, nullptr, X_FLIP_NONE, color);
    else
        XRender::renderTexture(x - GFX.MCursor[2].w / 2, y - GFX.MCursor[2].h, GFX.MCursor[2], color);
}

static inline bool Is1P()
{
    return s_context == Context::LegacyMenu;
}

static inline bool IsMenu()
{
    return (s_context == Context::MainMenu || s_context == Context::LegacyMenu);
}

// check if PlayerBox p (zero-indexed) can drop self
static inline bool ControlsMenu_ShowDrop(int p)
{
    // don't allow dropping below required player count
    if(s_context == Context::DropAdd && l_screen->player_count <= s_minPlayers)
        return false;

    // never show drop icon at main menu
    if(s_context == Context::MainMenu)
        return false;

    int plr_A = l_screen->players[p];

    // check that there is a different living player
    for(int B = 1; B <= numPlayers; B++)
    {
        if(B == plr_A)
            continue;

        if(!Player[B].Dead && Player[B].TimeToLive == 0)
            return true;
    }

    return false;
}

static inline int BoxCount(bool strict = false)
{
    int n = (int)Controls::g_InputMethods.size();

    if(!BattleMode)
        n += 1;

    if(n < s_minPlayers)
        n = s_minPlayers;

    if(s_context == Context::MainMenu && n < 2)
        n = 2;

    // disable entering >2P in vanilla mode
    if(n > 2 && !g_config.multiplayer_pause_controls)
        n = 2;

    // disable entering >1P from legacy menu
    if(s_context == Context::LegacyMenu)
        n = 1;

    // disable entering >1P when 2P is disabled
    if(g_gameInfo.disableTwoPlayer && !g_forceCharacter)
        n = 1;

    // but do show all players once they are already present
    if(!strict && n < (int)Controls::g_InputMethods.size())
        n = (int)Controls::g_InputMethods.size();

    // limit to maxLocalPlayers to prevent out-of-bounds access
    if(n > maxLocalPlayers)
        n = maxLocalPlayers;

    return n;
}

static void s_InitBlockCharacter()
{
    g_forceCharacter = false;
    For(A, 1, numCharacters)
    {
        if(MenuMode == MENU_CHARACTER_SELECT_NEW_BM)
            blockCharacter[A] = false;
        else
            blockCharacter[A] = SelectWorld[selWorld].blockChar[A];
    }
}

void MainMenu_Start(int minPlayers)
{
    s_minPlayers = minPlayers;
    s_context = Context::MainMenu;

    // clear input methods if invalid
    if((int)Controls::g_InputMethods.size() > BoxCount(true))
        Controls::ClearInputMethods();

    for(int i = 0; i < maxLocalPlayers; i++)
        l_screen->charSelect[i] = 0;

    s_InitBlockCharacter();

    for(int i = 0; i < maxLocalPlayers; i++)
    {
        s_players[i] = PlayerBox();
        s_players[i].Init();
    }

    MenuMouseRelease = false;
    MenuCursorCanMove = false;

    s_char_info.reset();

    // prepare for first frame
    Logic();
}

void LegacyMenu_Start()
{
    s_minPlayers = 1;
    s_context = Context::LegacyMenu;

    // clear input methods if invalid
    if((int)Controls::g_InputMethods.size() > BoxCount(true))
        Controls::ClearInputMethods();

    for(int i = 0; i < maxLocalPlayers; i++)
        l_screen->charSelect[i] = 0;

    s_InitBlockCharacter();

    for(int i = 0; i < maxLocalPlayers; i++)
    {
        s_players[i] = PlayerBox();
        s_players[i].Init();
    }

    // create an empty input slot
    if(Controls::g_InputMethods.size() == 0)
        Controls::g_InputMethods.push_back(nullptr);

    s_players[0].m_state = PlayerState::SelectChar;

    MenuMouseRelease = false;
    MenuCursorCanMove = false;

    s_char_info.reset();

    // prepare for first frame
    Logic();
}

void DropAdd_Start()
{
    if(BattleMode)
        s_minPlayers = l_screen->player_count;
    else
        s_minPlayers = 1;

    s_context = Context::DropAdd;

    for(int i = 0; i < maxLocalPlayers; i++)
    {
        s_players[i] = PlayerBox();
        s_players[i].Init();
    }

    MenuMouseRelease = false;
    MenuCursorCanMove = false;

    // update the present characters, IF they got changed by level logic
    if(s_char_info.chars_changed())
        SaveChars();

    // prepare for first frame
    Logic();
}

int PlayerBox::CalcIndex() const
{
    if(this >= &s_players[0] && this <= &s_players[maxLocalPlayers - 1])
    {
        return this - &s_players[0];
    }

    return 0;
}

bool PlayerBox::Ready() const
{
    switch(m_state)
    {
    case PlayerState::Disconnected:
    case PlayerState::Reconnecting:
    case PlayerState::SelectProfile:
    case PlayerState::ConfirmProfile:
        return false;

    default:
        return true;
    }
}

void PlayerBox::Init()
{
    int p = CalcIndex();

    if(p < (int)Controls::g_InputMethods.size() && Controls::g_InputMethods[p]
            && (s_context != Context::DropAdd || p < l_screen->player_count))
    {
        m_state = PlayerState::SelectChar;
    }
    else
        m_state = PlayerState::Disconnected;

    if(s_context == Context::DropAdd && p < l_screen->player_count)
    {
        l_screen->charSelect[p] = Player[l_screen->players[p]].Character;
    }
    else
    {
        l_screen->charSelect[p] = s_recent_char[p];
        ValidateChar(true);
    }
}

bool PlayerBox::CharAvailable(int c, bool ghost_mode)
{
    // check if invalid
    if(c < 1 || c > numCharacters)
        return false;

    int p = CalcIndex();

    // check if blocked for use in the level (blocked in episode, or a different character was previously dropped in the level)
    if(!s_char_info.accept(c, p, ghost_mode ? 1 : m_current_add))
        return false;

    int max_check = ghost_mode ? BoxCount() : (int)Controls::g_InputMethods.size();

    if(max_check < s_minPlayers)
        max_check = s_minPlayers;

    // check if a different player has selected or is currently selecting the character
    for(int i = 0; i < maxLocalPlayers && i < max_check; i++)
    {
        // fine if self
        if(i == p)
            continue;

        if(l_screen->charSelect[i] == c)
            return false;
    }

    return true;
}

static bool CheckDone()
{
    // What is the first player that is not done?
    int n = (int)Controls::g_InputMethods.size();
    if(n < s_minPlayers)
        n = s_minPlayers;
    if(n > maxLocalPlayers)
        n = maxLocalPlayers;

    // What is the first player that is not done?
    int notDone;
    for(notDone = 0; notDone < n; notDone++)
    {
        if(notDone == (int)Controls::g_InputMethods.size() || !Controls::g_InputMethods[notDone])
            break;

        if(!s_players[notDone].Ready())
            break;
    }

    // not enough players / controllers -> not done
    if(notDone < s_minPlayers || (int)Controls::g_InputMethods.size() < s_minPlayers)
        return false;

    // need all connected players to be done
    if(notDone == (int)Controls::g_InputMethods.size() || notDone == maxLocalPlayers)
        return true;

    return false;
}


// helper function when trying to force exit the drop/add screen
// returns zero-indexed player that can be dropped (is currently disconnected), -3 if the player set is already complete, -2 if exiting the menu is blocked, and -1 if dropping a player will not make the player set complete
static int CheckCanDrop()
{
    // What is the first player that is not done?
    int n = (int)Controls::g_InputMethods.size();
    if(n < s_minPlayers)
        n = s_minPlayers;
    if(n > maxLocalPlayers)
        n = maxLocalPlayers;

    int are_present = 0;
    int can_drop = -3;

    // What is the first player that is not done?
    for(int notDone = 0; notDone < n; notDone++)
    {
        if(s_players[notDone].Ready())
            are_present++;
        else if(s_players[notDone].m_state == PlayerState::Disconnected || s_players[notDone].m_state == PlayerState::Reconnecting)
            can_drop = notDone;
        else if(s_players[notDone].m_state == PlayerState::ConfirmProfile)
            return -2;
    }

    // not enough players / controllers -> can't drop to exit
    if(are_present < s_minPlayers)
        return -1;

    return can_drop;
}

static void Player_Remove(int p)
{
    // if it was a novel add, mark their character as allowed
    if(s_players[p].m_current_add == 2)
        s_char_info.mark_char_present(l_screen->charSelect[p]);

    Controls::DeleteInputMethodSlot(p);

    // move all player boxes back, and reset final box
    for(int p2 = p; p2 + 1 < maxLocalPlayers; p2++)
    {
        s_players[p2] = s_players[p2 + 1];
        s_players[p2].m_input_ready = false;
        l_screen->charSelect[p2] = l_screen->charSelect[p2 + 1];
    }

    s_players[maxLocalPlayers - 1] = PlayerBox();

    // in drop-add, remove the Player officially in main game engine
    if(p < l_screen->player_count && s_context == Context::DropAdd)
    {
        if((Player[l_screen->players[p]].Dead || Player[l_screen->players[p]].TimeToLive > 0) && Player[l_screen->players[p]].Effect != PLREFF_COOP_WINGS)
            s_char_info.dead_count++;

        DropPlayer(l_screen->players[p]);
    }
}

static void Player_Swap(int p1, int p2)
{
    if(p1 == p2)
        return;

    if(p1 >= (int)Controls::g_InputMethods.size() || p2 >= (int)Controls::g_InputMethods.size())
        return;

    std::swap(s_players[p1], s_players[p2]);

    std::swap(l_screen->charSelect[p1], l_screen->charSelect[p2]);

    s_players[p1].m_input_ready = false;
    s_players[p2].m_input_ready = false;
}

// transform players and items based on character
static void DoTransform(int p, int ch)
{
    for(int A = 1; A <= numPlayers; A++)
    {
        if(A % (p + 1) == 0)
        {
            if(Player[A].Character != ch)
            {
                Player[A].Character = ch;
                SizeCheck(Player[A]);
            }
        }
    }

    for(int A : NPCQueues::Active.no_change)
    {
        if(A % (p + 1) == 0 && NPC[A].Type == NPCID_PLR_FIREBALL)
            NPC[A].Special = ch;
    }
}

void PlayerBox::UpdatePlayer()
{
    int p = CalcIndex();

    // if initialized, then update character
    if(p < l_screen->player_count)
    {
        if(l_screen->charSelect[p] != Player[l_screen->players[p]].Character)
            SwapCharacter(l_screen->players[p], l_screen->charSelect[p]);
    }
    // otherwise, add new player!
    else
    {
        // swap p with the first non-existent player slot
        Player_Swap(l_screen->player_count, p);

        // after AddPlayer, numPlayers is always the new player
        AddPlayer(l_screen->charSelect[l_screen->player_count], *l_screen);

        // add as dead if dead player was dropped in this level
        if(s_char_info.dead_count > 0)
        {
            s_char_info.dead_count--;
            Player[numPlayers].Dead = true;

            // initialize ghost logic for player
            int living = CheckLiving();
            if(living)
            {
                Player[numPlayers].Effect2 = -living;
                Player[numPlayers].Location.X = Player[living].Location.X;
                Player[numPlayers].Location.Y = Player[living].Location.Y;
                Player[numPlayers].Section    = Player[living].Section;
            }
            else
                Player[numPlayers].Effect2 = 0;

            PlaySound(SFX_ShellHit);
        }
        else
        {
            PlaySound(SFX_DropItem);
        }
    }
}

void PlayerBox::ValidateChar(bool ghost_mode)
{
    int p = CalcIndex();

    uint8_t& sel = l_screen->charSelect[p];

    // ensure that character selection is still valid

    // bounds check
    if(sel < 1 || sel > numCharacters)
        sel = 1;

    // i is a dummy variable so that if invalid, end up where we started
    for(int i = 0; i < numCharacters; i++)
    {
        if(CharAvailable(sel, ghost_mode))
            return;

        sel ++;
        if(sel > numCharacters)
            sel = 1;
    }

    // if we failed (total block), first try to find an unblocked char
    for(int i = 1; i <= numCharacters; i++)
    {
        if(s_char_info.accept(i, p, m_current_add))
        {
            sel = i;
            return;
        }
    }

    // failed again? just do char 1
    sel = 1;
}

struct PlayDoSentinel
{
    bool active = true;
    ~PlayDoSentinel()
    {
        if(active)
            PlaySoundMenu(SFX_Do);
    }
};

// returns true to signal exit from menu
bool PlayerBox::Back()
{
    // int p = CalcIndex();

    m_input_ready = false;

    // first screen of main menu
    if(m_state == PlayerState::Disconnected && IsMenu())
    {
        PlaySoundMenu(SFX_Slide);
        return true;
    }
    // primary menu areas
    else if(m_state == PlayerState::SelectChar || m_state == PlayerState::ControlsMenu)
    {
        // adding player at main menu
        if(IsMenu())
        {
            // just go back!
            PlaySoundMenu(SFX_Slide);
            return true;
        }
        // escape the drop/add menu
        else
        {
            if(CheckDone())
            {
                PlaySoundMenu(SFX_Slide);
                return true;
            }
            else if(CheckCanDrop() >= -1)
            {
                m_state = PlayerState::ForceDrop;
                m_menu_item = 0;
                m_input_ready = true;
                m_marquee_state.reset_width();
                return false;
            }
            else
            {
                // m_back_requested = true;
                PlaySoundMenu(SFX_BlockHit);
                return false;
            }
        }
    }

    PlaySoundMenu(SFX_Slide);

    if(Is1P() && m_state == PlayerState::SelectProfile)
    {
        m_state = PlayerState::SelectChar;
        m_menu_item = 0;
    }
    else if(m_state == PlayerState::SelectProfile)
    {
        m_state = PlayerState::ControlsMenu;
        m_menu_item = 1;
        m_marquee_state.reset_width();
    }

    return false;
}

bool PlayerBox::Do()
{
    int p = CalcIndex();

    m_input_ready = false;

    // plays SFX_Do at end of scope unless inactivated
    PlayDoSentinel do_sentinel;

    // controls menu
    if(m_state == PlayerState::ControlsMenu && p < (int)Controls::g_InputMethods.size())
    {
        // drop me
        if(ControlsMenu_ShowDrop(p) && m_menu_item == -1)
        {
            do_sentinel.active = false;

            Player_Remove(p);

            // quit if there aren't any other players
            if(s_context == Context::MainMenu && Controls::g_InputMethods.empty())
            {
                PlaySoundMenu(SFX_Slide);
                return true;
            }

            PlaySoundMenu(SFX_PlayerDied2);
            return false;
        }
        // reconnect controller
        else if(m_menu_item == 0)
        {
            // does set Controls::g_InputMethods[p] to null
            Controls::DeleteInputMethod(Controls::g_InputMethods[p]);

            m_state = PlayerState::Reconnecting;
            m_marquee_state.reset_width();

            do_sentinel.active = false;

            PlaySoundMenu(SFX_PetHurt);

            return false;
        }
        // select a profile
        else if(m_menu_item == 1)
        {
            // only enter the profile select screen if safe to do so.
            if(Controls::g_InputMethods[p])
            {
                m_state = PlayerState::SelectProfile;
                m_marquee_state.reset_width();

                // set the current menu item to the current profile.
                std::vector<Controls::InputMethodProfile*> profiles = Controls::g_InputMethods[p]->Type->GetProfiles();
                std::vector<Controls::InputMethodProfile*>::iterator p_profile
                    = std::find(profiles.begin(), profiles.end(), Controls::g_InputMethods[p]->Profile);

                if(profiles.size() > 1 && p_profile != profiles.end())
                    m_menu_item = p_profile - profiles.begin();
                else
                    m_menu_item = 0;
            }
        }
        return false;
    }

    // select char
    if(m_state == PlayerState::SelectChar)
    {
        do_sentinel.active = false;

        if(IsMenu())
        {
            if(CheckDone())
            {
                // clean up charSelect fields
                for(int p = (int)Controls::g_InputMethods.size(); p < maxLocalPlayers; p++)
                    l_screen->charSelect[p] = 0;

                do_sentinel.active = true;
                return true;
            }
            else
                PlaySoundMenu(SFX_BlockHit);
        }
    }
    // select profile
    else if(m_state == PlayerState::SelectProfile)
    {
        m_marquee_state.reset_width();

        if(p < (int)Controls::g_InputMethods.size() && Controls::g_InputMethods[p])
        {
            const std::vector<Controls::InputMethodProfile*> profiles = Controls::g_InputMethods[p]->Type->GetProfiles();
            // store profile
            m_last_profile = Controls::g_InputMethods[p]->Profile;

            int back_index = (int)profiles.size();
            // if not possible, just go back
            if(m_menu_item < 0 || m_menu_item >= back_index)
            {
                Back();
                do_sentinel.active = false;
            }
            // don't do confirmation screen if just setting current profile
            else if(m_last_profile == profiles[m_menu_item])
            {
                Back();
                do_sentinel.active = false;
            }
            // advance to the confirmation screen if successful
            else if(Controls::SetInputMethodProfile(p, profiles[m_menu_item]))
            {
                m_state = PlayerState::ConfirmProfile;
                m_menu_item = 66 * 3;
            }
            // indicate that profile couldn't be bound
            else
            {
                PlaySoundMenu(SFX_BlockHit);
                do_sentinel.active = false;
            }
        }
    }

    return false;
}

bool PlayerBox::CanChangeChar()
{
    // if(!SwapCharAllowed() && !m_current_add)
    //     return false;

    int p = CalcIndex();

    // see if any other character is totally free for self
    for(int ch = 1; ch <= numCharacters; ch++)
    {
        if(ch == l_screen->charSelect[p])
            continue;

        if(CharAvailable(ch))
            return true;
    }

    // if own character is okay, then it'll be kept. return false here.
    if(CharAvailable(l_screen->charSelect[p]))
        return false;

    // if even own character not totally free, see if any character is allowed by the s_char_info state
    for(int ch = 1; ch <= numCharacters; ch++)
    {
        if(ch == l_screen->charSelect[p])
            continue;

        if(s_char_info.accept(l_screen->charSelect[p], p, m_current_add))
            return true;
    }

    return false;
}

void PlayerBox::PrevChar()
{
    // if(!SwapCharAllowed() && !m_current_add)
    // {
    //     PlaySoundMenu(SFX_BlockHit);
    //     return;
    // }

    int p = CalcIndex();
    int old_ch = l_screen->charSelect[p];

    int i = 0;
    for(i = 0; i < numCharacters; i++)
    {
        l_screen->charSelect[p] --;

        if(l_screen->charSelect[p] < 1)
            l_screen->charSelect[p] = numCharacters;

        if(CharAvailable(l_screen->charSelect[p]))
            break;
    }

    // if can't traverse normally, allow duplicates
    if(i == numCharacters)
    {
        for(i = 0; i < numCharacters; i++)
        {
            l_screen->charSelect[p] --;

            if(l_screen->charSelect[p] < 1)
                l_screen->charSelect[p] = numCharacters;

            if(s_char_info.accept(l_screen->charSelect[p], p, m_current_add))
                break;
        }
    }

    // eventually changed character, update the player
    if(l_screen->charSelect[p] != old_ch)
    {
        PlaySoundMenu(SFX_Slide);
        UpdatePlayer();
    }
    else
        PlaySoundMenu(SFX_BlockHit);
}

void PlayerBox::NextChar()
{
    // if(!SwapCharAllowed() && !m_current_add)
    // {
    //     PlaySoundMenu(SFX_BlockHit);
    //     return;
    // }

    int p = CalcIndex();
    int old_ch = l_screen->charSelect[p];

    int i;
    for(i = 0; i < numCharacters; i++)
    {
        l_screen->charSelect[p] ++;

        if(l_screen->charSelect[p] > numCharacters)
            l_screen->charSelect[p] = 1;

        if(CharAvailable(l_screen->charSelect[p]))
            break;
    }

    // if can't traverse normally, allow duplicates
    if(i == numCharacters)
    {
        for(i = 0; i < numCharacters; i++)
        {
            l_screen->charSelect[p] ++;

            if(l_screen->charSelect[p] > numCharacters)
                l_screen->charSelect[p] = 1;

            if(s_char_info.accept(l_screen->charSelect[p], p, m_current_add))
                break;
        }
    }

    // eventually changed character, update the player
    if(l_screen->charSelect[p] != old_ch)
    {
        PlaySoundMenu(SFX_Slide);
        UpdatePlayer();
    }
    else
        PlaySoundMenu(SFX_BlockHit);
}

void PlayerBox::Up()
{
    int p = CalcIndex();

    m_input_ready = false;

    if(m_state == PlayerState::SelectProfile)
    {
        PlaySoundMenu(SFX_Slide);

        if(m_menu_item == 0)
        {
            if(p < (int)Controls::g_InputMethods.size() && Controls::g_InputMethods[p])
            {
                std::vector<Controls::InputMethodProfile*> profiles = Controls::g_InputMethods[p]->Type->GetProfiles();
                int back_index = (int)profiles.size();
                m_menu_item = back_index;
            }
        }
        else
            m_menu_item --;

        m_marquee_state.reset_width();
    }

    if(m_state == PlayerState::ControlsMenu)
    {
        PlaySoundMenu(SFX_Slide);
        m_state = PlayerState::SelectChar;
        m_marquee_state.reset_width();
    }

    // rotate char backwards in 1P menu
    if(m_state == PlayerState::SelectChar && Is1P())
        PrevChar();
}

void PlayerBox::Down()
{
    int p = CalcIndex();

    m_input_ready = false;

    if(m_state == PlayerState::SelectProfile)
    {
        PlaySoundMenu(SFX_Slide);

        if(p < (int)Controls::g_InputMethods.size() && Controls::g_InputMethods[p])
        {
            std::vector<Controls::InputMethodProfile*> profiles = Controls::g_InputMethods[p]->Type->GetProfiles();
            int back_index = (int)profiles.size();
            if(m_menu_item == back_index)
                m_menu_item = -1;
        }

        m_menu_item ++;

        m_marquee_state.reset_width();
    }

    // go to controls menu in non-1P
    if(m_state == PlayerState::SelectChar && !Is1P())
    {
        PlaySoundMenu(SFX_Slide);
        m_state = PlayerState::ControlsMenu;

        if(ControlsMenu_ShowDrop(p))
            m_menu_item = -1;
        else
            m_menu_item = 0;

        m_marquee_state.reset_width();
    }

    // rotate char forwards in 1P menu
    if(m_state == PlayerState::SelectChar && Is1P())
        NextChar();
}

void PlayerBox::Left()
{
    m_input_ready = false;

    // rotate char backwards in non-1P
    if(m_state == PlayerState::SelectChar && !Is1P())
        PrevChar();

    if(m_state == PlayerState::ControlsMenu)
    {
        int p = CalcIndex();

        // don't auto-rotate for clarity
        if(m_menu_item > 0 || (ControlsMenu_ShowDrop(p) && m_menu_item == 0))
        {
            PlaySoundMenu(SFX_Slide);
            m_menu_item --;
            m_marquee_state.reset_width();
        }
    }
}

void PlayerBox::Right()
{
    m_input_ready = false;

    // rotate char forwards in non-1P
    if(m_state == PlayerState::SelectChar && !Is1P())
        NextChar();

    if(m_state == PlayerState::ControlsMenu)
    {
        if(m_menu_item < 1)
        {
            PlaySoundMenu(SFX_Slide);
            m_menu_item ++;
            m_marquee_state.reset_width();
        }
    }
}

bool PlayerBox::MouseItem(int i)
{
    if(!Is1P() && m_state == PlayerState::SelectChar)
    {
        m_state = PlayerState::ControlsMenu;
        PlaySoundMenu(SFX_Slide);
    }

    int sel = (m_state == PlayerState::SelectChar) ? l_screen->charSelect[0] : m_menu_item;

    if(sel != i)
    {
        PlaySoundMenu(SFX_Slide);

        if(m_state == PlayerState::SelectChar)
            l_screen->charSelect[0] = i;
        else
            m_menu_item = i;

        m_marquee_state.reset_width();
    }

    if(MenuMouseRelease && SharedCursor.Primary)
    {
        MenuMouseRelease = false;
        return Do();
    }

    return false;
}

bool PlayerBox::MenuItem_Mouse_Render(int i, const std::string& label, int X, int Y, bool mouse, bool render, int end_X)
{
    if(mouse)
    {
        int menuLen = (int)label.size() * 18;
        if(SharedCursor.X >= X && SharedCursor.X <= X + menuLen
            && SharedCursor.Y >= Y && SharedCursor.Y <= Y + 16)
        {
            return MouseItem(i);
        }
    }

    if(render)
    {
        if(m_state != PlayerState::SelectProfile)
        {
            if(GFX.CharSelIcons.inited && label.size() == 1)
            {
                if(label[0] == 'X')
                {
                    XRender::renderTexture(X, Y, 24, 24, GFX.CharSelIcons, 0, 0);
                    return false;
                }
                else if(label[0] == 'O')
                {
                    XRender::renderTexture(X, Y, 24, 24, GFX.CharSelIcons, 24, 0);
                    return false;
                }
                else if(label[0] == '?')
                {
                    XRender::renderTexture(X, Y, 24, 24, GFX.CharSelIcons, 48, 0);
                    return false;
                }
            }
            else if(label.size() == 1)
                X += 4;
        }

        if(end_X != 0)
        {
            bool cur_item = (m_menu_item == i);

            MarqueeState null_marquee;
            MarqueeSpec item_spec(end_X - X, 10, 32, 32, -1);

            if(m_menu_item == i)
                m_marquee_state.advance(item_spec);

            SuperPrintMarquee(label, 5, X, Y, item_spec, (cur_item) ? m_marquee_state : null_marquee);
        }
        else
            SuperPrint(label, 3, X, Y);
    }

    return false;
}

// render the selected character for player P
bool PlayerBox::DrawChar(int x, int w, int y, int h, bool show_name)
{
    // don't render character box in certain states
    if(m_state == PlayerState::SelectProfile || m_state == PlayerState::ConfirmProfile)
        return false;

    int p = CalcIndex();

    // player hasn't been added yet
    bool inactive = m_state == PlayerState::Disconnected && (IsMenu() || p >= l_screen->player_count);
    bool show_inactive = inactive && p >= s_minPlayers;

    // alpha for most uses
    uint8_t alpha = (show_inactive) ? 127 : 255;

    // verify that character is valid
    int ch = l_screen->charSelect[p];
    if(ch < 1 || ch > numCharacters)
        return false;

    // do the fun player transformation thing!
    if(s_context == Context::MainMenu)
        DoTransform(p, ch);

    // draw frame
    if(GFX.CharSelFrame.tex.inited)
        RenderFrameFill(newLoc(x, y, w, h), GFX.CharSelFrame, XTAlpha(alpha));
    else
    {
        uint8_t alpha_border = (show_inactive) ?  64 : 255;

        XRender::renderRect(x, y, w, h, {0, 0, 0, alpha_border});
        XRender::renderRect(x + 2, y + 2, w - 4, h - 4, {255, 255, 255, alpha});
    }

    // draw arrows
    if(m_state == PlayerState::SelectChar || (s_context == Context::MainMenu && inactive))
    {
        XTColor arrow_color;

        // show grayed out scroll indicator if can't change char
        if(!CanChangeChar())
            arrow_color.a = 127;
        // otherwise, hide half of the time
        else if(CommonFrame % 90 >= 45)
            arrow_color.a = 0;
        // never show full opacity when inactive
        else if(show_inactive)
            arrow_color.a = 127;

        if(arrow_color.a == 0)
        {
            // don't render
        }
        else if(GFX.CharSelIcons.inited)
        {
            XRender::renderTextureFL(x - 24 - 4, y + h / 2 - 24 / 2, 24, 24, GFX.CharSelIcons, 72, 0, 0.0, nullptr, X_FLIP_HORIZONTAL, arrow_color);
            XRender::renderTexture(x + w + 4, y + h / 2 - 24 / 2, 24, 24, GFX.CharSelIcons, 72, 0, arrow_color);
        }
        else
        {
            XRender::renderTextureFL(x - GFX.MCursor[1].h - 4, y + h / 2 - GFX.MCursor[1].w / 2, GFX.MCursor[1].w, GFX.MCursor[1].h, GFX.MCursor[1], 0, 0, -90.0, nullptr, X_FLIP_NONE, arrow_color);
            XRender::renderTextureFL(x + w + 4, y + h / 2 - GFX.MCursor[2].w / 2, GFX.MCursor[2].w, GFX.MCursor[2].h, GFX.MCursor[2], 0, 0, -90.0, nullptr, X_FLIP_NONE, arrow_color);
        }
    }

    // flash if player is currently disconnected
    bool player_flash = (!inactive && m_state == PlayerState::Disconnected);

    if(!player_flash || (CommonFrame % 60) < 30)
    {
        // always walk in Drop / Add, walk only after finalizing selection at main menu
        bool player_walk = (s_context == Context::DropAdd || !inactive);

        // find a spare player to edit
        int scratch_index = maxPlayers - maxLocalPlayers + 1 + p;
        Player_t& scratch_player = Player[scratch_index];

        // back up some info to the stack
        int PrevCh = scratch_player.Character;
        int Frame = scratch_player.Frame;
        int FrameCount = scratch_player.FrameCount;
        int YoshiTFrameCount = scratch_player.YoshiTFrameCount;
        int YoshiBFrameCount = scratch_player.YoshiBFrameCount;
        int YoshiWingsFrameCount = scratch_player.YoshiWingsFrameCount;

        // initialize player based on saved data (unless in Battle Mode)
        if(s_context == Context::MainMenu && MenuMode == MENU_CHARACTER_SELECT_NEW_BM)
        {
            scratch_player = Player_t();
            scratch_player.Character = ch;
            scratch_player.State = 2;
            scratch_player.Hearts = 2;
        }
        // empty save
        else if(s_context == Context::MainMenu && SaveSlotInfo[selSave].Progress < 0)
        {
            scratch_player = Player_t();
            scratch_player.Character = ch;
            scratch_player.State = 1;
            scratch_player.Hearts = 1;
        }
        else if(s_context == Context::MainMenu)
            scratch_player = SaveSlotInfo[selSave].SavedChar[ch];
        else if(BattleMode)
        {
            scratch_player = Player_t();
            scratch_player.Character = ch;
            scratch_player.State = 2;
            scratch_player.Hearts = 2;
        }
        else
            scratch_player = SavedChar[ch];

        if(scratch_player.Character < 1 || scratch_player.Character > numCharacters)
            scratch_player.Character = ch;

        if(scratch_player.State < 1 || scratch_player.State > numStates)
            scratch_player.State = 1;

        // restore data
        if(scratch_player.Character == PrevCh)
        {
            scratch_player.Frame = Frame;
            scratch_player.FrameCount = FrameCount;
            scratch_player.YoshiTFrameCount = YoshiTFrameCount;
            scratch_player.YoshiBFrameCount = YoshiBFrameCount;
            scratch_player.YoshiWingsFrameCount = YoshiWingsFrameCount;
        }
        else
            scratch_player.Frame = 1;

        // process height and frames
        scratch_player.Location.Height = Physics.PlayerHeight[scratch_player.Character][scratch_player.State];
        YoshiHeight(scratch_index);

        scratch_player.Direction = 1;
        scratch_player.Location.SpeedY = 0;
        if(player_walk)
            scratch_player.Location.SpeedX = 1;

        PlayerFrame(scratch_player);

        // set up location and place player
        scratch_player.Location.Width = Physics.PlayerWidth[scratch_player.Character][scratch_player.State];
        scratch_player.Location.Height = Physics.PlayerHeight[scratch_player.Character][scratch_player.State];
        SizeCheck(scratch_player);
        scratch_player.Location.X = x + w / 2 - scratch_player.Location.Width / 2 - vScreen[0].X;
        scratch_player.Location.Y = y + h / 2 + Physics.PlayerHeight[5][2] / 4 - scratch_player.Location.Height * 3 / 4 + 4 - vScreen[0].Y;

        if(scratch_player.Mount == 3)
            scratch_player.Location.Y += 6;

        // animate winged boot
        if(scratch_player.Mount == 1 && scratch_player.MountType == 3)
        {
            scratch_player.YoshiWingsFrameCount += 1;
            scratch_player.YoshiWingsFrame = 0;

            if(scratch_player.YoshiWingsFrameCount <= 12)
                scratch_player.YoshiWingsFrame = 1;
            else if(scratch_player.YoshiWingsFrameCount >= 24)
                scratch_player.YoshiWingsFrameCount = 0;

            if(scratch_player.Direction == 1)
                scratch_player.YoshiWingsFrame += 2;
        }

        DrawPlayer(scratch_player, 0, XTAlpha(alpha));
    }

#if 0
    // finish with character name
    if(show_name)
        SuperPrintCenter(g_gameInfo.characterName[l_screen->charSelect[p]], 3, x + w / 2, y + h + 12, XTAlpha(alpha));
#else
    UNUSED(show_name);
#endif

    return true;
}

// special the mouse logic and rendering 1P char select
int PlayerBox::Mouse_Render_1P(bool render)
{
    int p = CalcIndex();
    bool mouse = !render;

    if(p < 0 || p >= maxLocalPlayers)
    {
        // `p` should not be bigger than `maxLocalPlaeyrs`, otherwise the out of range at `s_playerState`
        SDL_assert(p >=0 && p < maxLocalPlayers);
        return false;
    }

    if(mouse && !render && !SharedCursor.Move && !SharedCursor.Primary && !SharedCursor.Secondary && !SharedCursor.ScrollUp && !SharedCursor.ScrollDown)
        return 0;

    if(mouse && SharedCursor.Secondary && MenuMouseRelease)
    {
        MenuMouseRelease = false;

        if(Back())
            return -1;
    }

    int MenuX, MenuY;
    GetMenuPos(&MenuX, &MenuY);

    int y_pos = MenuY;

    for(int i = 1; i <= numCharacters; i++)
    {
        if(!blockCharacter[i])
        {
            if(MenuItem_Mouse_Render(i, fmt::format_ne(g_mainMenu.selectCharacter, g_gameInfo.characterName[i]), MenuX, y_pos, mouse, render))
                return 1;

            if(render && l_screen->charSelect[p] == i)
                XRender::renderTexture(MenuX - 20, y_pos, GFX.MCursor[0]);

            y_pos += 30;
        }
    }

    // show controls info
    if(render)
    {
        int infobox_y = MenuY + 145;

        XRender::renderRect(XRender::TargetW / 2 - 240, infobox_y, 480, 68, XTColorF(0, 0, 0, 0.5));

        // disconnection hint
        if(p >= (int)Controls::g_InputMethods.size() || !Controls::g_InputMethods[p])
        {
            SuperPrintScreenCenter(g_gameStrings.connectWaitingForInputDevice, 3, infobox_y + 4, XTColorF(0.8f, 0.8f, 0.8f, 0.8f));
        }
        else
        {
            // global information about controller
            // Profile should never be null
            if(Controls::g_InputMethods[p]->Profile != nullptr)
                SuperPrintScreenCenter(Controls::g_InputMethods[p]->Name + " - " + Controls::g_InputMethods[p]->Profile->Name, 3, infobox_y + 4);
            else
                SuperPrintScreenCenter(Controls::g_InputMethods[p]->Name, 3, infobox_y + 4);
        }

        SuperPrintScreenCenter(g_gameStrings.connectPressSelectForControlsOptions_P1, 3, infobox_y + 24, XTColorF(0.8f, 0.8f, 0.8f, 0.8f));
        SuperPrintScreenCenter(g_gameStrings.connectPressSelectForControlsOptions_P2, 3, infobox_y + 44, XTColorF(0.8f, 0.8f, 0.8f, 0.8f));
    }

    // do char transform thing!
    if(render && l_screen->charSelect[p] >= 1 && l_screen->charSelect[p] <= numCharacters)
        DoTransform(p, l_screen->charSelect[p]);

    return 0;
}

// do the mouse logic and rendering for each player
int PlayerBox::Mouse_Render(bool render, int x, int y, int w, int h)
{
    if(Is1P() && (m_state == PlayerState::SelectChar))
        return Mouse_Render_1P(render);

    int p = CalcIndex();
    bool mouse = !render;
    const auto* input_method = (p < (int)Controls::g_InputMethods.size()) ? Controls::g_InputMethods[p] : nullptr;


    // calculate positions
    int cx = x + w / 2;

    // controls / info pane
    const int info_height = 80;
    int info_y = y + h - info_height;

    // icons column
    const int icons_height = 22;
    int icons_x = x + 8 + 76 + 8;
    int icons_y = info_y + (GFX.PCursor.inited ? GFX.PCursor.w : GFX.MCursor[2].h) + 8;
    int infotext_y = icons_y + icons_height + 8;

    // main char select pane
    int main_x = x + 4;
    int main_y = y + 6;
    int main_width = w - 8;
    int main_height = h - info_height - 6;
    int total_height = h - 12;

    // not-yet-added player
    bool inactive = m_state == PlayerState::Disconnected && (s_context == Context::MainMenu || p >= l_screen->player_count);
    bool show_inactive = inactive && p >= s_minPlayers;

    if(inactive)
    {
        if(show_inactive)
            l_screen->charSelect[p] = s_recent_char[p];
        ValidateChar(true);
    }

    // check currently selected character
    int ch = l_screen->charSelect[p];

    // verify that character is valid
    if(ch < 1 || ch > numCharacters)
        ch = 0;

    // calculate box color
    XTColor color = XTColorF(0.4f, 0.4f, 0.4f);

    switch(ch)
    {
    case 1:
        color = XTColorF(1.0f, 0.0f, 0.0f);
        break;
    case 2:
        color = XTColorF(0.0f, 1.0f, 0.0f);
        break;
    case 3:
        color = XTColorF(1.0f, 0.4f, 0.5f);
        break;
    case 4:
        color = XTColorF(0.0f, 0.4f, 1.0f);
        break;
    case 5:
        color = {255, 204, 0};
        break;
    }

    if(show_inactive)
    {
        color.r = color.r / 4 + 127;
        color.g = color.g / 4 + 127;
        color.b = color.b / 4 + 127;
    }

    // render background
    if(render)
    {
        // background
        XRender::renderRect(x + 0, y + 0, w - 0, h - 0, {0, 0, 0});
        XRender::renderRect(x + 2, y + 2, w - 4, h - 4, color * 0.95f);
        XRender::renderRect(x + 4, y + 4, w - 8, h - 8, color * 0.75f);

        if(m_state != PlayerState::SelectProfile)
        {
            // middle dividing line (bg)
            XRender::renderRect(x + 2, info_y - 2, w - 4, 6, color * 0.95f);

            // middle dividing line (fg)
            XRender::renderRect(x, info_y, w, 2, {0, 0, 0});
        }

        // confirmation progress
        if(m_state == PlayerState::ConfirmProfile || m_state == PlayerState::ForceDrop)
        {
            int menu_progress = m_menu_item;
            int menu_max = 66 * 2;

            // special asymmetric setup for confirming controls profile
            if(m_state == PlayerState::ConfirmProfile)
            {
                menu_progress = SDL_min(m_menu_item, 66 * 3) * 2 + SDL_max(m_menu_item - 66 * 3, 0) * 3;
                menu_max = 66 * 12;
            }

            XTColor shade = color * 0.95f;
            XRender::renderRect(x + 4, info_y + 4, (w - 8) * menu_progress / menu_max, info_height - 8, shade);

            // move infotext y up a bit, since controls will be drawn in main box (if at all)
            infotext_y = info_y + info_height / 2 - 8;
        }
    }

    // render the player box (if appropriate)
    // bool show_name = (main_height > 88 + 16 + 32);
    constexpr bool show_name = false;
    const int label_h = show_name ? 32 : 16;
    const int pbox_h = SDL_max(64, SDL_min(88, main_height - 16 - label_h));
    const int pbox_w = pbox_h;
    const int pbox_h_full = pbox_h + 16 + label_h;
    int pbox_x = main_x + main_width / 2 - pbox_w / 2;
    int pbox_y = main_y + main_height / 2 - pbox_h_full / 2 + 16;

    // draw character box
    if(render)
        DrawChar(pbox_x, pbox_w, pbox_y, pbox_h, show_name);

    // allow mouse to select char
    if(mouse && (m_state == PlayerState::ControlsMenu || m_state == PlayerState::SelectChar))
    {
        if(SharedCursor.X >= pbox_x && SharedCursor.X <= pbox_x + pbox_w
            && SharedCursor.Y >= pbox_y && SharedCursor.Y <= pbox_y + pbox_h)
        {
            if(m_state != PlayerState::SelectChar)
            {
                m_state = PlayerState::SelectChar;
                PlaySoundMenu(SFX_Slide);
            }

            if(m_input_ready && SharedCursor.ScrollUp)
                Left();

            if(m_input_ready && SharedCursor.ScrollDown)
                Right();

            if(MenuMouseRelease && SharedCursor.Primary)
            {
                MenuMouseRelease = false;
                return Do();
            }
        }
    }

    // draw cursor above char box
    if(render)
    {
        if(m_state == PlayerState::SelectChar || (inactive && p < s_minPlayers))
            Render_PCursorDownwards(pbox_x + pbox_w / 2, pbox_y - 2);
        else if(inactive && s_context == Context::MainMenu)
            Render_PCursorDownwards(pbox_x + pbox_w / 2, pbox_y - 2, XTAlpha(127));
    }

    // now render / process the player's menu as appropriate to its case

    // render the profile selection menu
    if(m_state == PlayerState::SelectProfile)
    {
        if(!input_method)
        {
            Back();
            return 0;
        }

        // set the current menu item to the current profile.
        std::vector<Controls::InputMethodProfile*> profiles = input_method->Type->GetProfiles();

        if(profiles.empty())
        {
            Back();
            return 0;
        }

        if(render)
            SuperPrintCenter(g_mainMenu.wordProfiles, 3, cx, main_y);

        int line = 20;

        int start_y = main_y + 16 + 1 * line;
        int end_y = main_y + total_height - 8;

        // calculate scroll (top rendered index) here
        int total_lines = (int)profiles.size() + 1;
        int avail_lines = (end_y - start_y) / line;

        int scroll_start = 0;
        int scroll_end = total_lines;

        if(avail_lines < total_lines)
        {
            if(m_menu_item >= 0)
                scroll_start = m_menu_item - avail_lines/2;
            if(scroll_start < 0)
                scroll_start = 0;

            scroll_end = scroll_start + avail_lines;
            if(scroll_end > total_lines)
            {
                scroll_end = total_lines;
                scroll_start = scroll_end - avail_lines;
            }
        }

        // render the scroll indicators
        if(render)
        {
            if(scroll_start > 0)
                XRender::renderTexture(cx - GFX.MCursor[1].w / 2, start_y - GFX.MCursor[1].h, GFX.MCursor[1]);

            if(scroll_end < total_lines)
                XRender::renderTexture(cx - GFX.MCursor[2].w / 2, start_y + (avail_lines)*line - line + 18, GFX.MCursor[2]);
        }

        // show the menu cursor for the player
        if(render && m_menu_item >= 0)
            Render_PCursor(x + 6, start_y + (m_menu_item-scroll_start)*line);

        // render the profile names and "Back"
        for(int i = scroll_start; i < scroll_end; i++)
        {
            std::string* name = &g_mainMenu.wordBack;

            if(i != (int)profiles.size())
                name = &(profiles[i]->Name);

            MenuItem_Mouse_Render(i, *name,
                x + 6 + 16 + 2, start_y + (i-scroll_start)*line, mouse, render, x + w - 6);
        }
    }

    // render the profile confirmation screen
    if(render && m_state == PlayerState::ConfirmProfile)
    {
        SuperPrintCenter(g_gameStrings.connectTestProfile, 3, cx, main_y);

        if(input_method && input_method->Profile != nullptr)
        {
            MarqueeSpec profile_spec(w - 12, 10, 64, 32, 0);
            m_marquee_state.advance(profile_spec);
            SuperPrintMarquee(input_method->Profile->Name, 5, x + 6, main_y + 20, profile_spec, m_marquee_state);
        }
    }

    // draw the controls box in the appropriate place
    if(render)
    {
        int controls_x = 0;
        int controls_y = 0;

        if(m_state == PlayerState::SelectProfile || m_state == PlayerState::ForceDrop)
        {
            // don't draw at all
        }
        else if(m_state == PlayerState::ConfirmProfile)
        {
            controls_x = main_x + main_width / 2 - 76 / 2;
            controls_y = main_y + main_height / 2 - 16;
        }
        else if(m_state == PlayerState::Disconnected || m_state == PlayerState::Reconnecting)
        {
            controls_x = main_x + main_width / 2 - 76 / 2;
            controls_y = icons_y - 6;
        }
        else
        {
            controls_x = icons_x - 84;
            controls_y = icons_y - 6;
        }

        if(controls_x != 0)
        {
            if(inactive)
                speedRun_syncControlKeys(p, Controls_t());

            RenderControls(p, controls_x, controls_y, 76, 30, !input_method, 255, true);
        }
    }

    // render the controls menu main icons
    if(m_state == PlayerState::SelectChar || m_state == PlayerState::ControlsMenu)
    {
        int col_spacer = (x + w - 4 - icons_x) / 3;
        int base_col = icons_x + col_spacer;

        // show the cursor for the player
        if(render && m_state == PlayerState::ControlsMenu)
            Render_PCursorDownwards(base_col + 12 + m_menu_item * col_spacer, icons_y);

        // drop me
        if(ControlsMenu_ShowDrop(p))
        {
            // this one returns true when no players are left
            if(MenuItem_Mouse_Render(-1, "X",
                base_col - col_spacer, icons_y, mouse, render))
            {
                return -1;
            }
        }

        // reconnect controller
        MenuItem_Mouse_Render(0, "O",
            base_col, icons_y, mouse, render);

        // switch profile
        MenuItem_Mouse_Render(1, "?",
            base_col + col_spacer, icons_y, mouse, render);
    }

    // hint string
    if(render)
    {
        const std::string* message = nullptr;
        std::string temporary;
        XTColor message_color;

        if(m_state == PlayerState::SelectChar)
        {
            if(input_method && input_method->Profile != nullptr)
            {
                message = &input_method->Profile->Name;
                message_color = XTAlphaF(0.75);
            }
        }
        else if(m_state == PlayerState::ControlsMenu)
        {
            if(m_menu_item == -1)
                message = &g_gameStrings.connectDropMe;
            else if(m_menu_item == 0)
                message = &g_gameStrings.connectReconnectTitle;
            else if(m_menu_item == 1)
                message = &g_mainMenu.wordProfiles;
        }
        else if(m_state == PlayerState::ConfirmProfile)
        {
            // using marquee for profile name
            SuperPrintCenter(g_gameStrings.connectHoldStart, 5, cx, infotext_y, message_color);
        }
        else if(m_state == PlayerState::ForceDrop)
        {
            int index = CheckCanDrop() + 1;

            if(index > 0)
                message = &(temporary = fmt::format_ne(g_gameStrings.connectDropPX, index));
            else
                message = &g_gameStrings.connectForceResume;
        }
        else if(m_state == PlayerState::Reconnecting)
            message = &g_gameStrings.connectReconnectTitle;
        else if(m_state == PlayerState::Disconnected)
            message = &g_gameStrings.connectPressAButton;

        if(message)
        {
            MarqueeSpec print_spec = MarqueeSpec(main_width - 8, 10, 16, 32, 0);

            m_marquee_state.advance(print_spec);
            SuperPrintMarquee(*message, 5, main_x + 4, infotext_y, print_spec, m_marquee_state, message_color);
        }
    }

    if(MenuMouseRelease && SharedCursor.Secondary
        && SharedCursor.X >= x && SharedCursor.X <= x + w
        && SharedCursor.Y >= y && SharedCursor.Y <= y + h)
    {
        MenuMouseRelease = false;
        if(Back())
            return -1;
    }

    return 0;
}

int Mouse_Render(bool mouse, bool render)
{
    int MenuX, MenuY;
    GetMenuPos(&MenuX, &MenuY);

    if(mouse && !SharedCursor.Move && !render && !SharedCursor.Primary && !SharedCursor.Secondary && !SharedCursor.ScrollUp && !SharedCursor.ScrollDown)
        return 0;

    int n = BoxCount();

    /*--------------------*\
    || Arrange screen     ||
    \*--------------------*/

    const int header_height = (s_context == Context::DropAdd) ? 32 : 0;
    int box_width = 220;
    int box_height = 240;
    if(IsMenu() && box_height > XRender::TargetH - MenuY - 8)
        box_height = XRender::TargetH - MenuY - 8;
    int padding = 16;

    // number of columns per row
    int n_cols[2] = {n, 0};

    // do drop/add in 2 rows when >2P
    if(n > 2 && s_context != Context::MainMenu)
    {
        n_cols[1] = n / 2;
        n_cols[0] = n - n_cols[1];
    }

    // shrink boxes when they would not fit on screen
    if(n_cols[0] * box_width + (n_cols[0] - 1) * padding > XRender::TargetW)
    {
        box_width = 168;
        padding = 8;

        int max_width = (XRender::TargetW - (n_cols[0] - 1) * padding) / n_cols[0];
        max_width &= ~1;
        if(box_width > max_width)
            box_width = max_width;
    }

    int full_height = header_height + box_height + padding;
    int full_width = padding + (box_width + padding) * n_cols[0];

    if(n_cols[1])
        full_height += padding + box_height;

    // vertical / horizontal start of the menu
    int start_x = XRender::TargetW / 2 - full_width / 2;
    int draw_y = XRender::TargetH / 2 - full_height / 2;

    if(IsMenu())
        GetMenuPos(nullptr, &draw_y);

    // make sure they are even
    start_x &= ~1;
    draw_y &= ~1;

    /*-----------------------*\
    || Background and header ||
    \*-----------------------*/

    if(s_context == Context::DropAdd)
    {
        if(render)
        {
            if(full_width < 440)
                XRender::renderRect((XRender::TargetW / 2 - 440 / 2) & ~1, draw_y, 440, full_height, XTColorF(0.0f, 0.0f, 0.0f, 0.5f));
            else
                XRender::renderRect(start_x, draw_y, full_width, full_height, XTColorF(0.0f, 0.0f, 0.0f, 0.5f));

            SuperPrintScreenCenter(g_gameStrings.pauseItemPlayerSetup, 3, draw_y + 8);
        }

        draw_y += header_height;
    }

    int ret = 0;

    // start drawing player boxes
    int draw_x = start_x + padding;

    for(int p = 0; p < n; p++)
    {
        // start new row if needed
        if(p == n_cols[0])
        {
            draw_x = start_x + padding;
            draw_y += box_height + padding;
        }

        int p_ret = s_players[p].Mouse_Render(render, draw_x, draw_y, box_width, box_height);

        if(p_ret != 0)
            ret = p_ret;

        draw_x += box_width + padding;
    }

    return ret;
}

int MouseLogic()
{
    // check mouse release if not in main menu
    if(s_context == Context::DropAdd)
    {
        if(!SharedCursor.Primary && !SharedCursor.Secondary)
            MenuMouseRelease = true;
    }

    int ret = Mouse_Render(true, false);

    // and set it to false if needed
    if(SharedCursor.Primary || SharedCursor.Secondary)
        MenuMouseRelease = false;

    return ret;
}

int PlayerBox::Logic()
{
    int p = CalcIndex();
    auto* input_method = (p < (int)Controls::g_InputMethods.size()) ? Controls::g_InputMethods[p] : nullptr;


    // hide "Drop Me", if it is no longer allowed
    if(m_state == PlayerState::ControlsMenu
        && m_menu_item == -1 && !ControlsMenu_ShowDrop(p))
    {
        m_menu_item = 0;
        m_marquee_state.reset_width();
    }

    // if player's input was lost, mark disconnected and return (unless in 1P menu)
    if(!Is1P() && !input_method)
    {
        m_input_ready = false;

        if(m_state != PlayerState::Reconnecting && m_state != PlayerState::Disconnected)
        {
            m_state = PlayerState::Disconnected;
            m_marquee_state.reset_width();
        }

        return 0;
    }

    // if the player has a controller and was previously disconnected, mark them as connected
    if(m_state == PlayerState::Disconnected || m_state == PlayerState::Reconnecting)
    {
        m_marquee_state.reset_width();
        m_input_ready = false;

        if(IsMenu())
        {
            if(m_state == PlayerState::Reconnecting) // requested reconnect
            {
                m_state = PlayerState::SelectChar;
                m_menu_item = 0;
                PlaySoundMenu(SFX_Pet);
            }
            else
            {
                m_input_ready = true;
                m_just_added = true;

                m_menu_item = 0;
                if(p >= s_minPlayers)
                    l_screen->charSelect[p] = s_recent_char[p];
                m_state = PlayerState::SelectChar;
                ValidateChar();
            }
        }
        else if(s_context == Context::DropAdd)
        {
            // this is an Add situation
            if(p >= l_screen->player_count)
            {
                m_menu_item = 0;
                l_screen->charSelect[p] = s_recent_char[p];
                m_state = PlayerState::SelectChar;

                // check whether a novel add
                if(p >= s_char_info.max_players)
                {
                    m_current_add = 2;
                    s_char_info.max_players = p + 1;
                }
                else
                {
                    m_current_add = 1;
                }

                // validate character
                ValidateChar();

                // add the player immediately
                UpdatePlayer();
            }
            else // was disconnected
            {
                m_state = PlayerState::SelectChar;
                m_menu_item = 0;
                PlaySoundMenu(SFX_Pet);
            }
        }

        // wait in case controls are not yet updated
        return 0;
    }

    const Controls_t& c = Player[p+1].Controls;

    if(m_just_added)
    {
        // play drop item noise?
        bool play_noise = (s_context != Context::LegacyMenu);

        // block back if a new-added player
        if((int)Controls::g_InputMethods.size() > s_minPlayers && (c.Run || SharedControls.MenuBack))
            m_input_ready = false;
        // if pressing back, don't play drop item noise
        else if(c.Run || SharedControls.MenuBack)
            play_noise = false;

        // if about to move cursor, don't play drop item noise
        if(m_input_ready && (c.Down || c.Up || c.Left || c.Right))
            play_noise = false;

        // don't allow going forwards at main menu
        if(s_context == Context::MainMenu && m_input_ready && (c.Jump || c.Start))
            m_input_ready = false;

        if(play_noise)
            PlaySoundMenu(SFX_DropItem);

        m_just_added = false;
    }

    // don't do any logic until all buttons have been released
    if(!m_input_ready)
    {
        if(!SharedControls.MenuDo && !SharedControls.MenuBack && !SharedControls.MenuDown
            && !SharedControls.MenuUp && !SharedControls.MenuLeft && !SharedControls.MenuRight
            && !c.Jump && !c.Start && !c.Run && !c.Down && !c.Up && !c.Left && !c.Right && !c.Drop && !c.AltRun && !c.AltJump)
        {
            m_input_ready = true;
        }

        return 0;
    }

    if(IsMenu() && !g_forceCharacter)
    {
        if((m_konami_bits == 0 && c.Up)
            || (m_konami_bits == 1 && c.Up)
            || (m_konami_bits == 2 && c.Down)
            || (m_konami_bits == 3 && c.Down)
            || (m_konami_bits == 4 && c.Left)
            || (m_konami_bits == 5 && c.Right)
            || (m_konami_bits == 6 && c.Left)
            || (m_konami_bits == 7 && c.Right)
            || (m_konami_bits == 8 && c.AltRun)
            || (m_konami_bits == 9 && c.AltJump))
        {
            m_konami_bits++;

            if(m_konami_bits == 10)
            {
                for(int A = 1; A <= numCharacters; A++)
                    blockCharacter[A] = false;

                g_forceCharacter = true;

                PlaySoundMenu(SFX_MedalGet);
                m_input_ready = false;
                return 0;
            }
            else if(m_konami_bits >= 9)
            {
                m_input_ready = false;
                return 0;
            }
        }
        else if(c.Jump || c.Start || c.Run || c.Down || c.Up || c.Left || c.Right || c.Drop || c.AltRun || c.AltJump)
            m_konami_bits = 0;
    }

    if(m_state == PlayerState::ConfirmProfile)
    {
        bool no_buttons = !c.Run && !c.Jump && !c.AltRun && !c.AltJump && !c.Start && !c.Drop && !c.Down && !c.Up && !c.Left && !c.Right;

        if(!no_buttons && m_menu_item < 66 * 3)
            m_menu_item = (4 * m_menu_item + 66 * 3) / 5;

        if(c.Start)
            m_menu_item += 2;
        else if(m_menu_item > 66 * 3 || no_buttons)
            m_menu_item--;

        if(m_menu_item < 0)
        {
            // Go back to profile select screen, if possible
            if(input_method)
            {
                m_state = PlayerState::SelectProfile;

                // set the current menu item to the profile that the player tried to activate.
                std::vector<Controls::InputMethodProfile*> profiles = input_method->Type->GetProfiles();
                std::vector<Controls::InputMethodProfile*>::iterator p_profile
                    = std::find(profiles.begin(), profiles.end(), input_method->Profile);

                if(profiles.size() > 1 && p_profile != profiles.end())
                    m_menu_item = p_profile - profiles.begin();
                else
                    m_menu_item = 0;

                Controls::SetInputMethodProfile(input_method, m_last_profile);
            }
            else
            {
                s_controls_dirty = true;
                m_state = PlayerState::ControlsMenu;
                m_menu_item = 1;
            }

            m_last_profile = nullptr;
            m_input_ready = false;
            m_marquee_state.reset_width();
            PlaySoundMenu(SFX_Slide);
        }
        else if(m_menu_item > 66 * 5)
        {
            PlaySoundMenu(SFX_Do);

            if(Is1P())
            {
                m_menu_item = 0;
                m_state = PlayerState::SelectChar;
            }
            else
            {
                m_menu_item = 1;
                m_state = PlayerState::ControlsMenu;
            }

            m_last_profile = nullptr;
            m_input_ready = false;
            m_marquee_state.reset_width();
        }

        return 0;
    }

    if(m_state == PlayerState::ForceDrop)
    {
        int to_drop = CheckCanDrop();

        if(to_drop <= -2)
        {
            PlaySoundMenu(SFX_BlockHit);
            m_state = PlayerState::SelectChar;
            m_menu_item = 0;
            m_marquee_state.reset_width();
            m_input_ready = false;
        }
        else if(c.Run)
            m_menu_item += 1;
        else if(m_menu_item >= 0)
            m_menu_item -= 2;

        if(m_menu_item > 66 * 2)
        {
            if(to_drop >= 0)
                Player_Remove(to_drop);
            else
                QuickReconnectScreen::g_active = true;

            if(to_drop == -1 || CheckDone())
            {
                PlaySoundMenu(SFX_Slide);
                return -1;
            }
            else
                PlaySoundMenu(SFX_PlayerDied2);

            m_menu_item = 0;
        }
        else if(m_menu_item < 0)
        {
            m_state = PlayerState::ControlsMenu;
            m_menu_item = 0;
            m_marquee_state.reset_width();
            m_input_ready = false;
        }

        return 0;
    }
    else if(c.Run || (Is1P() && SharedControls.MenuBack))
    {
        if(Back())
            return -1;
    }
    else if(c.Jump || c.Start || (Is1P() && SharedControls.MenuDo))
    {
        if(Do())
        {
            if(Controls::g_InputMethods.empty())
                return -1;

            return 1;
        }
    }
    else if(c.Down || (Is1P() && SharedControls.MenuDown))
    {
        Down();
    }
    else if(c.Up || (Is1P() && SharedControls.MenuUp))
    {
        Up();
    }
    else if(c.Left || (Is1P() && SharedControls.MenuLeft))
    {
        Left();
    }
    else if(c.Right || (Is1P() && SharedControls.MenuRight))
    {
        Right();
    }
    else if(Is1P() && m_state == PlayerState::SelectChar && c.Drop)
    {
        // force Select Profile mode
        m_state = PlayerState::ControlsMenu;
        m_menu_item = 1;
        Do();
    }

    return 0;
}

int Logic()
{
    /*-----------------------*\
    ||    Shared back key    ||
    \*-----------------------*/
    if(s_context == Context::MainMenu && Controls::g_InputMethods.size() == 0)
    {
        if(!SharedControls.MenuBack)
            MenuCursorCanMove = true;

        if((SharedControls.MenuBack && MenuCursorCanMove) || (SharedCursor.Secondary && MenuMouseRelease))
        {
            PlaySoundMenu(SFX_Slide);
            MenuCursorCanMove = false;
            return -1;
        }
    }

    /*-----------------------*\
    ||   per-player logic    ||
    \*-----------------------*/

    for(int p = 0; p < maxLocalPlayers; p++)
    {
        int p_ret = s_players[p].Logic();
        if(p_ret)
        {
            s_char_info.mark_present();
            if(p_ret == 1 || s_context == Context::DropAdd)
                s_logRecentChars();
            return p_ret;
        }
    }

    int ret = MouseLogic();
    if(ret)
    {
        s_char_info.mark_present();
        if(ret == 1 || s_context == Context::DropAdd)
            s_logRecentChars();
        return ret;
    }

    /*-----------------------*\
    || polling input methods ||
    \*-----------------------*/
    bool block_poll = false;

    // only allow the correct number of players to connect
    int first_not_connected = std::find(Controls::g_InputMethods.begin(), Controls::g_InputMethods.end(), nullptr) - Controls::g_InputMethods.begin();
    if(first_not_connected >= BoxCount())
        block_poll = true;

    // block polling if a player might need to restore their previous profile
    for(int p = 0; p < maxLocalPlayers; p++)
    {
        if(s_players[p].m_state == PlayerState::ConfirmProfile)
            block_poll = true;
    }

    if(!block_poll)
        Controls::PollInputMethod();

    return 0;
}

void Render()
{
    Mouse_Render(false, true);
}

// reset allowed char tracking
void SaveChars()
{
    s_char_info.reset();
    s_char_info.mark_present();
}

} // namespace ConnectScreen
