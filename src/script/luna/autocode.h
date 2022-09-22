/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2022 Vitaly Novichkov <admin@wohlnet.ru>
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

#pragma once
#ifndef AutoCode_hhh
#define AutoCode_hhh

#include <string>

#include "lunadefs.h"
#include "global_strings.h"

enum LunaControlAct
{
    LCA_Invalid = 0,
    LCA_DemoCounter = 1,
    LCA_SMBXHUD = 2,
};

enum BlueprintAttachType
{
    BPAT_Behavior = 0,
    BPAT_Draw = 1,
    BPAT_Birth = 2,
    BPAT_Death = 3,
};

enum AutocodeType
{
    AT_Invalid = 0,
    AT_FilterToSmall,           //1:        2:              3:              4:          5: Active time  6:
    AT_FilterToBig,             //1:        2:              3:              4:          5: Active time  6:
    AT_FilterToFire,            //1:        2:              3:              4:          5: Active time  6:
    AT_FilterMount,             //1:        2:              3:              4:          5: Active time  6:
    AT_FilterReservePowerup,    //1:        2:              3:              4:          5: Active time  6:
    AT_FilterPlayer,            //1:        2: Filter ID    3: Filter to    4:          5: Active time  6:

    AT_InfiniteFlying,          //1:        2:              3:              4:          5: Active time  6:
    AT_HeartSystem,             //1: X pos  2: Y pos        3: Max hearts   4:          5: Active time  6:

    AT_ScreenEdgeBuffer,        //1: NPC ID 2: UDLR Bordr   3: Buffer space 4:          5: Active time  6:

    AT_ShowText,                //1:        2: Xpos         3: Ypos         4: Font #   5: Active time  6: Text
    AT_ShowNPCLifeLeft,         //1: NpcID  2: Xpos         3: Ypos         4: Section  5: Active time  6: Base health

    AT_Trigger,                 //1: Newsec 2: Event        3:              4:          5: Active time  6:
    AT_Timer,                   //1: Target 2: Effect       3: Display Y/N  4: Repeat?  5: Active time  6:
    AT_IfNPC,                   //1: NpcID  2: Condition    3: Argument     4: Event    5: Active time  6: Option
    AT_BlockTrigger,            //1: NpcID  2: Block Type   3: Argument     4: Event    5: Active time  6: Option
    AT_IfCompatMode,            //1:        2: Condition    3: Mode         4: Event    5: Active time  6:
    AT_IfSpeedRunMode,          //1:        2: Condition    3: Mode         4: Event    5: Active time  6:
    AT_TriggerRandom,           //1: Event  2: Event        3: Event        4: Event    5: Active time  6:
    AT_TriggerRandomRange,      //1: StartEv2: EndEv        3:              4:          5: Active time  6:
    AT_TriggerZone,             //1: Event  2: Top          3: Bottom       4: Left     5: Right        6: Option
    AT_ScreenBorderTrigger,     //1: EventL 2: EventU       3: EventR       4: EventD   5: ActiveTime   6: Inward depth

    AT_OnInput,                 //1:        2: Button       3: First frame? 4: Event    5: Active time  6:
    AT_OnCustomCheat,           //1:        2:              3: Only once?   4: Event    5: Active time  6: Cheat string
    AT_OnPlayerMem,             //1: Offset 2: Value        3: Comparison   4: Event    5: Active time  6: Type (b, w, dw, f, qw/df)
    AT_OnGlobalMem,             //1: Addr   2: Value        3: Comparison   4: Event    5: Active time  6: Type (b, w, dw, f, qw/df)
    AT_RunCheat,                //1:        2:              3:              4:          5: Delay        6: Cheat string

    AT_SetVar,                  //1:        2: Op           3: Value        4:          5: Active time  6:
    AT_LoadPlayerVar,           //1:        2: Offset       3: Op           4:          5: Active time  6: Type (b, w, dw, f, qw/df)
    AT_LoadNPCVar,              //1: Type   2: Offset       3: Op           4: Section  5: Active time  6: Type (b, w, dw, f, qw/df)
    AT_LoadGlobalVar,           //1: Addr   2: Op           3:              4:          5: Active time  6: Type (b, w, dw, f, qw/df)
    AT_IfVar,                   //1:        2: CompareType  3: Value        4: Event    5: Active time  6:
    AT_CompareVar,              //$Var 1    1:  2:CompareType 3:            4: Event    5: ActiveTime   6: Var 2
    AT_ShowVar,                 //1:        2: Xpos         3: Ypos         4: Font #   5: Active time  6: Base text
    AT_BankVar,                 //1:        2:              3:              4:          5: Active time  6: Name of luna var
    AT_WriteBank,               //1:        2:              3:              4:          5:              6:

    AT_LunaControl,             //1: Action 2: Value        3:              4:          5: Active time  6:

    AT_DeleteCommand,           //1:        2:              3:              4:          5: Active time  6: Refernece to cmd
    AT_ModParam,                //1: To mod#2: Value        3: Operation    4:          5: Active time  6: Refernece to cmd
    AT_ChangeTime,              //1:        2: Value        3: Operation    4:          5: Active time  6: Refernece to cmd

    AT_DeleteEventsFrom,        //1: Section2:              3:              4:          5: Active time  6:
    AT_ClearInputString,        //1:        2:              3:              4:          5: Active time  6:

    AT_LayerXSpeed,             //1: Layer #2: Stop after?  3:              4:          5: Active time  6: Speed
    AT_LayerYSpeed,             //1: Layer #2: Stop after?  3:              4:          5: Active time  6: Speed
    AT_AccelerateLayerX,        //1: Layer #2: Max speed    3:              4:          5: Active time  6: Speed
    AT_AccelerateLayerY,        //1: Layer #2: Max speed    3:              4:          5: Active time  6: Speed
    AT_DeccelerateLayerX,       //1: Layer #2: Deccel to    3:              4:          5: Active time  6: Speed
    AT_DeccelerateLayerY,       //1: Layer #2: Deccel to    3:              4:          5: Active time  6: Speed

    AT_SetAllBlocksID,          //1: Type   2: Set to       3:              4:          5: Active time  6:
    AT_SwapAllBlocks,           //1: Type 1 2: Type 2       3:              4:          5: Active time  6:
    AT_ShowAllBlocks,           //1: Type 1 2:              3:              4:          5: Active time  6:
    AT_HideAllBlocks,           //1: Type 1 2:              3:              4:          5: Active time  6:

    AT_PushScreenBoundary,      //1: Section2: UDLR         3:              4:          5: Active time  6: Speed
    AT_SnapSectionBounds,       //1: Section2: Target X     3: Target Y     4:          5: Active time  6:

    AT_CyclePlayerRight,        //1:        2:              3:              4:          5: Active time  6:
    AT_CyclePlayerLeft,         //1:        2:              3:              4:          5: Active time  6:

    AT_SetHearts,               //1:        2: Hearts       3:              4:          5: Active time  6:
    AT_SetHits,                 //1: NpcID  2: Section      3: Hits         4:          5: Active time  6:
    AT_ForceFacing,             //1: NpcID  2: Section      3:              4:          5: Active time  6:

    AT_SFX,                     //1:        2: Index        3:              4:          5: Delay        6:
    AT_PlaySFX,                 //1:        2: Index        3: Loops        4: Volume   5: Delay        6:
    AT_StopSFX,                 //1:        2: Index        3:              4:          5: Delay        6:
    AT_SFXPreLoad,              //1:        2: Index        3:              4:          5: Delay        6:
    AT_SetMusic,                //1: Section2: Music #      3:              4:          5: Active time  6: Optional file name
    AT_PlayMusic,               //1:        2: Section      3:              4:          5: Delay        6:

    AT_TriggerSMBXEvent,        //1:        2: Arg          3:              4:          5: Active time  6: Name of event
    AT_OnEvent,                 //1:        2:              3: Only once?   4: Event    5: Active time  6: Name of event
    AT_CancelSMBXEvent,         //1:        2: Arg          3:              4:          5: Active time  6: Name of event

    AT_Hurt,                    //1: Target 2:              3:              4:          5: Active time  6: Option
    AT_Kill,                    //1: Target 2:              3:              4:          5: Active time  6: Option

    AT_NPCMemSet,               //1: Type   2: Offset       3: Value        4: Op       5: Active time  6: Type (b, w, dw, f, qw/df)
    AT_PlayerMemSet,            //1:        2: Offset       3: Value        4: Op       5: Active time  6: Type (b, w, dw, f, qw/df)
    AT_MemAssign,               //1: Addr   2: Value        3: Operation    4:          5: Active time  6: Type (b, w, dw, f, qw/df)

    AT_DebugPrint,              //1:        2:              3:              4:          5: Active time  6:
    AT_DebugWindow,             //1:        2:              3:              4:          5:              6: Text

    AT_CollisionScan,           //1:        2:              3:              4:          5: Active time  6:

    /// Sprite stuff ///
    AT_LoadImage,               //1: Code   2: TransColor   3:              4:          5:              6: File name
    AT_SpriteBlueprint,         //0: $RefName, others empty
    AT_Attach,                  //0: $Target blueprint,     1: Attach type  2: 3: 4: 5: 6: Source component name
    AT_PlaceSprite,             //1: Type   2: ImgResource  3: Xpos         4: Ypos     5: Sprite time  6:

    // Sprite components //
    AT_OnPlayerCollide = 10000, //1:        2: Player small circle hitbox? 3: 4: Comp   5: Active time  6:
    AT_OnPlayerDistance,        //1: Dist   2: far/close    3:              4: Comp     5: Active time  6:
    AT_WaitForPlayer,           //1: Offset 2: Value        3: Comparison   4: Comp     5: Active time  6: Type (b, w, dw, f, qw/df)
    AT_PlayerHoldingSprite,     //1: Type   2:              3:              4: Comp     5: Active time  6:

    AT_RandomComponent,         //1: choice 2: choice       3: choice       4: choice   5: Active time  6:
    AT_RandomComponentRange,    //1: start  2: end          3:              4:          5: Active time  6:

    AT_SetSpriteVar,            //1:        2: Op           3: Value        4:          5: Active time  6:
    AT_IfSpriteVar,             //1:        2: CompareType  3: Value        4: Event    5: Active time  6:
    AT_IfLunaVar,               //1:        2: CompareType  3: Value        4: Event    5: Active time  6:

    AT_Die,                     //1:        2:              3:              4:          5: Active time  6:

    AT_Deccelerate,             //1: X rate 2: Y rate       3: Minimum      4:          5: Active time  6:
    //AT_Accelerate             //1: X rate 2: Y rate       3: Maximum      4:          5: Active time  6: Option
    AT_AccelToPlayer,           //1: X rate 2: Y rate       3: Maximum      4:          5: Active time  6:
    AT_ApplyVariableGravity,    //1:        2: 0=x 1=y      3:              4:          5: Active time  6: Name of lunadll variable

    AT_TeleportNearPlayer,      //1: Radius 2:              3:              4:          5: Active time  6:
    AT_TeleportTo,              //1: x      2: y            3:              4:          5: Active time  6:

    AT_PhaseMove,               //1:        2:              3:              4:          5: Active time  6:
    AT_BumpMove,                //1:        2: NRGLoss%     3:              4:          5: Active time  6:
    AT_CrashMove,               //1:        2:              3:              4:          5: Active time  6:

    AT_SetXSpeed,               //1: Speed  2:              3:              4:          5: Active time  6:
    AT_SetYSpeed,               //1: Speed  2:              3:              4:          5: Active time  6:
    AT_SetAlwaysProcess,        //1: Off/On
    AT_SetVisible,              //1: Off/On
    AT_SetHitbox,               //1: LeftOff2: Top offset   3: Width        4: Height   5: Active time  6: option

    AT_BasicAnimate,            //1: Height 2: Speed
    AT_Blink,                   //1: Speed  2: Reverse      3:              4:          5: Active time  6:
    AT_AnimateFloat,            //1: Speed  2: X magnitude  3: Y magnitude

    AT_TriggerLunaEvent,        //1: Event  2:              3:              4:          5: Active time  6:
    AT_HarmPlayer,              //1:        2:              3:              4:          5: Active time  6:

    AT_GenerateInRadius,        //1:        2: ImgResource  3: Radius       4: SprTime  5: Active time  6: Sprite blueprint name
    AT_GenerateAtAngle,         //1:        2: ImgResource  3: Speed        4: SprTime  5: Active time  6: Sprite blueprint name

    AT_SpriteTimer,             //1:        2:              3: Repeat       4: Comp     5: Active time  6:

    AT_SpriteDebug,             //1:        2:              3: Repeat       4: Comp     5: Active time  6:

    AT_StaticDraw,
    AT_RelativeDraw
};

enum AutocodePredicate
{
    AP_Invalid = 0,
    AP_Death,
    AP_Hurt
};

enum AC_Conditional
{
    AC_Invalid = 0,
    AC_Exists,
    AC_DoesNotExist
};

struct SpriteComponent; // forward dec

// An autocode event
class Autocode
{

public:
    // Ctors
    Autocode();
    Autocode(AutocodeType, double Target, double p1, double p2, double p3,
             const stringindex_t &p4, double Length, int Section, const stringindex_t &VarRef);
    Autocode(const Autocode &o);
    ~Autocode() = default;

    Autocode &operator=(const Autocode &o);

    void Do(bool init);
    static void DoPredicate(int target, int predicate);

    static bool NPCConditional(int NPCID, int condition);
    static bool CheckConditionI(int value1, int value2, COMPARETYPE cond);
    static bool CheckConditionD(double value1, double value2, COMPARETYPE cond);

    static AutocodeType EnumerizeCommand(char *wbuf, int lineNumber = -1);
    static SpriteComponent GenerateComponent(const Autocode &obj_to_convert); // Convert an autocode object to a sprite component
    static pfnSprFunc GetSpriteFunc(const Autocode &pAC);                     // Get sprite function address from AC object, or NULL
    static pfnSprDraw GetDrawFunc(const Autocode &pAC);                       // Get draw function address from AC object, or NULL

    /// Command functions ///
    void HeartSystem() const;
    static void LunaControl(LunaControlAct act, int val);

    /// Members ///
    AutocodeType m_Type = AT_Invalid;

    double Target = 0.0;                       // arg 1 "Target"
    double Param1 = 0.0;                       // arg 2 "Param 1"
    double Param2 = 0.0;                       // arg 3 "Param 2"
    double Param3 = 0.0;                       // arg 4 "Param 3"
    double Length = 0.0;                       // arg 5 "Length"
    stringindex_t MyString = STRINGINDEX_NONE; // arg 6 "string"
    stringindex_t MyRef = STRINGINDEX_NONE;    // Optional arg 0 value

    double m_OriginalTime = 0.0;
    int ActiveSection = 0;          // Section to be active in, or custom event ID if > 1000
    FIELDTYPE ftype = FT_INVALID;
    bool Activated = false;             // False for custom event blueprints
    bool Expired = false;

    void expire();

    //SpriteComponent* comp;

private:
    static void modParam(double &dst, double src, OPTYPE operation);
    void SelfTick();
    void RunSelfOption(); // activate the string portion of this code on self
    bool ReferenceOK() const; // check if this object has a valid reference (not empty)
};

#endif // AutoCode_hhh
