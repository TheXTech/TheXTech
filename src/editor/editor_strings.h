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

#pragma once
#ifndef EDITOR_STRINGS_H
#define EDITOR_STRINGS_H

#include <string>

struct EditorContent
{
    std::string pickBlockContents1;
    std::string pickBlockContents2;

    std::string npcInContainer;
    std::string npcInertNice;
    std::string npcStuckStop;
    std::string npcAbbrevGen;

    std::string npcAiIs;
    std::string npcAiTarget;
    std::string npcAiJump;
    std::string npcAiLeap;
    std::string npcAiSwim;
    std::string npcAiLR;
    std::string npcAiUD;

    std::string npcCustomAi;

    std::string npcUse1_0Ai;
    std::string npcTooltipExpandSection;

    std::string wordNPC;
    std::string wordNPCGenitive;

    std::string wordEvent;
    std::string wordEventGenitive;

    std::string wordText;
    std::string phraseTextOf;
    std::string phraseSectionIndex;
    std::string phraseRadiusIndex;

    std::string eventsHeader;
    std::string eventsLetterActivate;
    std::string eventsLetterDeath;
    std::string eventsLetterTalk;
    std::string eventsLetterLayerClear;

    std::string labelLayer;
    std::string labelAbbrevAttLayer;
};

extern EditorContent g_editorStrings;

void initEditorStrings();

#endif // EDITOR_STRINGS_H
