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

#include "editor/editor_strings.h"

EditorContent g_editorStrings;

void initEditorStrings()
{
    g_editorStrings.pickBlockContents1 = "Pick block";
    g_editorStrings.pickBlockContents2 = "contents";

    g_editorStrings.npcInContainer = "In";
    g_editorStrings.npcInertNice = "Nice";
    g_editorStrings.npcStuckStop = "Stop";
    g_editorStrings.npcAbbrevGen = "Gen";

    g_editorStrings.npcAiIs = "AI: {0}";
    g_editorStrings.npcAiTarget = "Target";
    g_editorStrings.npcAiJump = "Jump";
    g_editorStrings.npcAiLeap = "Leap";
    g_editorStrings.npcAiSwim = "Swim";
    g_editorStrings.npcAiLR = "LR";
    g_editorStrings.npcAiUD = "UD";

    g_editorStrings.npcCustomAi = "Custom AI:";

    g_editorStrings.npcUse1_0Ai = "Use 1.0 AI?";
    g_editorStrings.npcTooltipExpandSection = "Expand section";

    g_editorStrings.npcGenHeader = "Generator Settings";
    g_editorStrings.npcGenDirection = "Direction";
    g_editorStrings.npcGenEffectIs = "Effect: {0}";
    g_editorStrings.npcGenEffectWarp = "Warp";
    g_editorStrings.npcGenEffectShoot = "Shoot";
    g_editorStrings.npcGenDelayIsMs = "Delay: {0} ms";

    g_editorStrings.wordNPC = "NPC";
    g_editorStrings.wordNPCGenitive = "NPC";

    g_editorStrings.wordEvent = "Event";
    g_editorStrings.wordEventGenitive = "Event";

    g_editorStrings.wordCoins = "Coins";

    g_editorStrings.wordEnabled = "Enabled";
    g_editorStrings.wordText = "Text";
    g_editorStrings.phraseTextOf = "{0} Text";
    g_editorStrings.phraseSectionIndex = "Section {0}";
    g_editorStrings.phraseRadiusIndex = "Radius {0}";

    g_editorStrings.eventsHeader = "Events:";
    g_editorStrings.eventsLetterActivate = "A:";
    g_editorStrings.eventsLetterDeath = "D:";
    g_editorStrings.eventsLetterTalk = "T:";
    g_editorStrings.eventsLetterLayerClear = "L:";

    g_editorStrings.labelLayer = "Layer:";
    g_editorStrings.labelAbbrevAttLayer = "Att: ";

    g_editorStrings.listWarpTransitNames = {"NONE", "SCROLL", "FADE", "CIRCLE", "FLIP (H)", "FLIP (V)"};
}
