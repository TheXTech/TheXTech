/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <Logger/logger.h>
#include <fmt_format_ne.h>

#include "autocode.h"
#include "autocode_manager.h"
#include "lunamisc.h"
#include "sprite_funcs.h"
#include "csprite.h"
#include "globals.h"
#include "player.h"
#include "graphics.h"
#include "sound.h"
#include "layers.h"
#include "core/msgbox.h"
#include "main/cheat_code.h"
#include "lunarender.h"
#include "lunaplayer.h"
#include "lunalevel.h"
#include "lunanpc.h"
#include "lunablock.h"
#include "lunacell.h"
#include "lunaspriteman.h"
#include "lunainput.h"
#include "renderop_string.h"
#include "mememu.h"


namespace LayerF
{

Layer_t *Get(int layerIdx)
{
    if(layerIdx < 0 || layerIdx > numLayers)
        return nullptr;
    return &Layer[layerIdx];
}

// Stop a layer
void Stop(Layer_t *layer)
{
    layer->EffectStop = false;
    layer->SpeedX = 0.0001f;
    layer->SpeedY = 0.0001f;
}

void SetYSpeed(Layer_t *layer, float setY)
{
    setY = (setY == 0 ?  0.0001f : setY);
    layer->SpeedY = setY;
    layer->EffectStop = true;
}

void SetXSpeed(Layer_t *layer, float setX)
{
    setX = (setX == 0 ?  0.0001f : setX);
    layer->SpeedX = setX;
    layer->EffectStop = true;
}

} // LayerF



static FIELDTYPE StrToFieldtype(std::string string)
{
    string.erase(string.find_last_not_of(" \n\r\t") + 1);
    if(string == "b")
        return FT_BYTE;
    else if(string == "s")
        return FT_WORD;
    else if(string == "w")
        return FT_WORD;
    else if(string == "dw")
        return FT_DWORD;
    else if(string == "f")
        return FT_FLOAT;
    else if(string == "df")
        return FT_DFLOAT;

    return FT_BYTE;
}

static void PrintSyntaxError(std::string errored_line)
{
    static int errors = 0;
    errors += 25;
    //    RenderStringOp* render_str = new RenderStringOp();
    //    render_str->m_FontType = 2;
    //    render_str->m_FramesLeft = 440;
    //    render_str->m_String = errored_line;
    //    render_str->m_String += L"- SYNTAX ERROR";
    //    render_str->m_X = 125;
    //    render_str->m_Y = (float)(errors % 600);
    //    Renderer::Get().AddOp(render_str);
    pLogWarning("Autocode Syntax Error: %s", errored_line.c_str());
}



Autocode::Autocode()
{
    m_Type = AT_Invalid;
}

Autocode::Autocode(AutocodeType _Type, double _Target, double _p1, double _p2, double _p3,
                   std::string _p4, double _Length, int _Section, std::string _VarRef)
{
    m_Type = _Type;
    Target = _Target;
    Param1 = _p1;
    Param2 = _p2;
    Param3 = _p3;
    Length = _Length;
    MyString = _p4;
    MyRef = _VarRef;
    m_OriginalTime = _Length;
    ftype = FT_INVALID;
    Activated = true;
    Expired = false;
    //comp = NULL;

    // Adjust section
    ActiveSection = (_Section < 1000 ? --_Section : _Section);
    Activated = (_Section < 1000 ? true : false);
}

Autocode *Autocode::MakeCopy()
{
    Autocode *newcode = new Autocode();

    newcode->Activated = Activated;
    newcode->ActiveSection = ActiveSection;
    newcode->Expired = Expired;
    newcode->ftype = ftype;
    newcode->Length = Length;
    newcode->MyString = MyString;
    newcode->MyRef = MyRef;
    newcode->m_OriginalTime = m_OriginalTime;
    newcode->m_Type = m_Type;
    newcode->Param1 = Param1;
    newcode->Param2 = Param2;
    newcode->Param3 = Param3;
    newcode->Target = Target;
    //newcode->comp = NULL;

    return newcode;
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
        init = ((uint8_t)ActiveSection == (uint8_t)0xFE ? true : false);

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
            Player_t *demo = PlayerF::Get(1);
            if(demo != 0)
            {
                if(demo->Character == Param1)
                {
                    if(Param2 > 0 && Param2 < 6)
                        demo->Character = (int)Param2;
                }
            }
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

            // Get all target NPCs in section into a list
            std::list<NPC_t *> npcs;
            NpcF::FindAll((int)Target, demo->Section, &npcs);

            if(npcs.size() > 0)
            {
                // Loop over list of NPCs
                for(std::list<NPC_t *>::iterator iter = npcs.begin(), end = npcs.end(); iter != end; ++iter)
                {
                    NPC_t *npc = *iter;
                    switch((int)Param1)
                    {
                    default:
                    case 0: // UP
                    {
                        //double* pCamera = vScreenY;
                        double top = -vScreenY[1];
                        if(npc->Location.Y < top + Param2)
                            npc->Location.Y = (top + Param2) + 1;
                        break;
                    }

                    case 1:  // DOWN
                    {
                        //double* pCamera = GM_CAMERA_Y;
                        double bot = -vScreenY[1] + ScreenH;
                        if(npc->Location.Y > bot - Param2)
                            npc->Location.Y = (bot - Param2) - 1;
                        break;
                    }

                    case 2: // LEFT
                    {
                        //double* pCamera = GM_CAMERA_X;
                        double left = -vScreenX[1];
                        if(npc->Location.X < left + Param2)
                            npc->Location.X = (left + Param2) + 1;
                        break;
                    }

                    case 3: // RIGHT
                    {
                        //double* pCamera = GM_CAMERA_X;
                        double rt = -vScreenX[1] + ScreenW;
                        if(npc->Location.Y > rt - Param2)
                            npc->Location.Y = (rt - Param2) - 1;
                        break;
                    }
                    }
                }
            }
            break;
        }



        // SHOW TEXT
        case AT_ShowText:
            Renderer::Get().AddOp(new RenderStringOp(MyString, (int)Param3, (float)Param1, (float)Param2));
            break;

        // SHOW NPC LIFE LEFT
        case AT_ShowNPCLifeLeft:
        {
            int base_health = SDL_atoi(MyString.data());
            NPC_t *npc = NpcF::GetFirstMatch((int)Target, (int)Param3 - 1);
            if(npc != NULL)
            {
                float hits = *(((float *)((&(*(uint8_t *)npc)) + 0x148)));
                Renderer::Get().AddOp(new RenderStringOp(std::to_string((long long)(base_health - hits)), 3, (float)Param1, (float)Param2));
            }
            else
                Renderer::Get().AddOp(new RenderStringOp("?", 3, (float)Param1, (float)Param2));
            break;
        }

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
                    if(MyString.length() > 0)
                    {
                        //char* dbg = "CUSTOM SOUND PLAY DBG";
                        std::string full_path = FileNamePath + FileName + "/" + MyString;
                        PlayExtSound(full_path);
                    }

                }
                Expired = true;
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
                if(MyString.length() >= 5)
                    CustomMusic[sec] = MyString;
            }
            break;
        }

        case AT_PlayMusic:
        {
            if(Length <= 1)   // Play once when delay runs out
            {
                StartMusic((int)Param1 - 1);
                Expired = true;
            }
            break;
        }

        // EVENTS & BRANCHES
        case AT_Trigger:
        {
            Expired = true;
            gAutoMan.ActivateCustomEvents((int)Target, (int)Param1);
            break;
        }

        case AT_Timer:
            if(Param2) // Display timer?
            {
                Renderer::Get().AddOp(new RenderStringOp("TIMER", 3, 600, 27));
                Renderer::Get().AddOp(new RenderStringOp(std::to_string((long long)Length / 60), 3, 618, 48));
            }

            if(Length == 1 || Length == 0)
            {
                if(Length == 1)
                    Expired = true;

                DoPredicate((int)Target, (int)Param1);

                // Reset time?
                if(Param3)
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

            if(NPCConditional((int)Target, (int)Param1))
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

        case AT_TriggerRandom:
        {
            int choice = rand() % 4;
            switch(choice)
            {
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
            int choice = iRand(diff);
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
            if(MyString.length() > 0)
                depth = SDL_atoi(MyString.c_str());

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
                ftype = StrToFieldtype(MyString);
            }
//            uint8_t *ptr = (uint8_t *)demo;
//            ptr += (int)Target; // offset
            bool triggered = CheckMem(demo, Target, Param1, (COMPARETYPE)(int)Param2, ftype);
            if(triggered)
                gAutoMan.ActivateCustomEvents(0, (int)Param3);
            break;
        }

        case AT_OnGlobalMem:
        {
            if(ftype == FT_INVALID)
            {
                ftype = FT_BYTE;
                ftype = StrToFieldtype(MyString);
            }

            bool triggered = CheckMem((int)Target, Param1, (COMPARETYPE)(int)Param2, ftype);
            if(triggered)
                gAutoMan.ActivateCustomEvents(0, (int)Param3);
            break;
        }

        // USER VARS
        case AT_SetVar:
        {
            if(ReferenceOK())
                gAutoMan.VarOperation(MyRef, Param2, (OPTYPE)(int)Param1);
            else
                gAutoMan.VarOperation(MyString, Param2, (OPTYPE)(int)Param1);
            break;
        }

        case AT_LoadPlayerVar:
        {
            if(!this->ReferenceOK() || Param1 > (0x184 * 99))
                break;

            if(ftype == FT_INVALID)
            {
                ftype = FT_BYTE;
                ftype = StrToFieldtype(MyString);
            }

            // Get the memory
            //uint8_t *ptr = (uint8_t *)demo;
            //ptr += (int)Param1; // offset
            double gotval = GetMem(demo, (int)Param1, ftype);

            // Perform the load/add/sub/etc operation on the banked variable using the ref as the name
            gAutoMan.VarOperation(MyRef, gotval, (OPTYPE)(int)Param2);

            break;
        }

        case AT_LoadNPCVar:
        {
            if(!this->ReferenceOK() || Param1 > (0x158))
                break;
            if(ftype == FT_INVALID)
            {
                ftype = FT_BYTE;
                ftype = StrToFieldtype(MyString);
            }

            NPC_t *pFound_npc = NpcF::GetFirstMatch((int)Target, (int)Param3);
            if(pFound_npc != nullptr)
            {
                double gotval = GetMem(pFound_npc, (int)Param1, ftype);
                gAutoMan.VarOperation(MyRef, gotval, (OPTYPE)(int)Param2);
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
                    ftype = StrToFieldtype(MyString);
                }

                // byte *ptr = (byte *)(int)Target;
                double gotval = GetMem((int)Target, ftype);
                gAutoMan.VarOperation(MyRef, gotval, (OPTYPE)(int)Param1);
            }
            break;
        }

        case AT_IfVar:
        {
            if(!ReferenceOK())
            {
                InitIfMissing(&gAutoMan.m_UserVars, MyString, 0);// Initalize var if not existing
            }
            double varval;
            if(ReferenceOK())
                varval = gAutoMan.m_UserVars[MyRef];
            else
                varval = gAutoMan.m_UserVars[MyString];

            // Check if the value meets the criteria and activate event if so
            switch((COMPARETYPE)(int)Param1)
            {
            case CMPT_EQUALS:
                if(varval == Param2)
                    gAutoMan.ActivateCustomEvents(0, (int)Param3);
                break;
            case CMPT_GREATER:
                if(varval > Param2)
                    gAutoMan.ActivateCustomEvents(0, (int)Param3);
                break;
            case CMPT_LESS:
                if(varval < Param2)
                    gAutoMan.ActivateCustomEvents(0, (int)Param3);
                break;
            case CMPT_NOTEQ:
                if(varval != Param2)
                    gAutoMan.ActivateCustomEvents(0, (int)Param3);
                break;
            default:
                break;
            }
            break;
        }

        case AT_CompareVar:
        {
            if(ReferenceOK())
            {
                COMPARETYPE compare_type = (COMPARETYPE)(int)Param1;
                if(true)
                {
                    InitIfMissing(&gAutoMan.m_UserVars, MyString, 0);
                    InitIfMissing(&gAutoMan.m_UserVars, MyRef, 0);

                    double var1 = gAutoMan.m_UserVars[MyRef];
                    double var2 = gAutoMan.m_UserVars[MyString];

                    switch(compare_type)
                    {
                    case CMPT_EQUALS:
                        if(var1 == var2)
                            gAutoMan.ActivateCustomEvents(0, (int)Param3);
                        break;
                    case CMPT_GREATER:
                        if(var1 > var2)
                            gAutoMan.ActivateCustomEvents(0, (int)Param3);
                        break;
                    case CMPT_LESS:
                        if(var1 < var2)
                            gAutoMan.ActivateCustomEvents(0, (int)Param3);
                        break;
                    case CMPT_NOTEQ:
                        if(var1 != var2)
                            gAutoMan.ActivateCustomEvents(0, (int)Param3);
                        break;
                    default:
                        break;
                    }
                }
            }
            break;
        }

        case AT_ShowVar:
        {
            if(ReferenceOK())
            {
                std::string str = std::to_string((long double)gAutoMan.GetVar(MyRef));
                if(MyString.length() > 0)
                    str = MyString + str;
                Renderer::Get().AddOp(new RenderStringOp(str, (int)Param3, (float)Param1, (float)Param2));
            }
            break;
        }

        case AT_BankVar:
        {
//            if(MyString.length() > 0)
//                gSavedVarBank.SetVar(MyString, gAutoMan.GetVar(MyString));
            break;
        }

        case AT_WriteBank:
        {
//            gSavedVarBank.WriteBank();
            break;
        }


        // LUNA CONTROL
        case AT_LunaControl:
        {
            LunaControl((LunaControlAct)(int)Target, (int)Param1);
            break;
        }


        // DELETE COMMAND
        case AT_DeleteCommand:
        {
            gAutoMan.DeleteEvent(MyString);
            break;
        }

        // MOD PARAM
        case AT_ModParam:
        {
            if(Target < 6 && Target > 0)
            {
                Autocode *coderef = 0;
                coderef = gAutoMan.GetEventByRef(MyString);
                if(coderef != 0)
                {
                    switch((int)Target)
                    {
                    case 1:
                        modParam(coderef->Target, Param1, (OPTYPE)(int)Param2);
                        break;
                    case 2:
                        modParam(coderef->Param1, Param1, (OPTYPE)(int)Param2);
                        break;
                    case 3:
                        modParam(coderef->Param2, Param1, (OPTYPE)(int)Param2);
                        break;
                    case 4:
                        modParam(coderef->Param3, Param1, (OPTYPE)(int)Param2);
                        break;
                    case 5:
                        modParam(coderef->Length, Param1, (OPTYPE)(int)Param2);
                        break;
                    }
                }
            }
            break;
        }

        // ChangeTime
        case AT_ChangeTime:
        {
            Autocode *coderef = 0;
            coderef = gAutoMan.GetEventByRef(MyString);
            if(coderef != 0)
                modParam(coderef->Length, Param1, (OPTYPE)(int)Param2);
            break;
        }

        // INPUT BUFFER STUFF
        case AT_OnCustomCheat:
        {
            if(cheats_contains(MyString))
            {
                gAutoMan.ActivateCustomEvents(0, (int)Param3);
                cheats_clearBuffer();
                if(Param2 != 0)
                    this->Expired = true;
            }
            break;
        }

        case AT_ClearInputString:
        {
            //wchar_t* dbg = L"ClearInputString debug";
            cheats_clearBuffer();
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
                LayerF::SetXSpeed(layer, (float)SDL_atof(MyString.c_str()));
                if(Length == 1 && Param1)
                    LayerF::SetXSpeed(layer, 0.0001f);
            }
            break;
        }

        case AT_LayerYSpeed:
        {
            Layer_t *layer = LayerF::Get((int)Target);
            if(layer)
            {
                LayerF::SetYSpeed(layer, (float)SDL_atof(MyString.c_str()));
                if(Length == 1 && Param1)
                    LayerF::SetYSpeed(layer, 0.0001f);
            }
            break;
        }

        case AT_AccelerateLayerX:
        {
            Layer_t *layer = LayerF::Get((int)Target);
            if(layer)
            {
                float accel = (float)SDL_atof(MyString.c_str());
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
                float accel = (float)SDL_atof(MyString.c_str());
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
                float deccel = (float)SDL_atof(MyString.c_str());
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
                float deccel = (float)SDL_atof(MyString.c_str());
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
                LevelF::PushSectionBoundary((int)Target - 1, (int)Param1, SDL_atof(MyString.c_str()));
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
            Player_t *demo = PlayerF::Get(1);
            if(demo != 0)
                NpcF::AllFace((int)Target, (int)Param1 - 1, demo->Location.SpeedX);
            break;
        }

        case AT_TriggerSMBXEvent:
        {
            ProcEvent(MyString, (int)Param1);
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
                KillPlayer(tempint);
            RunSelfOption();
            break;
        }

        // NPC MEMORY SET
        case AT_NPCMemSet:
        {
            if(ftype == FT_INVALID)
            {
                ftype = FT_BYTE;
                ftype = StrToFieldtype(MyString);
            }

            // Assign the mem
            if(ReferenceOK())   // Use referenced var as value
            {
                double gotval = gAutoMan.GetVar(MyRef);
                NpcF::MemSet((int)Target, (int)Param1, gotval, (OPTYPE)(int)Param3, ftype);
            }
            else   // Use given value as value
            {
                NpcF::MemSet((int)Target, (int)Param1, Param2, (OPTYPE)(int)Param3, ftype); // NPC ID, offset in obj, value, op, field type
            }

            break;
        }

        // PLAYER MEMORY SET
        case AT_PlayerMemSet:
        {
            if(ftype == FT_INVALID)
            {
                ftype = FT_BYTE;
                ftype = StrToFieldtype(MyString);
            }
            if(ReferenceOK())
            {
                double gotval = gAutoMan.GetVar(MyRef);
                PlayerF::MemSet((int)Param1, gotval, (OPTYPE)(int)Param3, ftype);
            }
            else
                PlayerF::MemSet((int)Param1, Param2, (OPTYPE)(int)Param3, ftype);
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
                    ftype = StrToFieldtype(MyString);
                }
                if(ReferenceOK())
                {
                    double gotval = gAutoMan.GetVar(MyRef);
                    MemAssign((int)Target, gotval, (OPTYPE)(int)Param2, ftype);
                }
                else
                    MemAssign((int)Target, Param1, (OPTYPE)(int)Param2, ftype);
            }
            break;
        }

        // DEBUG
        case AT_DebugPrint:
        {
            Renderer::Get().AddOp(new RenderStringOp(fmt::format_ne("LUNADLL (TheXTech) VERSION-{0}", LUNA_VERSION), 3, 50, 250));
            //Renderer::Get().SafePrint(, 3, 340, 250);
            Renderer::Get().AddOp(new RenderStringOp(fmt::format_ne("GLOBL-{0}", gAutoMan.m_GlobalCodes.size()), 3, 50, 300));
            Renderer::Get().AddOp(new RenderStringOp(fmt::format_ne("INIT -{0}", gAutoMan.m_InitAutocodes.size()), 3, 50, 330));
            Renderer::Get().AddOp(new RenderStringOp(fmt::format_ne("CODES-{0}", gAutoMan.m_Autocodes.size()), 3, 50, 360));
            Renderer::Get().AddOp(new RenderStringOp(fmt::format_ne("QUEUE-{0}", gAutoMan.m_CustomCodes.size()), 3, 50, 390));
            Renderer::Get().AddOp(new RenderStringOp(fmt::format_ne("SPRITE-{0}", gSpriteMan.CountSprites()), 3, 50, 420));
            Renderer::Get().AddOp(new RenderStringOp(fmt::format_ne("BLPRNT-{0}", gSpriteMan.CountBlueprints()), 3, 50, 450));
            Renderer::Get().AddOp(new RenderStringOp(fmt::format_ne("COMP-{0}", gSpriteMan.m_ComponentList.size()), 3, 50, 480));

            int buckets = 0, cells = 0, objs = 0;
            gCellMan.CountAll(&buckets, &cells, &objs);
            Renderer::Get().AddOp(new RenderStringOp(fmt::format_ne("BCO-{0} {1} {2}", buckets, cells, objs), 3, 50, 510));

            std::list<CellObj> cellobjs;
            gCellMan.GetObjectsOfInterest(&cellobjs, demo->Location.X, demo->Location.Y, (int)demo->Location.Width, (int)demo->Location.Height);
            Renderer::Get().AddOp(new RenderStringOp(fmt::format_ne("NEAR-{0}", cellobjs.size()), 3, 50, 540));
            break;
        }

        case AT_DebugWindow:
        {
            XMsgBox::simpleMsgBox(AbstractMsgBox_t::MESSAGEBOX_INFORMATION, "LunaDLL debug message", MyString);
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
                Renderer::Get().LoadBitmapResource(MyString, (int)Target, (int)Param1);
            Expired = true;
            break;
        }

        case AT_SpriteBlueprint:
        {
            if(ReferenceOK())
            {
                CSprite *blueprint = new CSprite();
                gSpriteMan.AddBlueprint(MyRef.c_str(), blueprint);
            }

            Expired = true;
            break;
        }

        case AT_Attach:
        {
            //char* dbg = "!!! ATTACH DEBUG !!!";
            if(ReferenceOK() && MyString.length() > 0)
            {
                if(gSpriteMan.m_SpriteBlueprints.find(MyRef) != gSpriteMan.m_SpriteBlueprints.end()) // BLueprint exists
                {
                    CSprite *pSpr = gSpriteMan.m_SpriteBlueprints[MyRef];                   // Get blueprint
                    Autocode *pComponent = gAutoMan.GetEventByRef(MyString);                // Get autocode containing component
                    if(pComponent != NULL)
                    {
                        switch((BlueprintAttachType)(int)Target)
                        {
                        case BPAT_Behavior:
                            pSpr->AddBehaviorComponent(GenerateComponent(pComponent));
                            break;
                        case BPAT_Draw:
                            pSpr->AddDrawComponent(GetDrawFunc(pComponent));
                            break;
                        case BPAT_Birth:
                            pSpr->AddBirthComponent(GenerateComponent(pComponent));
                            break;
                        case BPAT_Death:
                            pSpr->AddDeathComponent(GenerateComponent(pComponent));
                            break;
                        default:
                            break;
                        }
                    }
                }
            }

            Expired = true;
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
            req.str = MyString;
            gSpriteMan.InstantiateSprite(&req, false);

            Expired = true;
            break;
        }

        }//switch

    }//section
}

// SELF TICK
void Autocode::SelfTick()
{
    if(Length == 1)
        Expired = true;
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
    AutocodePredicate pred = (AutocodePredicate)predicate;
    short tempint = 1;

    switch(pred)
    {
    // DEATH PREDICATE
    case AP_Hurt:
        PlayerHurt(tempint);
        break;

    case AP_Death:
        KillPlayer(tempint);
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
        break;

    case AC_Exists:
    {
        return (NpcF::GetFirstMatch(target, -1) == nullptr ? false : true);
        break;
    }

    case AC_DoesNotExist:
    {
        return (NpcF::GetFirstMatch(target, -1) == nullptr ? true : false);
        break;
    }
    }
}

// RUN SELF OPTION
void Autocode::RunSelfOption()
{
    if(this->MyString.find("once") != std::string::npos)
        this->Expired = true;
}

// REFERENCE OK
bool Autocode::ReferenceOK()
{
    return (this->MyRef.length() >= 1);
}



void Autocode::HeartSystem()
{
    Player_t *sheath = PlayerF::Get(1);

    if(sheath != 0)
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
    UNUSED(val);

    switch(act)
    {
    case LCA_DemoCounter:
        break;

    case LCA_SMBXHUD:
//        if(val == 1)
//            gSMBXHUDSettings.skip = true;
//        else
//            gSMBXHUDSettings.skip = false;
        break;

    case LCA_Invalid:
    default:
        return;
    }
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


AutocodeType Autocode::EnumerizeCommand(char *wbuf)
{
    if(wbuf)
    {
        char command[100];
        SDL_memset(command, 9, 100 * sizeof(char));
        int success = SDL_sscanf(wbuf, " %99[^,] ,", command);
        if(!success)
        {
            // Bad or mistyped command?
            std::string line = std::string(wbuf);
            if(line.size() > 10)
                PrintSyntaxError(line);
            return AT_Invalid;
        }

        if(SDL_strcmp(command, "FilterToSmal") == 0)
            return AT_FilterToSmall;

        if(SDL_strcmp(command, "FilterToBig") == 0)
            return AT_FilterToBig;

        if(SDL_strcmp(command, "FilterToFire") == 0)
            return AT_FilterToFire;

        if(SDL_strcmp(command, "FilterMount") == 0)
            return AT_FilterMount;

        if(SDL_strcmp(command, "FilterReservePowerup") == 0)
            return AT_FilterReservePowerup;

        if(SDL_strcmp(command, "FilterPlayer") == 0)
            return AT_FilterPlayer;


        if(SDL_strcmp(command, "SetHearts") == 0)
            return AT_SetHearts;

        if(SDL_strcmp(command, "HeartSystem") == 0)
            return AT_HeartSystem;

        if(SDL_strcmp(command, "InfiniteFlying") == 0)
            return AT_InfiniteFlying;


        if(SDL_strcmp(command, "ScreenEdgeBuffer") == 0)
            return AT_ScreenEdgeBuffer;


        if(SDL_strcmp(command, "ShowText") == 0)
            return AT_ShowText;

        if(SDL_strcmp(command, "ShowNPCLifeLeft") == 0)
            return AT_ShowNPCLifeLeft;


        if(SDL_strcmp(command, "Trigger") == 0)
            return AT_Trigger;

        if(SDL_strcmp(command, "Timer") == 0)
            return AT_Timer;

        if(SDL_strcmp(command, "IfNPC") == 0)
            return AT_IfNPC;

        if(SDL_strcmp(command, "BlockTrigger") == 0)
            return AT_BlockTrigger;

        if(SDL_strcmp(command, "TriggerRandom") == 0)
            return AT_TriggerRandom;

        if(SDL_strcmp(command, "TriggerRandomRange") == 0)
            return AT_TriggerRandomRange;

        if(SDL_strcmp(command, "TriggerZone") == 0)
            return AT_TriggerZone;

        if(SDL_strcmp(command, "ScreenBorderTrigger") == 0)
            return AT_ScreenBorderTrigger;

        if(SDL_strcmp(command, "OnInput") == 0)
            return AT_OnInput;

        if(SDL_strcmp(command, "OnCustomCheat") == 0)
            return AT_OnCustomCheat;

        if(SDL_strcmp(command, "OnPlayerMem") == 0)
            return AT_OnPlayerMem;

        if(SDL_strcmp(command, "OnGlobalMem") == 0)
            return AT_OnGlobalMem;


        if(SDL_strcmp(command, "SetVar") == 0)
            return AT_SetVar;

        if(SDL_strcmp(command, "LoadPlayerVar") == 0)
            return AT_LoadPlayerVar;

        if(SDL_strcmp(command, "LoadNPCVar") == 0)
            return AT_LoadNPCVar;

        if(SDL_strcmp(command, "LoadGlobalVar") == 0)
            return AT_LoadGlobalVar;

        if(SDL_strcmp(command, "ShowVar") == 0)
            return AT_ShowVar;

        if(SDL_strcmp(command, "IfVar") == 0)
            return AT_IfVar;

        if(SDL_strcmp(command, "CompareVar") == 0)
            return AT_CompareVar;

        if(SDL_strcmp(command, "BankVar") == 0)
            return AT_BankVar;

        if(SDL_strcmp(command, "WriteBank") == 0)
            return AT_WriteBank;


        if(SDL_strcmp(command, "LunaContro") == 0)
            return AT_LunaControl;


        if(SDL_strcmp(command, "DeleteCommand") == 0)
            return AT_DeleteCommand;

        if(SDL_strcmp(command, "ModParam") == 0)
            return AT_ModParam;

        if(SDL_strcmp(command, "ChangeTime") == 0)
            return AT_ChangeTime;


        if(SDL_strcmp(command, "DeleteEventsFrom") == 0)
            return AT_DeleteEventsFrom;

        if(SDL_strcmp(command, "ClearInputString") == 0)
            return AT_ClearInputString;


        if(SDL_strcmp(command, "LayerXSpeed") == 0)
            return AT_LayerXSpeed;

        if(SDL_strcmp(command, "LayerYSpeed") == 0)
            return AT_LayerYSpeed;

        if(SDL_strcmp(command, "AccelerateLayerX") == 0)
            return AT_AccelerateLayerX;

        if(SDL_strcmp(command, "AccelerateLayerY") == 0)
            return AT_AccelerateLayerY;

        if(SDL_strcmp(command, "DeccelerateLayerX") == 0)
            return AT_DeccelerateLayerX;

        if(SDL_strcmp(command, "DeccelerateLayerY") == 0)
            return AT_DeccelerateLayerY;

        if(SDL_strcmp(command, "SetAllBlocksID") == 0)
            return AT_SetAllBlocksID;

        if(SDL_strcmp(command, "SwapAllBlocks") == 0)
            return AT_SwapAllBlocks;

        if(SDL_strcmp(command, "ShowAllBlocks") == 0)
            return AT_ShowAllBlocks;

        if(SDL_strcmp(command, "HideAllBlocks") == 0)
            return AT_HideAllBlocks;


        if(SDL_strcmp(command, "PushScreenBoundary") == 0)
            return AT_PushScreenBoundary;

        if(SDL_strcmp(command, "SnapSectionBounds") == 0)
            return AT_SnapSectionBounds;


        if(SDL_strcmp(command, "CyclePlayerRight") == 0)
            return AT_CyclePlayerRight;

        if(SDL_strcmp(command, "CyclePlayerLeft") == 0)
            return AT_CyclePlayerLeft;


        if(SDL_strcmp(command, "SFX") == 0)
            return AT_SFX;

        if(SDL_strcmp(command, "SetMusic") == 0)
            return AT_SetMusic;

        if(SDL_strcmp(command, "PlayMusic") == 0)
            return AT_PlayMusic;


        if(SDL_strcmp(command, "TriggerSMBXEvent") == 0)
            return AT_TriggerSMBXEvent;


        if(SDL_strcmp(command, "Kil") == 0)
            return AT_Kill;

        if(SDL_strcmp(command, "Hurt") == 0)
            return AT_Hurt;


        if(SDL_strcmp(command, "SetHits") == 0)
            return AT_SetHits;

        if(SDL_strcmp(command, "NPCMemSet") == 0)
            return AT_NPCMemSet;

        if(SDL_strcmp(command, "PlayerMemSet") == 0)
            return AT_PlayerMemSet;

        if(SDL_strcmp(command, "ForceFacing") == 0)
            return AT_ForceFacing;

        if(SDL_strcmp(command, "MemAssign") == 0)
            return AT_MemAssign;

        if(SDL_strcmp(command, "DebugPrint") == 0)
            return AT_DebugPrint;

        if(SDL_strcmp(command, "DebugWindow") == 0)
            return AT_DebugWindow;


        if(SDL_strcmp(command, "CollisionScan") == 0)
            return AT_CollisionScan;


        if(SDL_strcmp(command, "LoadImage") == 0)
            return AT_LoadImage;

        if(SDL_strcmp(command, "SpriteBlueprint") == 0)
            return AT_SpriteBlueprint;

        if(SDL_strcmp(command, "Attach") == 0)
            return AT_Attach;

        if(SDL_strcmp(command, "PlaceSprite") == 0)
            return AT_PlaceSprite;


        // Sprite Component section
        if(SDL_strcmp(command, "OnPlayerCollide") == 0)
            return AT_OnPlayerCollide;

        if(SDL_strcmp(command, "OnPlayerDistance") == 0)
            return AT_OnPlayerDistance;

        if(SDL_strcmp(command, "WaitForPlayer") == 0)
            return AT_WaitForPlayer;

        if(SDL_strcmp(command, "PlayerHoldingSprite") == 0)
            return AT_PlayerHoldingSprite;

        if(SDL_strcmp(command, "RandomComponent") == 0)
            return AT_RandomComponent;

        if(SDL_strcmp(command, "RandomComponentRange") == 0)
            return AT_RandomComponentRange;

        if(SDL_strcmp(command, "SetSpriteVar") == 0)
            return AT_SetSpriteVar;

        if(SDL_strcmp(command, "IfSpriteVar") == 0)
            return AT_IfSpriteVar;

        if(SDL_strcmp(command, "IfLunaVar") == 0)
            return AT_IfLunaVar;

        if(SDL_strcmp(command, "Die") == 0)
            return AT_Die;

        if(SDL_strcmp(command, "Deccelerate") == 0)
            return AT_Deccelerate;

        if(SDL_strcmp(command, "AccelToPlayer") == 0)
            return AT_AccelToPlayer;

        if(SDL_strcmp(command, "ApplyVariableGravity") == 0)
            return AT_ApplyVariableGravity;

        if(SDL_strcmp(command, "PhaseMove") == 0)
            return AT_PhaseMove;

        if(SDL_strcmp(command, "BumpMove") == 0)
            return AT_BumpMove;

        if(SDL_strcmp(command, "CrashMove") == 0)
            return AT_CrashMove;

        if(SDL_strcmp(command, "SetXSpeed") == 0)
            return AT_SetXSpeed;

        if(SDL_strcmp(command, "SetYSpeed") == 0)
            return AT_SetYSpeed;

        if(SDL_strcmp(command, "SetAlwaysProcess") == 0)
            return AT_SetAlwaysProcess;

        if(SDL_strcmp(command, "SetVisible") == 0)
            return AT_SetVisible;

        if(SDL_strcmp(command, "SetHitbox") == 0)
            return AT_SetHitbox;

        if(SDL_strcmp(command, "TeleportNearPlayer") == 0)
            return AT_TeleportNearPlayer;

        if(SDL_strcmp(command, "TeleportTo") == 0)
            return AT_TeleportTo;

        if(SDL_strcmp(command, "HarmPlayer") == 0)
            return AT_HarmPlayer;

        if(SDL_strcmp(command, "GenerateInRadius") == 0)
            return AT_GenerateInRadius;

        if(SDL_strcmp(command, "GenerateAtAngle") == 0)
            return AT_GenerateAtAngle;

        if(SDL_strcmp(command, "BasicAnimate") == 0)
            return AT_BasicAnimate;

        if(SDL_strcmp(command, "Blink") == 0)
            return AT_Blink;

        if(SDL_strcmp(command, "AnimateFloat") == 0)
            return AT_AnimateFloat;

        if(SDL_strcmp(command, "TriggerLunaEvent") == 0)
            return AT_TriggerLunaEvent;

        if(SDL_strcmp(command, "HarmPlayer") == 0)
            return AT_HarmPlayer;

        if(SDL_strcmp(command, "SpriteTimer") == 0)
            return AT_SpriteTimer;

        if(SDL_strcmp(command, "SpriteDebug") == 0)
            return AT_SpriteDebug;

        if(SDL_strcmp(command, "StaticDraw") == 0)
            return AT_StaticDraw;

        if(SDL_strcmp(command, "RelativeDraw") == 0)
            return AT_RelativeDraw;
    }

    if(wbuf)
    {
        // Nothing matched. Bad or mistyped command?
        std::string line = std::string(wbuf);
        if(line.size() > 10)
            PrintSyntaxError(line);
    }

    return AT_Invalid;
}

SpriteComponent Autocode::GenerateComponent(Autocode *obj_to_convert)
{
    SpriteComponent comp;
    comp.Init((int)obj_to_convert->Length);
    comp.data1 = obj_to_convert->Target;
    comp.data2 = obj_to_convert->Param1;
    comp.data3 = obj_to_convert->Param2;
    comp.data4 = obj_to_convert->Param3;
    comp.data5 = obj_to_convert->MyString;
    comp.lookup_code = obj_to_convert->ActiveSection;

    comp.func = Autocode::GetSpriteFunc(obj_to_convert);
    return comp;
}

pfnSprFunc Autocode::GetSpriteFunc(Autocode *pAC)
{
    switch(pAC->m_Type)
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
        return NULL;
    }
}

pfnSprDraw Autocode::GetDrawFunc(Autocode *pAC)
{
    switch(pAC->m_Type)
    {
    case AT_StaticDraw:
        return SpriteFunc::StaticDraw;
    case AT_RelativeDraw:
        return SpriteFunc::RelativeDraw;
    default:
        return NULL;
    }
}
