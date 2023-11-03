/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <list>
#include <sstream>
#include <unordered_map>

#include <Logger/logger.h>
#include <fmt_format_ne.h>

#include "sdl_proxy/sdl_stdinc.h"

#include "autocode.h"
#include "autocode_manager.h"
#include "lunamisc.h"
#include "sprite_funcs.h"
#include "csprite.h"
#include "globals.h"
#include "global_dirs.h"
#include "player.h"
#include "graphics.h"
#include "sound.h"
#include "layers.h"
#include "core/msgbox.h"
#include "main/cheat_code.h"
#include "luna.h"
#include "lunarender.h"
#include "lunaplayer.h"
#include "lunalevel.h"
#include "lunanpc.h"
#include "lunalayer.h"
#include "lunablock.h"
#include "lunacell.h"
#include "lunaspriteman.h"
#include "lunainput.h"
#include "lunavarbank.h"
#include "lunacounter.h"
#include "renderop_string.h"
#include "mememu.h"
#include "compat.h"
#include "main/speedrunner.h"

#include "main/trees.h"

static FIELDTYPE StrToFieldtype(std::string string)
{
    string.erase(string.find_last_not_of(" \n\r\t") + 1);
    if(string == "b")
        return FT_BYTE;
    else if(string == "s" || string == "w")
        return FT_WORD;
    else if(string == "dw")
        return FT_DWORD;
    else if(string == "f")
        return FT_FLOAT;
    else if(string == "df")
        return FT_DFLOAT;

    return FT_BYTE;
}


Autocode::Autocode()
{
    m_Type = AT_Invalid;
}

Autocode::Autocode(AutocodeType iType, double iTarget, double ip1, double ip2, double ip3,
                   const stringindex_t& ip4, double iLength, int iSection, const stringindex_t& iVarRef)
{
    m_Type = iType;
    Target = iTarget;
    Param1 = ip1;
    Param2 = ip2;
    Param3 = ip3;
    Length = iLength;
    MyString = ip4;
    MyRef = iVarRef;
    m_OriginalTime = iLength;
    ftype = FT_INVALID;
    Activated = true;
    Expired = false;
    //comp = nullptr;

    // Adjust section
    ActiveSection = (iSection < 1000 ? --iSection : iSection);
    Activated = (iSection < 1000);
}

Autocode::Autocode(const Autocode &o)
{
    operator=(o);
}

Autocode &Autocode::operator=(const Autocode &o)
{
    m_Type = o.m_Type;

    Target = o.Target;
    Param1 = o.Param1;
    Param2 = o.Param2;
    Param3 = o.Param3;
    Length = o.Length;
    // de-duplicate strings, while re-using allocated string indices if possible
    MyString = o.MyString;
    MyRef = o.MyRef;

    m_OriginalTime = o.m_OriginalTime;
    ActiveSection = o.ActiveSection;
    ftype = o.ftype;
    Activated = o.Activated;
    Expired = o.Expired;

    return *this;
}

// DO - Perform autocodes for this section. Only does init codes if "init" is set
void Autocode::Do(bool init)
{
    // Is it expired?
    if(Expired || !Activated)
        return;

    // Make sure game is in OK state to run
    Player_t *demo = PlayerF::Get(1);

    if(!demo)
        return;

    // Only allow initrun on codes set to section -1
    if(init)
        init = (uint8_t)ActiveSection == (uint8_t)0xFE;

    // Run this code if "always" section, or if current section is a match, or forced by init
    if((uint8_t)ActiveSection == (uint8_t)0xFF || demo->Section == ActiveSection || init)
    {
        int pretick_len = (int)this->Length;
        this->SelfTick();

        // Run specified autocode
        switch(m_Type)
        {
        // INVALID
        case 0 /*Invalid*/:
        default:
            break;

        // FILTERS
        case AT_FilterToSmall:
        {
            PlayerF::FilterToSmall(demo);
            break;
        }
        case AT_FilterToBig:
        {
            PlayerF::FilterToBig(demo);
            break;
        }
        case AT_FilterToFire:
        {
            PlayerF::FilterToFire(demo);
            break;
        }
        case AT_FilterMount:
        {
            PlayerF::FilterMount(demo);
            break;
        }
        case AT_FilterReservePowerup:
        {
            PlayerF::FilterReservePowerup(demo);
            break;
        }

        case AT_FilterPlayer:
        {
            // Player_t *demo = PlayerF::Get(1); // Already declared above
            //if(demo) // Useless, null check was done above
            //{
            if(demo->Character == Param1)
            {
                if(Param2 > 0 && Param2 < 6)
                    demo->Character = (int)Param2;
            }
            //}
            break;
        }

        // INFINITE FLYING
        case AT_InfiniteFlying:
            PlayerF::InfiniteFlying(1);
            break;

        // SET HEARTS
        case AT_SetHearts:
            demo->Hearts = (short)Param1;
            if(Param1 > 1 && demo->State < 2)
                demo->State = 2;
            break;

        // HEART SYSTEM
        case AT_HeartSystem:
            HeartSystem();
            break;

        // SCREEN EDGE BUFFER
        case AT_ScreenEdgeBuffer:
        {
            //char* dbg = "SCREEN EDGE DBG";

            // Get all target NPCs in section into a list (slow, could easily optimize using NPCQueues::Active.no_change)
            std::list<NPC_t *> npcs;
            NpcF::FindAll((int)Target, demo->Section, &npcs);

            if(!npcs.empty())
            {
                // Loop over list of NPCs
                for(auto npc : npcs)
                {
                    switch((int)Param1)
                    {
                    default:
                    case 0: // UP
                    {
                        //double* pCamera = vScreenY;
                        double top = -vScreen[1].Y;
                        if(npc->Location.Y < top + Param2)
                        {
                            npc->Location.Y = (top + Param2) + 1;
                            treeNPCUpdate(npc);
                        }
                        break;
                    }

                    case 1:  // DOWN
                    {
                        //double* pCamera = GM_CAMERA_Y;
                        double bot = -vScreen[1].Y + ScreenH;
                        if(npc->Location.Y > bot - Param2)
                        {
                            npc->Location.Y = (bot - Param2) - 1;
                            treeNPCUpdate(npc);
                        }
                        break;
                    }

                    case 2: // LEFT
                    {
                        //double* pCamera = GM_CAMERA_X;
                        double left = -vScreen[1].X;
                        if(npc->Location.X < left + Param2)
                        {
                            npc->Location.X = (left + Param2) + 1;
                            treeNPCUpdate(npc);
                        }
                        break;
                    }

                    case 3: // RIGHT
                    {
                        //double* pCamera = GM_CAMERA_X;
                        double rt = -vScreen[1].X + ScreenW;
                        if(npc->Location.Y > rt - Param2)
                        {
                            npc->Location.Y = (rt - Param2) - 1;
                            treeNPCUpdate(npc);
                        }
                        break;
                    }
                    }
                }
            }
            break;
        }



        // SHOW TEXT
        case AT_ShowText:
            Renderer::Get().AddOp(new RenderStringOp(GetS(MyString), (int)Param3, (float)Param1, (float)Param2));
            break;

        // SHOW NPC LIFE LEFT
        case AT_ShowNPCLifeLeft:
        {
            int base_health = SDL_atoi(GetS(MyString).data());
            NPC_t *npc = NpcF::GetFirstMatch((int)Target, (int)Param3 - 1);
            if(npc != nullptr)
            {
                //float hits = *(((float *)((&(*(uint8_t *)npc)) + 0x148)));
                int hits = Maths::iRound(npc->Damage);
                Renderer::Get().AddOp(new RenderStringOp(fmt::format_ne("{0}", (base_health - hits)), 3, (float)Param1, (float)Param2));
            }
            else
                Renderer::Get().AddOp(new RenderStringOp("?", 3, (float)Param1, (float)Param2));
            break;
        }

        // Show level's internal name (or show the filename if empty)
        case AT_ShowLevelName:
            Renderer::Get().AddOp(new RenderStringOp(LevelName.empty() ? FileName : LevelName, (int)Param3, (float)Param1, (float)Param2));
            break;

        // Show level's file name (without extension)
        case AT_ShowLevelFile:
            Renderer::Get().AddOp(new RenderStringOp(FileName, (int)Param3, (float)Param1, (float)Param2));
            break;

        // AUDIO
        case AT_SFX:
        {
            if(this->Length <= 1) // Play once when delay runs out
            {
                // Play built in sound
                if(Param1 > 0)
                    PlaySound((int)Param1);
                else
                {
                    // Sound from level folder
                    if(GetS(MyString).length() > 0)
                    {
                        //char* dbg = "CUSTOM SOUND PLAY DBG";
                        std::string full_path = g_dirCustom.resolveFileCaseAbs(GetS(MyString));
                        PlayExtSound(full_path);
                    }

                }
                expire();
            }
            break;
        }

        case AT_PlaySFX:
        {
            if(this->Length <= 1) // Play once when delay runs out
            {
                // Play built in sound
                if(Param1 > 0)
                    PlaySound((int)Param1, (int)Param2, (int)(Param3 <= 0.0 ? 128 : Param3));
                else
                {
                    // Sound from level folder
                    if(GetS(MyString).length() > 0)
                    {
                        //char* dbg = "CUSTOM SOUND PLAY DBG";
                        std::string full_path = g_dirCustom.resolveFileCaseAbs(GetS(MyString));
                        PlayExtSound(full_path, (int)Param2, (int)(Param3 <= 0.0 ? 128 : Param3));
                    }

                }
                expire();
            }
            break;
        }

        case AT_StopSFX:
        {
            if(this->Length <= 1) // Stop once when delay runs out
            {
                // Sound from level folder
                if(GetS(MyString).length() > 0)
                {
                    //char* dbg = "CUSTOM SOUND STOP DBG";
                    std::string full_path = g_dirCustom.resolveFileCaseAbs(GetS(MyString));
                    StopExtSound(full_path);
                }
                expire();
            }
            break;
        }

        case AT_SFXPreLoad:
        {
            if(this->Length <= 1) // Preload custom SFX file
            {
                // Sound from level folder
                if(GetS(MyString).length() > 0)
                {
                    //char* dbg = "CUSTOM SOUND PLAY DBG";
                    std::string full_path = g_dirCustom.resolveFileCaseAbs(GetS(MyString));
                    PreloadExtSound(full_path);
                }
                expire();
            }
            break;
        }

        case AT_SetMusic:
        {
            int sec = (int)Target - 1;
            if(sec >= 0 && sec <= numSections)
            {
                if(Param1 >= 0.0 && Param1 <= 24)
                    bgMusic[sec] = (int)Param1;
                if(GetS(MyString).length() >= 5)
                    CustomMusic[sec] = GetS(MyString);
            }
            break;
        }

        case AT_PlayMusic:
        {
            if(Length <= 1)   // Play once when delay runs out
            {
                StartMusic((int)Param1 - 1);
                expire();
            }
            break;
        }

        // EVENTS & BRANCHES
        case AT_Trigger:
        {
            expire();
            gAutoMan.ActivateCustomEvents((int)Target, (int)Param1);
            break;
        }

        case AT_Timer:
            if(Param2 != 0.0) // Display timer?
            {
                Renderer::Get().AddOp(new RenderStringOp("TIMER", 3, 600, 27));
                Renderer::Get().AddOp(new RenderStringOp(fmt::format_ne("{0}", (int64_t)Length / 60), 3, 618, 48));
            }

            if(Length == 1 || Length == 0)
            {
                if(Length == 1)
                    expire();

                Autocode::DoPredicate((int)Target, (int)Param1);

                // Reset time?
                if(Param3 != 0.0)
                {
                    Activated = true;
                    Expired = false;
                    Length = m_OriginalTime;
                }
            }
            break;

        case AT_IfNPC:
        {
            if(numNPCs < 1)
                break;

            if(Autocode::NPCConditional((int)Target, (int)Param1))
            {
                RunSelfOption();
                gAutoMan.ActivateCustomEvents((int)Param2, (int)Param3);
            }
            break;
        }

        case AT_BlockTrigger:
        {
            if(Target == 0) //if target is player
            {
                if(BlocksF::IsPlayerTouchingType((int)Param1, (int)Param2, demo))
                {
                    RunSelfOption();
                    gAutoMan.ActivateCustomEvents(0, (int)Param3);
                }
            }
            break;
        }

        case AT_IfCompatMode:
        {
            if(CheckConditionI(CompatGetLevel(), (int)Param2, (COMPARETYPE)(int)Param1))
                gAutoMan.ActivateCustomEvents(0, (int)Param3);
            break;
        }

        case AT_IfSpeedRunMode:
        {
            if(CheckConditionI(g_speedRunnerMode, (int)Param2, (COMPARETYPE)(int)Param1))
                gAutoMan.ActivateCustomEvents(0, (int)Param3);
            break;
        }

        case AT_TriggerRandom:
        {
            int choice = iRand2(4);
            switch(choice)
            {
            default:
                break;
            case 0:
                gAutoMan.ActivateCustomEvents(0, (int)Target);
                break;
            case 1:
                gAutoMan.ActivateCustomEvents(0, (int)Param1);
                break;
            case 2:
                gAutoMan.ActivateCustomEvents(0, (int)Param2);
                break;
            case 3:
                gAutoMan.ActivateCustomEvents(0, (int)Param3);
                break;
            }
            break;
        }

        case AT_TriggerRandomRange:
        {
            if(Target >= Param1) // rule out bad values
                break;

            int diff = (int)Param1 - (int)Target;
            int choice = iRand2(diff);
            gAutoMan.ActivateCustomEvents(0, (int)Target + choice);
            break;
        }

        case AT_TriggerZone:
        {
            Length++; // undo length decrement
            if(demo->Location.X > Param3 &&
               demo->Location.Y > Param1 &&
               demo->Location.Y < Param2 &&
               demo->Location.X < Length)
            {
                gAutoMan.ActivateCustomEvents(0, (int)Target);
                RunSelfOption();
            }
            break;
        }

        case AT_ScreenBorderTrigger:
        {
            LunaRect player_screen_rect = PlayerF::GetScreenPosition(demo);
            int depth = 0;
            if(GetS(MyString).length() > 0)
                depth = SDL_atoi(GetS(MyString).c_str());

            double L_edge = 0 + depth;
            double U_edge = 0 + depth;
            double D_edge = ScreenH - depth;
            double R_edge = ScreenW - depth;

            if(demo->WarpCD < 1)
            {
                if(player_screen_rect.left <= L_edge && demo->Location.SpeedX < 0)
                {
                    gAutoMan.ActivateCustomEvents(0, (int)Target);
                    demo->WarpCD = 2;
                }
                else if(player_screen_rect.top <= U_edge  && demo->Location.SpeedY < 0)
                {
                    gAutoMan.ActivateCustomEvents(0, (int)Param1);
                    demo->WarpCD = 2;
                }
                else if(player_screen_rect.right >= R_edge  && demo->Location.SpeedX > 0)
                {
                    gAutoMan.ActivateCustomEvents(0, (int)Param2);
                    demo->WarpCD = 2;
                }
                else if(player_screen_rect.bottom >= D_edge && demo->Location.SpeedY > 0)
                {
                    gAutoMan.ActivateCustomEvents(0, (int)Param3);
                    demo->WarpCD = 2;
                }
            }
            break;
        }

        case AT_OnInput:
        {
            switch((int)Param1)
            {
            case 1: // Up
                if(Param2 == 1 && Input::UpThisFrame()) // First frame press only
                    gAutoMan.ActivateCustomEvents(0, (int)Param3);
                else if(Param2 != 1 && Input::PressingUp()) // Any press
                    gAutoMan.ActivateCustomEvents(0, (int)Param3);
                break;

            case 2: // Down
                if(Param2 == 1 && Input::DownThisFrame()) // First frame press only
                    gAutoMan.ActivateCustomEvents(0, (int)Param3);
                else if(Param2 != 1 && Input::PressingDown()) // Any press
                    gAutoMan.ActivateCustomEvents(0, (int)Param3);
                break;

            case 3: // Left
                if(Param2 == 1 && Input::LeftThisFrame()) // First frame press only
                    gAutoMan.ActivateCustomEvents(0, (int)Param3);
                else if(Param2 != 1 && Input::PressingLeft()) // Any press
                    gAutoMan.ActivateCustomEvents(0, (int)Param3);
                break;

            case 4: // Right
                if(Param2 == 1 && Input::RightThisFrame()) // First frame press only
                    gAutoMan.ActivateCustomEvents(0, (int)Param3);
                else if(Param2 != 1 && Input::PressingRight()) // Any press
                    gAutoMan.ActivateCustomEvents(0, (int)Param3);
                break;

            case 5: //Run
                if(Param2 == 1 && Input::RunThisFrame()) // First frame press only
                    gAutoMan.ActivateCustomEvents(0, (int)Param3);
                else if(Param2 != 1 && Input::PressingRun()) // Any press
                    gAutoMan.ActivateCustomEvents(0, (int)Param3);
                break;

            case 6: //Jump
                if(Param2 == 1 && Input::JumpThisFrame()) // First frame press only
                    gAutoMan.ActivateCustomEvents(0, (int)Param3);
                else if(Param2 != 1 && Input::PressingJump()) // Any press
                    gAutoMan.ActivateCustomEvents(0, (int)Param3);
                break;
            }
            break;
        }

        case AT_OnPlayerMem:
        {
            if(ftype == FT_INVALID)
            {
                ftype = FT_BYTE;
                ftype = StrToFieldtype(GetS(MyString));
            }
//            uint8_t *ptr = (uint8_t *)demo;
//            ptr += (int)Target; // offset
            bool triggered = CheckMem(demo, (size_t)Target, Param1, (COMPARETYPE)(int)Param2, ftype);
            if(triggered)
                gAutoMan.ActivateCustomEvents(0, (int)Param3);
            break;
        }

        case AT_OnGlobalMem:
        {
            if(ftype == FT_INVALID)
            {
                ftype = FT_BYTE;
                ftype = StrToFieldtype(GetS(MyString));
            }

            bool triggered = CheckMem((size_t)Target, Param1, (COMPARETYPE)(int)Param2, ftype);
            if(triggered)
                gAutoMan.ActivateCustomEvents(0, (int)Param3);
            break;
        }

        // USER VARS
        case AT_SetVar:
        {
            if(ReferenceOK())
                gAutoMan.VarOperation(GetS(MyRef), Param2, (OPTYPE)(int)Param1);
            else
                gAutoMan.VarOperation(GetS(MyString), Param2, (OPTYPE)(int)Param1);
            break;
        }

        case AT_LoadPlayerVar:
        {
            if(!this->ReferenceOK() || Param1 > (0x184 * 99))
                break;

            if(ftype == FT_INVALID)
            {
                ftype = FT_BYTE;
                ftype = StrToFieldtype(GetS(MyString));
            }

            // Get the memory
            //uint8_t *ptr = (uint8_t *)demo;
            //ptr += (int)Param1; // offset
            double gotval = GetMem(demo, (size_t)Param1, ftype);

            // Perform the load/add/sub/etc operation on the banked variable using the ref as the name
            gAutoMan.VarOperation(GetS(MyRef), gotval, (OPTYPE)(int)Param2);

            break;
        }

        case AT_LoadNPCVar:
        {
            if(!this->ReferenceOK() || Param1 > (0x158))
                break;
            if(ftype == FT_INVALID)
            {
                ftype = FT_BYTE;
                ftype = StrToFieldtype(GetS(MyString));
            }

            NPC_t *pFound_npc = NpcF::GetFirstMatch((int)Target, (int)Param3);
            if(pFound_npc != nullptr)
            {
                double gotval = GetMem(pFound_npc, (size_t)Param1, ftype);
                gAutoMan.VarOperation(GetS(MyRef), gotval, (OPTYPE)(int)Param2);
            }

            break;
        }

        case AT_LoadGlobalVar:
        {
            if(Target >= GM_BASE && Param1 <=  GM_END && ReferenceOK())
            {
                if(ftype == FT_INVALID)
                {
                    ftype = FT_BYTE;
                    ftype = StrToFieldtype(GetS(MyString));
                }

                // byte *ptr = (byte *)(int)Target;
                double gotval = GetMem((size_t)Target, ftype);
                gAutoMan.VarOperation(GetS(MyRef), gotval, (OPTYPE)(int)Param1);
            }
            break;
        }

        case AT_IfVar:
        {
            if(!ReferenceOK())
            {
                InitIfMissing(&gAutoMan.m_UserVars, GetS(MyString), 0);// Initalize var if not existing
            }
            double varval;
            if(ReferenceOK())
                varval = gAutoMan.m_UserVars[GetS(MyRef)];
            else
                varval = gAutoMan.m_UserVars[GetS(MyString)];

            // Check if the value meets the criteria and activate event if so
            if(CheckConditionD(varval, Param2, (COMPARETYPE)(int)Param1))
                gAutoMan.ActivateCustomEvents(0, (int)Param3);

            break;
        }

        case AT_CompareVar:
        {
            if(ReferenceOK())
            {
                auto compare_type = (COMPARETYPE)(int)Param1;
                InitIfMissing(&gAutoMan.m_UserVars, GetS(MyString), 0);
                InitIfMissing(&gAutoMan.m_UserVars, GetS(MyRef), 0);

                double var1 = gAutoMan.m_UserVars[GetS(MyRef)];
                double var2 = gAutoMan.m_UserVars[GetS(MyString)];

                if(CheckConditionD(var1, var2, compare_type))
                    gAutoMan.ActivateCustomEvents(0, (int)Param3);
            }
            break;
        }

        case AT_ShowVar:
        {
            if(ReferenceOK())
            {
                std::string str = fmt::format_ne("{0}", gAutoMan.GetVar(GetS(MyRef)));
                if(GetS(MyString).length() > 0)
                    str = GetS(MyString) + str;
                Renderer::Get().AddOp(new RenderStringOp(str, (int)Param3, (float)Param1, (float)Param2));
            }
            break;
        }

        case AT_BankVar:
        {
            if(GetS(MyString).length() > 0)
                gSavedVarBank.SetVar(GetS(MyString), gAutoMan.GetVar(GetS(MyString)));
            break;
        }

        case AT_WriteBank:
        {
            gSavedVarBank.WriteBank();
            break;
        }


        // LUNA CONTROL
        case AT_LunaControl:
        {
            Autocode::LunaControl((LunaControlAct)(int)Target, (int)Param1);
            break;
        }


        // DELETE COMMAND
        case AT_DeleteCommand:
        {
            gAutoMan.DeleteEvent(GetS(MyString));
            break;
        }

        // MOD PARAM
        case AT_ModParam:
        {
            if(Target < 6 && Target > 0)
            {
                Autocode *coderef = nullptr;
                coderef = gAutoMan.GetEventByRef(GetS(MyString));
                if(coderef)
                {
                    switch((int)Target)
                    {
                    case 1:
                        Autocode::modParam(coderef->Target, Param1, (OPTYPE)(int)Param2);
                        break;
                    case 2:
                        Autocode::modParam(coderef->Param1, Param1, (OPTYPE)(int)Param2);
                        break;
                    case 3:
                        Autocode::modParam(coderef->Param2, Param1, (OPTYPE)(int)Param2);
                        break;
                    case 4:
                        Autocode::modParam(coderef->Param3, Param1, (OPTYPE)(int)Param2);
                        break;
                    case 5:
                        Autocode::modParam(coderef->Length, Param1, (OPTYPE)(int)Param2);
                        break;
                    }
                }
            }
            break;
        }

        // ChangeTime
        case AT_ChangeTime:
        {
            Autocode *coderef = nullptr;
            coderef = gAutoMan.GetEventByRef(GetS(MyString));
            if(coderef)
                Autocode::modParam(coderef->Length, Param1, (OPTYPE)(int)Param2);
            break;
        }

        // INPUT BUFFER STUFF
        case AT_OnCustomCheat:
        {
            if(cheats_contains(GetS(MyString)))
            {
                gAutoMan.ActivateCustomEvents(0, (int)Param3);
                cheats_clearBuffer();
                if(Param2 != 0)
                    this->expire();
            }
            break;
        }

        case AT_ClearInputString:
        {
            //wchar_t* dbg = L"ClearInputString debug";
            cheats_clearBuffer();
            break;
        }

        case AT_RunCheat:
        {
            if(this->Length <= 1) // Play once when delay runs out
            {
                cheats_setBuffer(GetS(MyString));
                this->expire();
            }
            break;
        }

        case AT_DeleteEventsFrom:
        {
            gAutoMan.ForceExpire((int)Target);
            break;
        }

        // LAYER CONTROL
        case AT_LayerXSpeed:
        {
            Layer_t *layer = LayerF::Get((int)Target);
            if(layer)
            {
                LayerF::SetXSpeed(layer, (float)SDL_atof(GetS(MyString).c_str()));
                if(Length == 1 && Param1 != 0.0)
                    LayerF::SetXSpeed(layer, 0.0001f);
            }
            break;
        }

        case AT_LayerYSpeed:
        {
            Layer_t *layer = LayerF::Get((int)Target);
            if(layer)
            {
                LayerF::SetYSpeed(layer, (float)SDL_atof(GetS(MyString).c_str()));
                if(Length == 1 && Param1 != 0.0)
                    LayerF::SetYSpeed(layer, 0.0001f);
            }
            break;
        }

        case AT_AccelerateLayerX:
        {
            Layer_t *layer = LayerF::Get((int)Target);
            if(layer)
            {
                auto accel = (float)SDL_atof(GetS(MyString).c_str());
                if(std::abs(layer->SpeedX) + std::abs(accel) >= std::abs((float)Param1))
                    LayerF::SetXSpeed(layer, (float)Param1);
                else
                    LayerF::SetXSpeed(layer, layer->SpeedX + (float)accel);
            }
            break;
        }

        case AT_AccelerateLayerY:
        {
            Layer_t *layer = LayerF::Get((int)Target);
            if(layer)
            {
                auto accel = (float)SDL_atof(GetS(MyString).c_str());
                if(std::abs(layer->SpeedY) + std::abs(accel) >= std::abs((float)Param1))
                    LayerF::SetYSpeed(layer, (float)Param1);
                else
                    LayerF::SetYSpeed(layer, layer->SpeedY + (float)accel);
            }
            break;
        }

        case AT_DeccelerateLayerX:
        {
            Layer_t *layer = LayerF::Get((int)Target);
            if(layer)
            {
                auto deccel = (float)SDL_atof(GetS(MyString).c_str());
                deccel = std::abs(deccel);
                if(layer->SpeedX > 0)
                {
                    layer->SpeedX -= deccel;
                    if(layer->SpeedX < 0)
                        LayerF::Stop(layer);
                }
                else if(layer->SpeedX < 0)
                {
                    layer->SpeedX += deccel;
                    if(layer->SpeedX > 0)
                        LayerF::Stop(layer);
                }
            }
            break;
        }


        case AT_DeccelerateLayerY:
        {
            Layer_t *layer = LayerF::Get((int)Target);
            if(layer)
            {
                auto deccel = (float)SDL_atof(GetS(MyString).c_str());
                deccel = std::abs(deccel);
                if(layer->SpeedY > 0)
                {
                    layer->SpeedY -= deccel;
                    if(layer->SpeedY < 0)
                        LayerF::Stop(layer);
                }
                else if(layer->SpeedY < 0)
                {
                    layer->SpeedY += deccel;
                    if(layer->SpeedY > 0)
                        LayerF::Stop(layer);
                }
            }
            break;
        }

        // BLOCK MODIFIERS
        case AT_SetAllBlocksID:
        {
            BlocksF::SetAll((int)Target, (int)Param1);
            break;
        }

        case AT_SwapAllBlocks:
        {
            BlocksF::SwapAll((int)Target, (int)Param1);
            break;
        }

        case AT_ShowAllBlocks:
        {
            BlocksF::ShowAll((int)Target);
            break;
        }

        case AT_HideAllBlocks:
        {
            BlocksF::HideAll((int)Target);
            break;
        }


        case AT_PushScreenBoundary:
        {
            if(Target > 0 && Target < numSections && Param1 >= 0 && Param1 < 5)
                LevelF::PushSectionBoundary((int)Target - 1, (int)Param1, SDL_atof(GetS(MyString).c_str()));
            break;
        }

        case AT_SnapSectionBounds:
        {
            if(Target > 0 && Target <= maxSections) // Make sure valid section
            {
                //RECT current_bounds;
                //int sec = ((int)Target) - 1;
                //Level::GetBoundary(&current_bounds, sec);
                //double x_dist = Param1 - current_bounds.left;
                //double y_dist = Param2 - current_bounds.top;
                //double x_stepsize = x_dist / Length;
                //double y_stepsize = y_dist / Length;
                //double x_stepped =  current_bounds.left + x_stepsize;
                //double y_stepped =  current_bounds.top + y_stepsize;
                //Level::SetSectionBounds(sec, x_stepped,  y_stepped, x_stepped + 800, y_stepped + 600);

                //if(Length <= 1) { // When travel time is up, force screen into the right place
                LevelF::SetSectionBounds((int)Target - 1, Param1, Param2, Param1 + 800, Param2 + 600);
                //}
            }
            break;
        }

        // PLAYER CYCLE
        case AT_CyclePlayerRight:
        {
            PlayerF::CycleRight(demo);
            break;
        }

        case AT_CyclePlayerLeft:
        {
            PlayerF::CycleLeft(demo);
            break;
        }


        // SET HITS
        case AT_SetHits:
        {
            NpcF::AllSetHits((int)Target, (int)Param1 - 1, (float)Param2);
            break;
        }

        // FORCE FACING
        case AT_ForceFacing:
        {
            // Player_t *demo = PlayerF::Get(1); // No need, everything was done above
            // if(demo != 0)
            NpcF::AllFace((int)Target, (int)Param1 - 1, demo->Location.SpeedX);
            break;
        }

        case AT_TriggerSMBXEvent:
        {
            ProcEvent(FindEvent(GetS(MyString)), (int)Param1);
            break;
        }

        case AT_OnEvent:
        {
            if(EventWasTriggered(FindEvent(GetS(MyString))))
            {
                gAutoMan.ActivateCustomEvents(0, (int)Param3);
                if(Param2 != 0)
                    this->expire();
            }
            break;
        }

        case AT_CancelSMBXEvent:
        {
            if(Length <= 1) // Cancel event after delay
            {
                CancelNewEvent(FindEvent(GetS(MyString)));
                expire();
            }
            break;
        }

        // PREDICATES
        case AT_Hurt:
        {
            short tempint = 1;
            if(Target == 0)
                PlayerHurt(tempint);
            RunSelfOption();
            break;
        }

        case AT_Kill:
        {
            short tempint = 1;
            if(Target == 0)
                PlayerDead(tempint);
            RunSelfOption();
            break;
        }

        // NPC MEMORY SET
        case AT_NPCMemSet:
        {
            if(ftype == FT_INVALID)
            {
                ftype = FT_BYTE;
                ftype = StrToFieldtype(GetS(MyString));
            }

            // Assign the mem
            if(ReferenceOK())   // Use referenced var as value
            {
                double gotval = gAutoMan.GetVar(GetS(MyRef));
                NpcF::MemSet((int)Target, (size_t)Param1, gotval, (OPTYPE)(int)Param3, ftype);
            }
            else   // Use given value as value
            {
                NpcF::MemSet((int)Target, (size_t)Param1, Param2, (OPTYPE)(int)Param3, ftype); // NPC ID, offset in obj, value, op, field type
            }

            break;
        }

        // PLAYER MEMORY SET
        case AT_PlayerMemSet:
        {
            if(ftype == FT_INVALID)
            {
                ftype = FT_BYTE;
                ftype = StrToFieldtype(GetS(MyString));
            }
            if(ReferenceOK())
            {
                double gotval = gAutoMan.GetVar(GetS(MyRef));
                PlayerF::MemSet((size_t)Param1, gotval, (OPTYPE)(int)Param3, ftype);
            }
            else
                PlayerF::MemSet((size_t)Param1, Param2, (OPTYPE)(int)Param3, ftype);
            break;
        }


        // MEM ASSIGN
        case AT_MemAssign:
        {
            if(Target >= GM_BASE && Param1 <=  GM_END)
            {
                if(ftype == FT_INVALID)
                {
                    ftype = FT_BYTE;
                    ftype = StrToFieldtype(GetS(MyString));
                }
                if(ReferenceOK())
                {
                    double gotval = gAutoMan.GetVar(GetS(MyRef));
                    MemAssign((size_t)Target, gotval, (OPTYPE)(int)Param2, ftype);
                }
                else
                    MemAssign((size_t)Target, Param1, (OPTYPE)(int)Param2, ftype);
            }
            break;
        }

        // DEBUG
        case AT_DebugPrint:
        {
            Renderer::Get().AddOp(new RenderStringOp(fmt::format_ne("LunaScript (TheXTech) VERSION-{0}", LUNA_VERSION), 3, 50, 250));
            //Renderer::Get().SafePrint(, 3, 340, 250);
            Renderer::Get().AddOp(new RenderStringOp(fmt::format_ne("Globl: {0}", gAutoMan.m_GlobalCodes.size()), 3, 50, 280));
            Renderer::Get().AddOp(new RenderStringOp(fmt::format_ne("Init:  {0}", gAutoMan.m_InitAutocodes.size()), 3, 50, 300));
            Renderer::Get().AddOp(new RenderStringOp(fmt::format_ne("Codes: {0}", gAutoMan.m_Autocodes.size()), 3, 50, 320));
            Renderer::Get().AddOp(new RenderStringOp(fmt::format_ne("Queue: {0}", gAutoMan.m_CustomCodes.size()), 3, 50, 340));
            Renderer::Get().AddOp(new RenderStringOp(fmt::format_ne("Sprites: {0}", gSpriteMan.CountSprites()), 3, 50, 360));
            Renderer::Get().AddOp(new RenderStringOp(fmt::format_ne("BlueePrints: {0}", gSpriteMan.CountBlueprints()), 3, 50, 380));
            Renderer::Get().AddOp(new RenderStringOp(fmt::format_ne("Components: {0}", gSpriteMan.m_ComponentList.size()), 3, 50, 400));

            int buckets = 0, cells = 0, objs = 0;
            gCellMan.CountAll(&buckets, &cells, &objs);
            Renderer::Get().AddOp(new RenderStringOp(fmt::format_ne("Buckets={0} Cells={1} Objs={2}", buckets, cells, objs), 3, 50, 420));

            std::list<CellObj> cellobjs;
            gCellMan.GetObjectsOfInterest(&cellobjs, demo->Location.X, demo->Location.Y, (int)demo->Location.Width, (int)demo->Location.Height);
            Renderer::Get().AddOp(new RenderStringOp(fmt::format_ne("NEAR: {0}", cellobjs.size()), 3, 50, 440));

            Renderer::Get().AddOp(new RenderStringOp(fmt::format_ne("STRINGS: {0}", StringsBankSize()), 3, 50, 460));
            Renderer::Get().AddOp(new RenderStringOp(fmt::format_ne("STRINGS-Unused: {0}", StringsUnusedEntries()), 3, 50, 480));
            break;
        }

        case AT_DebugWindow:
        {
            XMsgBox::simpleMsgBox(XMsgBox::MESSAGEBOX_INFORMATION, "LunaScript debug message", GetS(MyString));
            expire();
            break;
        }

        case AT_CollisionScan:
        {
            gCellMan.ScanLevel(true);
            break;
        }

        // SPRITE FUNCTIONS
        case AT_LoadImage:
        {
            // Only allow loading image during init phase
            if(init)
                Renderer::Get().LoadBitmapResource(GetS(MyString), (int)Target, (uint32_t)(int32_t)Param1);
            expire();
            break;
        }

        case AT_SpriteBlueprint:
        {
            if(ReferenceOK())
            {
                auto *blueprint = new CSprite();
                gSpriteMan.AddBlueprint(GetS(MyRef).c_str(), blueprint);
            }

            expire();
            break;
        }

        case AT_Attach:
        {
            //char* dbg = "!!! ATTACH DEBUG !!!";
            if(ReferenceOK() && !GetS(MyString).empty())
            {
                if(gSpriteMan.m_SpriteBlueprints.find(GetS(MyRef)) != gSpriteMan.m_SpriteBlueprints.end()) // BLueprint exists
                {
                    CSprite *pSpr = gSpriteMan.m_SpriteBlueprints[GetS(MyRef)];                   // Get blueprint
                    Autocode *pComponent = gAutoMan.GetEventByRef(GetS(MyString));                // Get autocode containing component
                    if(pComponent)
                    {
                        switch((BlueprintAttachType)(int)Target)
                        {
                        case BPAT_Behavior:
                            pSpr->AddBehaviorComponent(GenerateComponent(*pComponent));
                            break;
                        case BPAT_Draw:
                            pSpr->AddDrawComponent(GetDrawFunc(*pComponent));
                            break;
                        case BPAT_Birth:
                            pSpr->AddBirthComponent(GenerateComponent(*pComponent));
                            break;
                        case BPAT_Death:
                            pSpr->AddDeathComponent(GenerateComponent(*pComponent));
                            break;
                        default:
                            break;
                        }
                    }
                }
            }

            expire();
            break;
        }

        case AT_PlaceSprite:
        {
            //1: Type   2: ImgResource  3: Xpos         4: Ypos     5:              6: Extra
            CSpriteRequest req;
            req.type = (int)Target;
            req.img_resource_code = (int)Param1;
            req.x = (int)Param2;
            req.y = (int)Param3;
            req.time = pretick_len;
            req.str = GetS(MyString);
            gSpriteMan.InstantiateSprite(&req, false);

            expire();
            break;
        }

        }//switch

    }//section
}

// SELF TICK
void Autocode::SelfTick()
{
    if(Length == 1)
        expire();
    else if(Length == 0)
        return;
    else
        Length--;
}

// DO PREDICATE
void Autocode::DoPredicate(int target, int predicate)
{
    // Activate custom event?
    if(predicate >= 1000 && predicate < 100000)
    {
        gAutoMan.ActivateCustomEvents(target, predicate);
        return;
    }

    // Else, do predicate
    auto pred = (AutocodePredicate)predicate;
    short tempint = 1;

    switch(pred)
    {
    // DEATH PREDICATE
    case AP_Hurt:
        PlayerHurt(tempint);
        break;

    case AP_Death:
        PlayerDead(tempint);
        break;

    default:
        break;
    }
}

// NPC CONDITIONAL
bool Autocode::NPCConditional(int target, int cond)
{
    //const char* dbg = "NPC COND DBG";
    bool ret = false;

    switch((AC_Conditional)cond)
    {
    case AC_Invalid:
    default:
        return ret;

    case AC_Exists:
        return NpcF::GetFirstMatch(target, -1) != nullptr;

    case AC_DoesNotExist:
        return NpcF::GetFirstMatch(target, -1) == nullptr;
    }
}

bool Autocode::CheckConditionI(int value1, int value2, COMPARETYPE cond)
{
    switch(cond)
    {
    case CMPT_EQUALS:
        if(value1 == value2)
            return true;
        break;
    case CMPT_GREATER:
        if(value1 > value2)
            return true;
        break;
    case CMPT_LESS:
        if(value1 < value2)
            return true;
        break;
    case CMPT_NOTEQ:
        if(value1 != value2)
            return true;
        break;
    default:
        break;
    }

    return false;
}

bool Autocode::CheckConditionD(double value1, double value2, COMPARETYPE cond)
{
    switch(cond)
    {
    case CMPT_EQUALS:
        if(fEqual(value1, value2))
            return true;
        break;
    case CMPT_GREATER:
        if(value1 > value2)
            return true;
        break;
    case CMPT_LESS:
        if(value1 < value2)
            return true;
        break;
    case CMPT_NOTEQ:
        if(!fEqual(value1, value2))
            return true;
        break;
    default:
        break;
    }

    return false;
}

// RUN SELF OPTION
void Autocode::RunSelfOption()
{
    if(GetS(this->MyString).find("once") != std::string::npos)
        this->expire();
}

// REFERENCE OK
bool Autocode::ReferenceOK() const
{
    return (!GetS(this->MyRef).empty());
}



void Autocode::HeartSystem() const
{
    Player_t *sheath = PlayerF::Get(1);

    if(sheath)
    {
        // Don't run for demo or iris
        if(!PlayerF::UsesHearts(sheath))
            return;

        // Detect extra heart pickup and hold displayed hearts at 2
        if(sheath->Hearts > 2)
        {
            gAutoMan.m_Hearts++;
            sheath->Hearts = 2;
            sheath->State = (sheath->State > 2 ? sheath->State : 2);
        }

        if(sheath->Hearts == 2 && gAutoMan.m_Hearts < 2)
            gAutoMan.m_Hearts = 2;

        // Limit tracked max hearts
        if(gAutoMan.m_Hearts > Param2)
            gAutoMan.m_Hearts = (int)Param2;

        // If damaged, take hearts from extra hearts
        if(sheath->Hearts == 1 && gAutoMan.m_Hearts > 2)
        {
            //char* dbg = "HEART SET";
            sheath->Hearts = 2;
            sheath->State = (sheath->State > 2 ? sheath->State : 2);
            gAutoMan.m_Hearts--;
        }
        else if(sheath->Hearts == 1)
        {
            sheath->State = 1;
            gAutoMan.m_Hearts = 1;
        }
        else if(sheath->Hearts == 0)
            gAutoMan.m_Hearts = 0;

        std::stringstream gAutoMan_m_Hearts;
        gAutoMan_m_Hearts << (long long)gAutoMan.m_Hearts;
        // Display life stuff on screen
        Renderer::Get().AddOp(new RenderStringOp(std::string(
                                  std::string("HP: ") + std::string(gAutoMan_m_Hearts.str())
                              )
                              , 3, (float)Target, (float)Param1));
    }//if heartuser
}


void Autocode::LunaControl(LunaControlAct act, int val)
{
    switch(act)
    {
    case LCA_DemoCounter:
        gShowDemoCounter = (val == 1);

        if(gShowDemoCounter && !gEnableDemoCounter && !gEnableDemoCounterByLC) // Initialize the demos counter if wasn't enabled before
        {
            if(!GameMenu && !GameOutro && !BattleMode && !LevelEditor && !TestLevel)
            {
                gEnableDemoCounterByLC = true;
                gDeathCounter.init();
                gDeathCounter.Recount();
            }
        }
        break;

    case LCA_SMBXHUD:
        gSMBXHUDSettings.skip = (val == 1);
        break;

    case LCA_Invalid:
    default:
        return;
    }
}

void Autocode::expire()
{
    Expired = true;
    gAutoMan.m_hasExpired = true;
}

void Autocode::modParam(double &dst, double src, OPTYPE operation)
{
    switch(operation)
    {
    default: // Do nothing
        break;
    case OP_Assign:
        dst = src;
        break;
    case OP_Add:
        dst += src;
        break;
    case OP_Sub:
        dst -= src;
        break;
    case OP_Mult:
        dst *= src;
        break;
    case OP_Div:
        dst /= src;
        break;
    case OP_XOR:
        dst = (int)dst ^ (int)src;
        break;
    }
}

static const std::unordered_map<std::string, AutocodeType> s_commandMap =
{
    {"FilterToSmall", AT_FilterToSmall},
    {"FilterToBig", AT_FilterToBig},
    {"FilterToFire", AT_FilterToFire},
    {"FilterMount", AT_FilterMount},
    {"FilterReservePowerup", AT_FilterReservePowerup},
    {"FilterPlayer", AT_FilterPlayer},

    {"SetHearts", AT_SetHearts},
    {"HeartSystem", AT_HeartSystem},
    {"InfiniteFlying", AT_InfiniteFlying},

    {"ScreenEdgeBuffer", AT_ScreenEdgeBuffer},

    {"ShowText", AT_ShowText},
    {"ShowNPCLifeLeft", AT_ShowNPCLifeLeft},
    {"ShowLevelName", AT_ShowLevelName},
    {"ShowLevelFile", AT_ShowLevelFile},

    {"Trigger", AT_Trigger},
    {"Timer", AT_Timer},
    {"IfNPC", AT_IfNPC},
    {"BlockTrigger", AT_BlockTrigger},
    {"IfCompatMode", AT_IfCompatMode},
    {"IfSpeedRunMode", AT_IfSpeedRunMode},
    {"TriggerRandom", AT_TriggerRandom},
    {"TriggerRandomRange", AT_TriggerRandomRange},
    {"TriggerZone", AT_TriggerZone},
    {"ScreenBorderTrigger", AT_ScreenBorderTrigger},
    {"OnInput", AT_OnInput},
    {"OnCustomCheat", AT_OnCustomCheat},
    {"OnPlayerMem", AT_OnPlayerMem},
    {"OnGlobalMem", AT_OnGlobalMem},
    {"RunCheat", AT_RunCheat},

    {"SetVar", AT_SetVar},
    {"LoadPlayerVar", AT_LoadPlayerVar},
    {"LoadNPCVar", AT_LoadNPCVar},
    {"LoadGlobalVar", AT_LoadGlobalVar},
    {"ShowVar", AT_ShowVar},
    {"IfVar", AT_IfVar},
    {"CompareVar", AT_CompareVar},
    {"BankVar", AT_BankVar},
    {"WriteBank", AT_WriteBank},

    {"LunaControl", AT_LunaControl},

    {"DeleteCommand", AT_DeleteCommand},
    {"ModParam", AT_ModParam},
    {"ChangeTime", AT_ChangeTime},

    {"DeleteEventsFrom", AT_DeleteEventsFrom},
    {"ClearInputString", AT_ClearInputString},

    {"LayerXSpeed", AT_LayerXSpeed},
    {"LayerYSpeed", AT_LayerYSpeed},
    {"AccelerateLayerX", AT_AccelerateLayerX},
    {"AccelerateLayerY", AT_AccelerateLayerY},
    {"DeccelerateLayerX", AT_DeccelerateLayerX},
    {"DeccelerateLayerY", AT_DeccelerateLayerY},
    {"SetAllBlocksID", AT_SetAllBlocksID},
    {"SwapAllBlocks", AT_SwapAllBlocks},
    {"ShowAllBlocks", AT_ShowAllBlocks},
    {"HideAllBlocks", AT_HideAllBlocks},

    {"PushScreenBoundary", AT_PushScreenBoundary},
    {"SnapSectionBounds", AT_SnapSectionBounds},

    {"CyclePlayerRight", AT_CyclePlayerRight},
    {"CyclePlayerLeft", AT_CyclePlayerLeft},

    {"SFX", AT_SFX},
    {"PlaySFX", AT_PlaySFX},
    {"StopSFX", AT_StopSFX},
    {"SFXPreLoad", AT_SFXPreLoad},
    {"SetMusic", AT_SetMusic},
    {"PlayMusic", AT_PlayMusic},

    {"TriggerSMBXEvent", AT_TriggerSMBXEvent},
    {"OnEvent", AT_OnEvent},
    {"CancelSMBXEvent", AT_CancelSMBXEvent},

    {"Kill", AT_Kill},
    {"Hurt", AT_Hurt},

    {"SetHits", AT_SetHits},
    {"NPCMemSet", AT_NPCMemSet},
    {"PlayerMemSet", AT_PlayerMemSet},
    {"ForceFacing", AT_ForceFacing},
    {"MemAssign", AT_MemAssign},
    {"DebugPrint", AT_DebugPrint},
    {"DebugWindow", AT_DebugWindow},

    {"CollisionScan", AT_CollisionScan},

    {"LoadImage", AT_LoadImage},
    {"SpriteBlueprint", AT_SpriteBlueprint},
    {"Attach", AT_Attach},
    {"PlaceSprite", AT_PlaceSprite},

    {"OnPlayerCollide", AT_OnPlayerCollide},
    {"OnPlayerDistance", AT_OnPlayerDistance},
    {"WaitForPlayer", AT_WaitForPlayer},
    {"PlayerHoldingSprite", AT_PlayerHoldingSprite},
    {"RandomComponent", AT_RandomComponent},
    {"RandomComponentRange", AT_RandomComponentRange},
    {"SetSpriteVar", AT_SetSpriteVar},
    {"IfSpriteVar", AT_IfSpriteVar},
    {"IfLunaVar", AT_IfLunaVar},
    {"Die", AT_Die},
    {"Deccelerate", AT_Deccelerate},
    {"AccelToPlayer", AT_AccelToPlayer},
    {"ApplyVariableGravity", AT_ApplyVariableGravity},
    {"PhaseMove", AT_PhaseMove},
    {"BumpMove", AT_BumpMove},
    {"CrashMove", AT_CrashMove},
    {"SetXSpeed", AT_SetXSpeed},
    {"SetYSpeed", AT_SetYSpeed},
    {"SetAlwaysProcess", AT_SetAlwaysProcess},
    {"SetVisible", AT_SetVisible},
    {"SetHitbox", AT_SetHitbox},
    {"TeleportNearPlayer", AT_TeleportNearPlayer},
    {"TeleportTo", AT_TeleportTo},
    {"HarmPlayer", AT_HarmPlayer},
    {"GenerateInRadius", AT_GenerateInRadius},
    {"GenerateAtAngle", AT_GenerateAtAngle},
    {"BasicAnimate", AT_BasicAnimate},
    {"Blink", AT_Blink},
    {"AnimateFloat", AT_AnimateFloat},
    {"TriggerLunaEvent", AT_TriggerLunaEvent},
    {"HarmPlayer", AT_HarmPlayer},
    {"SpriteTimer", AT_SpriteTimer},
    {"SpriteDebug", AT_SpriteDebug},
    {"StaticDraw", AT_StaticDraw},
    {"RelativeDraw", AT_RelativeDraw}
};

AutocodeType Autocode::EnumerizeCommand(char *wbuf, int lineNumber)
{
    if(wbuf)
    {
        char command[100];
        SDL_memset(command, 0, 100 * sizeof(char));

        int success = XTECH_sscanf(wbuf, " %99[^,] ,", command);
        if(!success)
        {
            // Bad or mistyped command?
            std::string line = std::string(wbuf);
            gAutoMan.addError(lineNumber, line, "Syntax error");
            gAutoMan.m_hasExpired = true;
            return AT_Invalid;
        }

        auto cmd = s_commandMap.find(std::string(command));
        if(cmd != s_commandMap.end())
            return cmd->second;
    }

    if(wbuf)
    {
        // Nothing matched. Bad or mistyped command?
        std::string line = std::string(wbuf);
        gAutoMan.addError(lineNumber, line, "Unknown command");
    }

    gAutoMan.m_hasExpired = true;
    return AT_Invalid;
}

SpriteComponent Autocode::GenerateComponent(const Autocode &obj_to_convert)
{
    SpriteComponent comp;
    comp.Init((int)obj_to_convert.Length);
    comp.data1 = obj_to_convert.Target;
    comp.data2 = obj_to_convert.Param1;
    comp.data3 = obj_to_convert.Param2;
    comp.data4 = obj_to_convert.Param3;
    comp.data5 = GetS(obj_to_convert.MyString);
    comp.lookup_code = obj_to_convert.ActiveSection;

    comp.func = Autocode::GetSpriteFunc(obj_to_convert);
    return comp;
}

pfnSprFunc Autocode::GetSpriteFunc(const Autocode &pAC)
{
    switch(pAC.m_Type)
    {
    case AT_OnPlayerCollide:
        return SpriteFunc::OnPlayerCollide;
    case AT_OnPlayerDistance:
        return SpriteFunc::OnPlayerDistance;
    case AT_WaitForPlayer:
        return SpriteFunc::WaitForPlayer;
    case AT_PlayerHoldingSprite:
        return SpriteFunc::PlayerHoldingSprite;
    case AT_RandomComponent:
        return SpriteFunc::RandomComponent;
    case AT_RandomComponentRange:
        return SpriteFunc::RandomComponentRange;
    case AT_SetSpriteVar:
        return SpriteFunc::SetSpriteVar;
    case AT_IfSpriteVar:
        return SpriteFunc::IfSpriteVar;
    case AT_IfLunaVar:
        return SpriteFunc::IfLunaVar;
    case AT_Die:
        return SpriteFunc::Die;
    case AT_Deccelerate:
        return SpriteFunc::Deccelerate;
    case AT_AccelToPlayer:
        return SpriteFunc::AccelToPlayer;
    case AT_ApplyVariableGravity:
        return SpriteFunc::ApplyVariableGravity;
    case AT_PhaseMove:
        return SpriteFunc::PhaseMove;
    case AT_BumpMove:
        return SpriteFunc::BumpMove;
    case AT_CrashMove:
        return SpriteFunc::CrashMove;
    case AT_SetXSpeed:
        return SpriteFunc::SetXSpeed;
    case AT_SetYSpeed:
        return SpriteFunc::SetYSpeed;
    case AT_SetAlwaysProcess:
        return SpriteFunc::SetAlwaysProcess;
    case AT_SetVisible:
        return SpriteFunc::SetVisible;
    case AT_SetHitbox:
        return SpriteFunc::SetHitbox;
    case AT_TeleportNearPlayer:
        return SpriteFunc::TeleportNearPlayer;
    case AT_TeleportTo:
        return SpriteFunc::TeleportTo;
    case AT_GenerateInRadius:
        return SpriteFunc::GenerateInRadius;
    case AT_GenerateAtAngle:
        return SpriteFunc::GenerateAtAngle;
    case AT_BasicAnimate:
        return SpriteFunc::BasicAnimate;
    case AT_Blink:
        return SpriteFunc::Blink;
    case AT_AnimateFloat:
        return SpriteFunc::AnimateFloat;
    case AT_TriggerLunaEvent:
        return SpriteFunc::TriggerLunaEvent;
    case AT_HarmPlayer:
        return SpriteFunc::HarmPlayer;
    case AT_SpriteTimer:
        return SpriteFunc::SpriteTimer;
    case AT_SpriteDebug:
        return SpriteFunc::SpriteDebug;
    default:
        return nullptr;
    }
}

pfnSprDraw Autocode::GetDrawFunc(const Autocode &pAC)
{
    switch(pAC.m_Type)
    {
    case AT_StaticDraw:
        return SpriteFunc::StaticDraw;
    case AT_RelativeDraw:
        return SpriteFunc::RelativeDraw;
    default:
        return nullptr;
    }
}
