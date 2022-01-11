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

#include "lunaplayer.h"
#include "lunanpc.h"
#include "globals.h"
#include "mememu.h"


Player_t *PlayerF::Get(int num)
{
    if(num > numPlayers || num < 1)
        return nullptr;

    return &Player[num];
}

void PlayerF::MemSet(int offset, double value, OPTYPE operation, FIELDTYPE ftype)
{
    //    char* dbg =  "PLAYER MEM SET";
    if(ftype == FT_INVALID || offset > (0x184 * 99) || offset < 0)
        return;
    Player_t *pPlayer = Get(1);
    MemAssign(pPlayer, offset, value, operation, ftype);
}


bool PlayerF::PressingDown(Player_t *player)
{
    return player->Controls.Down;
}

bool PlayerF::PressingUp(Player_t *player)
{
    return player->Controls.Up;
}

bool PlayerF::PressingLeft(Player_t *player)
{
    return player->Controls.Left;
}

bool PlayerF::PressingRight(Player_t *player)
{
    return player->Controls.Right;
}

bool PlayerF::PressingJump(Player_t *player)
{
    return player->Controls.Jump;
}

bool PlayerF::PressingRun(Player_t *player)
{
    return player->Controls.Run;
}

bool PlayerF::PressingSEL(Player_t *player)
{
    return player->Controls.Drop;
}



void PlayerF::FilterToFire(Player_t *player)
{
    if(player->State > 3)
        player->State = 3;
}

void PlayerF::FilterToBig(Player_t *player)
{
    if(player->State > 2)
        player->State = 2;
}

void PlayerF::FilterToSmall(Player_t *player)
{
    if(player->State > 1)
        player->State = 1;
}

void PlayerF::FilterReservePowerup(Player_t *player)
{
    player->HeldBonus = 0;
}

void PlayerF::FilterMount(Player_t *player)
{
    player->MountType = 0;
    player->Mount = 0;
}


void PlayerF::InfiniteFlying(int player)
{
    Player_t *demo = Get(player);

    if(demo != 0)
        demo->FlyCount = 50;
}

bool PlayerF::UsesHearts(Player_t *p)
{
    return p->Character != 1 && p->Character != 2;
}

LunaRect PlayerF::GetScreenPosition(Player_t *player)
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

void PlayerF::CycleRight(Player_t *player)
{
    player->Character++;
    if(player->Character > 5)
        player->Character = 1;
}

void PlayerF::CycleLeft(Player_t *player)
{
    player->Character--;
    if(player->Character <= 0)
        player->Character = 5;
}

bool PlayerF::IsHoldingSpriteType(Player_t *player, int NPC_ID)
{
    if(player->HoldingNPC != 0)
    {
        NPC_t *npc = NpcF::Get(player->HoldingNPC);
        if(npc->Type == NPC_ID)
            return true;
    }

    return false;
}
