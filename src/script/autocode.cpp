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

#include "autocode.h"
#include "globals.h"
#include "player.h"
#include "graphics.h"
#include "sound.h"
#include "layers.h"
#include "core/msgbox.h"
#include "main/cheat_code.h"


struct LunaRect
{
    int left;
    int top;
    int right;
    int bottom;
};

namespace PlayerF
{

// FILTER FUNCS
void FilterToFire(Player_t *player)
{
    if(player->State > 3)
        player->State = 3;
}

void FilterToBig(Player_t *player)
{
    if(player->State > 2)
        player->State = 2;
}

void FilterToSmall(Player_t *player)
{
    if(player->State > 1)
        player->State = 1;
}

void FilterReservePowerup(Player_t *player)
{
    player->HeldBonus = 0;
}

void FilterMount(Player_t *player)
{
    player->MountType = 0;
    player->Mount = 0;
}

Player_t *Get(int num)
{
    if(num > numPlayers || num < 1)
        return nullptr;

    return &Player[num];
}

void InfiniteFlying(int player)
{
    Player_t* demo = Get(player);

    if(demo != 0)
        demo->FlyCount = 50;
}

bool UsesHearts(Player_t *p)
{
    return p->Character != 1 && p->Character != 2;
}

// GET SCREEN POSITION
LunaRect GetScreenPosition(Player_t *player)
{
//    double* pCameraY = (double*)GM_CAMERA_Y;
//    double* pCameraX = (double*)GM_CAMERA_X;
    double cam_y = -vScreenY[1];
    double cam_x = -vScreenX[1];
//    double cam_d = cam_y + 600;
//    double cam_r = cam_x + 800;

    LunaRect ret_rect;
    ret_rect.left = (player->Location.X - cam_x);
    ret_rect.top = (player->Location.Y - cam_y);
    ret_rect.right = ret_rect.left + player->Location.Width;
    ret_rect.bottom = ret_rect.top + player->Location.Height;
    return ret_rect;
}

} // PlayerF


namespace NpcF
{

void FindAll(int ID, int section, std::list<NPC_t*>* return_list)
{
    bool anyID = (ID == -1 ? true : false);
    bool anySec = (section == -1 ? true : false);
    NPC_t* thisnpc = NULL;

    for(int i = 0; i < numNPCs; i++)
    {
        thisnpc = &NPC[i];
        if(thisnpc->Type == ID || anyID)
        {
            if(thisnpc->Section == section || anySec)
                return_list->push_back(thisnpc);
        }
    }
}

// GET FIRST MATCH
NPC_t* GetFirstMatch(int ID, int section)
{
    bool anyID = (ID == -1 ? true : false);
    bool anySec = (section == -1 ? true : false);
    NPC_t* thisnpc = nullptr;

    for(int i = 0; i < numNPCs; i++)
    {
        thisnpc = &NPC[i];
        if(thisnpc->Type == ID || anyID)
        {
            if(thisnpc->Section == section || anySec)
                return thisnpc; //matched
        }
    }

    return nullptr; //not matched
}

} // NpcF


namespace LayerF
{

Layer_t *Get(int layerIdx)
{
    if(layerIdx < 0 || layerIdx > numLayers)
        return nullptr;
    return &Layer[layerIdx];
}

// Stop a layer
void Stop(Layer_t* layer)
{
    layer->EffectStop = false;
    layer->SpeedX = 0.0001f;
    layer->SpeedY = 0.0001f;
}

void SetYSpeed(Layer_t* layer, float setY)
{
    setY = (setY == 0 ?  0.0001f : setY);
    layer->SpeedY = setY;
    layer->EffectStop = true;
}

void SetXSpeed(Layer_t* layer, float setX)
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

Autocode* Autocode::MakeCopy()
{
    Autocode* newcode = new Autocode();

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
void Autocode::Do(bool init) {

    // Is it expired?
    if(Expired || !Activated)
        return;

    // Make sure game is in OK state to run
    Player_t* demo = PlayerF::Get(1);

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
        case AT_FilterToSmall: {
            PlayerF::FilterToSmall(demo);
            break;
                            }
        case AT_FilterToBig: {
            PlayerF::FilterToBig(demo);
            break;
                          }
        case AT_FilterToFire: {
            PlayerF::FilterToFire(demo);
            break;
                           }
        case AT_FilterMount: {
            PlayerF::FilterMount(demo);
            break;
                          }
        case AT_FilterReservePowerup: {
            PlayerF::FilterReservePowerup(demo);
            break;
                                   }

        case AT_FilterPlayer:
        {
            Player_t* demo = PlayerF::Get(1);
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
        case AT_ScreenEdgeBuffer: {
            //char* dbg = "SCREEN EDGE DBG";

            // Get all target NPCs in section into a list
            std::list<NPC_t*> npcs;
            NpcF::FindAll((int)Target, demo->Section, &npcs);

            if(npcs.size() > 0)
            {
                // Loop over list of NPCs
                for(std::list<NPC_t*>::iterator iter = npcs.begin(), end = npcs.end(); iter != end; ++iter)
                {
                    NPC_t* npc = *iter;
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
        case AT_ShowNPCLifeLeft: {
            int base_health = SDL_atoi(MyString.data());
            NPC_t* npc = NpcF::GetFirstMatch((int)Target, (int)Param3 - 1);
            if(npc != NULL) {
                float hits = *(((float*)((&(*(byte*)npc)) + 0x148)));
                Renderer::Get().AddOp(new RenderStringOp(std::to_wstring((long long)(base_health - hits)), 3, (float)Param1, (float)Param2));
            } else {
                Renderer::Get().AddOp(new RenderStringOp(L"?", 3, (float)Param1, (float)Param2));
            }
            break;
                            }

        // AUDIO
        case AT_SFX: {
            if(this->Length <= 1) // Play once when delay runs out
            {
                // Play built in sound
                if(Param1 > 0)
                {
                    PlaySound((int)Param1);
                }
                else
                {
                    // Sound from level folder
                    if(MyString.length() > 0)
                    {
                        //char* dbg = "CUSTOM SOUND PLAY DBG";
                        std::wstring full_path = getCustomFolderPath() + MyString;
                        PlaySound(full_path.c_str(), 0, SND_FILENAME | SND_ASYNC);
                    }

                }
                Expired = true;
            }
            break;
                     }

        case AT_SetMusic: {
            SMBXSound::SetMusic((int)Param1, (int)Target - 1);

            if(MyString.length() >= 5)
                SMBXSound::SetMusicPath((int)Target-1, MyString);
            break;
                           }

        case AT_PlayMusic: {
            if(Length <= 1) { // Play once when delay runs out
                SMBXSound::PlayMusic((int)Param1 - 1, true);
                Expired = true;
            }
            break;
                           }

        // EVENTS & BRANCHES
        case AT_Trigger: {
            Expired = true;
            gAutoMan.ActivateCustomEvents((int)Target, (int)Param1);
            break;
                         }

        case AT_Timer:
            if(Param2) // Display timer?
            {
                Renderer::Get().AddOp(new RenderStringOp(L"TIMER", 3, 600, 27));
                Renderer::Get().AddOp(new RenderStringOp(std::to_wstring((long long)Length / 60), 3, 618, 48));
            }

            if(Length == 1 || Length == 0)
            {
                if(Length == 1)
                    Expired = true;

                DoPredicate((int)Target, (int)Param1);

                // Reset time?
                if(Param3) {
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
                if(Blocks::IsPlayerTouchingType((int)Param1, (int)Param2, demo)) {
                    RunSelfOption();
                    gAutoMan.ActivateCustomEvents(0, (int)Param3);
                }
            }
            break;
        }

        case AT_TriggerRandom:
        {
            int choice = rand() % 4;
            switch(choice) {
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
            if(ftype == FT_INVALID) {
                ftype = FT_BYTE;
                ftype = StrToFieldtype(MyString);
            }
            byte* ptr = (byte*)demo;
            ptr += (int)Target; // offset
            bool triggered = CheckMem((int)ptr, Param1, (COMPARETYPE)(int)Param2, ftype);
            if(triggered) {
                gAutoMan.ActivateCustomEvents(0, (int)Param3);
            }
            break;
        }

        case AT_OnGlobalMem: {
            if(ftype == FT_INVALID) {
                ftype = FT_BYTE;
                ftype = StrToFieldtype(MyString);
            }
            bool triggered = CheckMem((int)Target, Param1, (COMPARETYPE)(int)Param2, ftype);
            if(triggered) {
                gAutoMan.ActivateCustomEvents(0, (int)Param3);
            }
            break;
                             }

        // USER VARS
        case AT_SetVar:
        {
            if(ReferenceOK()) {
                gAutoMan.VarOperation(MyRef, Param2, (OPTYPE)(int)Param1);
            }
            else {
                gAutoMan.VarOperation(MyString, Param2, (OPTYPE)(int)Param1);
            }
            break;
        }

        case AT_LoadPlayerVar:
        {
            if(!this->ReferenceOK() || Param1 > (0x184 * 99))
                break;
            if(ftype == FT_INVALID) {
                ftype = FT_BYTE;
                ftype = StrToFieldtype(MyString);
            }

            // Get the memory
            byte* ptr = (byte*)demo;
            ptr += (int)Param1; // offset
            double gotval = GetMem((int)ptr, ftype);

            // Perform the load/add/sub/etc operation on the banked variable using the ref as the name
            gAutoMan.VarOperation(MyRef, gotval, (OPTYPE)(int)Param2);

            break;
        }

        case AT_LoadNPCVar:
        {
            if(!this->ReferenceOK() || Param1 > (0x158))
                break;
            if(ftype == FT_INVALID) {
                ftype = FT_BYTE;
                ftype = StrToFieldtype(MyString);
            }

            NPC_t* pFound_npc = NpcF::GetFirstMatch((int)Target, (int)Param3);
            if(pFound_npc != nullptr)
            {
                uint8_t* ptr = (uint8_t*)pFound_npc;
                ptr += (int)Param1;
                double gotval = GetMem((int)ptr, ftype);
                gAutoMan.VarOperation(MyRef, gotval, (OPTYPE)(int)Param2);
            }

            break;
        }

        case AT_LoadGlobalVar:
        {
            if(Target >= GM_BASE && Param1 <=  GM_END && ReferenceOK()) {
                if(ftype == FT_INVALID) {
                    ftype = FT_BYTE;
                    ftype = StrToFieldtype(MyString);
                }

                byte* ptr = (byte*)(int)Target;
                double gotval = GetMem((int)ptr, ftype);

                gAutoMan.VarOperation(MyRef, gotval, (OPTYPE)(int)Param1);
            }
            break;
        }

        case AT_IfVar:
        {
            if(!ReferenceOK()) {
                InitIfMissing(&gAutoMan.m_UserVars, MyString, 0);// Initalize var if not existing
            }
            double varval;
            if(ReferenceOK()) {
                varval = gAutoMan.m_UserVars[MyRef];
            }
            else {
                varval = gAutoMan.m_UserVars[MyString];
            }

            // Check if the value meets the criteria and activate event if so
            switch((COMPARETYPE)(int)Param1) {
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
                if(true) {
                    InitIfMissing(&gAutoMan.m_UserVars, MyString, 0);
                    InitIfMissing(&gAutoMan.m_UserVars, MyRef, 0);

                    double var1 = gAutoMan.m_UserVars[MyRef];
                    double var2 = gAutoMan.m_UserVars[MyString];

                    switch(compare_type) {
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
            if(ReferenceOK()) {
                std::wstring str = std::to_wstring((long double)gAutoMan.GetVar(MyRef));
                if(MyString.length() > 0)
                    str = MyString + str;
                Renderer::Get().AddOp(new RenderStringOp(str, (int)Param3, (float)Param1, (float)Param2));
            }
            break;
        }

        case AT_BankVar:
        {
            if(MyString.length() > 0) {
                gSavedVarBank.SetVar(MyString, gAutoMan.GetVar(MyString));
            }
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
                Autocode* coderef = 0;
                coderef = gAutoMan.GetEventByRef(MyString);
                if(coderef !=0)
                {
                    switch((int)Target)
                    {
                    case 1:
                        MemAssign((int)&(coderef->Target), Param1, (OPTYPE)(int)Param2, FT_DWORD);
                        break;
                    case 2:
                        MemAssign((int)&(coderef->Param1), Param1, (OPTYPE)(int)Param2, FT_DWORD);
                        break;
                    case 3:
                        MemAssign((int)&(coderef->Param2), Param1, (OPTYPE)(int)Param2, FT_DWORD);
                        break;
                    case 4:
                        MemAssign((int)&(coderef->Param3), Param1, (OPTYPE)(int)Param2, FT_DWORD);
                        break;
                    case 5:
                        MemAssign((int)&(coderef->Length), Param1, (OPTYPE)(int)Param2, FT_DWORD);
                        break;
                    }
                }
            }
            break;
        }

        // ChangeTime
        case AT_ChangeTime:
        {
            Autocode* coderef = 0;
            coderef = gAutoMan.GetEventByRef(MyString);
            if(coderef !=0) {
                MemAssign((int)&coderef->Length, Param1, (OPTYPE)(int)Param2, FT_DFLOAT);
            }
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
            Layer_t* layer = LayerF::Get((int)Target);
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
            Layer_t* layer = LayerF::Get((int)Target);
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
            Layer_t* layer = LayerF::Get((int)Target);
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
            Layer_t* layer = LayerF::Get((int)Target);
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
            Layer_t* layer = LayerF::Get((int)Target);
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
                else if (layer->SpeedX < 0)
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
            Layer_t* layer = LayerF::Get((int)Target);
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
                else if (layer->SpeedY < 0)
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
            Blocks::SetAll((int)Target, (int)Param1);
            break;
        }

        case AT_SwapAllBlocks:
        {
            Blocks::SwapAll((int)Target, (int)Param1);
            break;
        }

        case AT_ShowAllBlocks:
        {
            Blocks::ShowAll((int)Target);
            break;
        }

        case AT_HideAllBlocks:
        {
            Blocks::HideAll((int)Target);
            break;
        }


        case AT_PushScreenBoundary:
        {
            if(Target > 0 && Target < 22 && Param1 >= 0 && Param1 < 5) {
                Level::PushSectionBoundary((int)Target - 1, (int)Param1, SDL_atof(MyString.c_str()));
            }
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
                    Level::SetSectionBounds((int)Target - 1, Param1, Param2, Param1 + 800, Param2 + 600);
                //}
            }
            break;
        }

        // PLAYER CYCLE
        case AT_CyclePlayerRight:
        {
            Player::CycleRight(demo);
            break;
        }

        case AT_CyclePlayerLeft:
        {
            Player::CycleLeft(demo);
            break;
        }


        // SET HITS
        case AT_SetHits:
        {
            NPC::AllSetHits((int)Target, (int)Param1 - 1, (float)Param2);
            break;
        }

        // FORCE FACING
        case AT_ForceFacing:
        {
            Player_t* demo = PlayerF::Get(1);
            if(demo != 0) {
                NpcF::AllFace((int)Target, (int)Param1 - 1, demo->Location.SpeedX);
            }
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
                Player::Harm(tempint);
            RunSelfOption();
            break;
        }

        case AT_Kill:
        {
            short tempint = 1;
            if(Target == 0)
                Player::Kill(tempint);
            RunSelfOption();
            break;
        }

        // NPC MEMORY SET
        case AT_NPCMemSet:
        {
            if(ftype == FT_INVALID) {
                ftype = FT_BYTE;
                ftype = StrToFieldtype(MyString);
            }

            // Assign the mem
            if(ReferenceOK()) { // Use referenced var as value
                double gotval = gAutoMan.GetVar(MyRef);
                NPC::MemSet((int)Target, (int)Param1, gotval, (OPTYPE)(int)Param3, ftype);
            }
            else { // Use given value as value
                NPC::MemSet((int)Target, (int)Param1, Param2, (OPTYPE)(int)Param3, ftype); // NPC ID, offset in obj, value, op, field type
            }

            break;
        }

        // PLAYER MEMORY SET
        case AT_PlayerMemSet: {
            if(ftype == FT_INVALID) {
                ftype = FT_BYTE;
                ftype = StrToFieldtype(MyString);
            }
            if(ReferenceOK()) {
                double gotval = gAutoMan.GetVar(MyRef);
                Player::MemSet((int)Param1, gotval, (OPTYPE)(int)Param3, ftype);
            }
            else {
                Player::MemSet((int)Param1, Param2, (OPTYPE)(int)Param3, ftype);
            }
            break;
                              }


        // MEM ASSIGN
        case AT_MemAssign: {
            if(Target >= GM_BASE && Param1 <=  GM_END) {
                if(ftype == FT_INVALID) {
                    ftype = FT_BYTE;
                    ftype = StrToFieldtype(MyString);
                }
                if(ReferenceOK()) {
                    double gotval = gAutoMan.GetVar(MyRef);
                    MemAssign((int)Target, gotval, (OPTYPE)(int)Param2, ftype);
                }
                else {
                    MemAssign((int)Target, Param1, (OPTYPE)(int)Param2, ftype);
                }
            }
            break;
                           }

        // DEBUG
        case AT_DebugPrint:
        {
            Renderer::Get().AddOp(new RenderStringOp(L"LUNADLL (WITH LUA) VERSION-" + std::to_wstring((long long)LUNA_VERSION), 3, 50, 250));
            //Renderer::Get().SafePrint(, 3, 340, 250);

            Renderer::Get().AddOp(new RenderStringOp(L"GLOBL-" + std::to_wstring((long long)gAutoMan.m_GlobalCodes.size()), 3, 50, 300));

            Renderer::Get().AddOp(new RenderStringOp(L"INIT -" + std::to_wstring((long long)gAutoMan.m_InitAutocodes.size()), 3, 50, 330));

            Renderer::Get().AddOp(new RenderStringOp(L"CODES-" + std::to_wstring((long long)gAutoMan.m_Autocodes.size()), 3, 50, 360));

            Renderer::Get().AddOp(new RenderStringOp(L"QUEUE-" + std::to_wstring((long long)gAutoMan.m_CustomCodes.size()), 3, 50, 390));

            Renderer::Get().AddOp(new RenderStringOp(L"SPRITE-" + std::to_wstring((long long)gSpriteMan.CountSprites()), 3, 50, 420));

            Renderer::Get().AddOp(new RenderStringOp(L"BLPRNT-" + std::to_wstring((long long)gSpriteMan.CountBlueprints()), 3, 50, 450));

            Renderer::Get().AddOp(new RenderStringOp(L"COMP-" + std::to_wstring((long long)gSpriteMan.m_ComponentList.size()), 3, 50, 480));

            int buckets = 0, cells = 0, objs = 0;
            gCellMan.CountAll(&buckets, &cells, &objs);
            Renderer::Get().AddOp(new RenderStringOp(L"BCO-" + std::to_wstring((long long)buckets) + L" "
                                    + std::to_wstring((long long)cells) + L" "
                                    + std::to_wstring((long long)objs), 3, 50, 510));

            std::list<CellObj> cellobjs;
            gCellMan.GetObjectsOfInterest(&cellobjs, demo->momentum.x, demo->momentum.y, (int)demo->momentum.width, (int)demo->momentum.height);
            Renderer::Get().AddOp(new RenderStringOp(L"NEAR-" + std::to_wstring((long long)cellobjs.size()), 3, 50, 540));

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
                CSprite* blueprint = new CSprite();
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
                    CSprite* pSpr = gSpriteMan.m_SpriteBlueprints[MyRef];					// Get blueprint
                    Autocode* pComponent = gAutoMan.GetEventByRef(MyString);				// Get autocode containing component
                    if(pComponent != NULL) {
                        switch((BlueprintAttachType)(int)Target) {
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
            //1: Type	2: ImgResource	3: Xpos			4: Ypos		5:				6: Extra
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
void Autocode::DoPredicate(int target, int predicate) {

    // Activate custom event?
    if(predicate >= 1000 && predicate < 100000)
    {
        gAutoMan.ActivateCustomEvents(target, predicate);
        return;
    }

    // Else, do predicate
    AutocodePredicate pred = (AutocodePredicate)predicate;
    short tempint = 1;

    switch (pred)
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

    switch((AC_Conditional)cond) {

    case AC_Invalid:
    default:
        return ret;
        break;

    case AC_Exists: {
        return (NpcF::GetFirstMatch(target, -1) == nullptr ? false : true);
        break;
                    }

    case AC_DoesNotExist: {
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
    Player_t* sheath = PlayerF::Get(1);

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
        {
            gAutoMan.m_Hearts = 0;
        }

        std::stringstream gAutoMan_m_Hearts;
        gAutoMan_m_Hearts << (long long)gAutoMan.m_Hearts;
        // Display life stuff on screen
        Renderer::Get().AddOp(new RenderStringOp(std::wstring(
                                  std::string("HP: ") + std::string(gAutoMan_m_Hearts.str())
                                  )
                              ,3, (float)Target, (float)Param1));
    }//if heartuser
}


void Autocode::LunaControl(LunaControlAct act, int val)
{
    switch(act) {

    case LCA_DemoCounter:
        break;

    case LCA_SMBXHUD:
        if(val == 1)
            gSMBXHUDSettings.skip = true;
        else
            gSMBXHUDSettings.skip = false;
        break;

    case LCA_Invalid:
    default:
        return;
    }
}
