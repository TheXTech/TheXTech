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
#include <vector>

struct EditorContent
{
    std::string phraseAreYouSure;
    std::string pageBlankOfBlank;

    std::string pickBlockContents1;
    std::string pickBlockContents2;

    std::string blockLetterWidth;
    std::string blockLetterHeight;
    std::string blockCanBreak;
    std::string blockTooltipCanBreak;
    std::string blockSlick;
    std::string blockInvis;
    std::string blockInside;

    std::string npcInContainer;
    std::string npcInertNice;
    std::string npcStuckStop;
    std::string npcAbbrevGen;
    std::string npcPropertyActive;
    std::string npcPropertyAttachSurface;
    std::string npcPropertyFacing;

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

    std::string npcGenHeader;
    std::string npcGenDirection;
    std::string npcGenEffectIs;
    std::string npcGenEffectWarp;
    std::string npcGenEffectShoot;

    std::string wordNPC;
    std::string wordNPCGenitive;

    std::string wordEvent;
    std::string wordEventGenitive;
    std::string phraseTypeLabelEvent;

    std::string wordCoins;

    std::string wordEnabled;
    std::string wordText;
    std::string wordInstant;
    std::string phraseTextOf;
    std::string phraseSectionIndex;
    std::string phraseRadiusIndex;
    std::string phraseCountMore;
    std::string phraseDelayIsMs;

    std::string letterUp;
    std::string letterDown;
    std::string letterLeft;
    std::string letterRight;

    std::string toggleMagicBlock;

    std::string testMagicHand;
    std::string testChar;
    std::string testPower;
    std::string testBoot;
    std::string testPet;

    std::string eventsHeader;

    std::string eventsLetterActivate;
    std::string eventsLetterDeath;
    std::string eventsLetterTalk;
    std::string eventsLetterLayerClear;
    std::string eventsLetterHit;
    std::string eventsLetterDestroy;
    std::string eventsLetterEnter;

    std::string eventsLabelNext;
    std::string eventsLabelActivate;
    std::string eventsLabelDeath;
    std::string eventsLabelTalk;
    std::string eventsLabelLayerClear;
    std::string eventsLabelHit;
    std::string eventsLabelDestroy;
    std::string eventsLabelEnter;

    std::string eventsDescActivate1;
    std::string eventsDescActivate2;
    std::string eventsDescDeath1;
    std::string eventsDescTalk1;
    std::string eventsDescTalk2;
    std::string eventsDescLayerClear1;
    std::string eventsDescLayerClear2;
    std::string eventsDescLayerClear3;
    std::string eventsDescHit1;
    std::string eventsDescDestroy1;
    std::string eventsDescDestroy2;
    std::string eventsDescEnter1;
    std::string eventsDescEnter2;

    std::string eventsDescPhraseTriggersWhen;

    std::string eventsDeletingEvent;
    std::string eventsDeletionConfirm;
    std::string eventsDeletionCancel;

    std::string eventsPromptEventText;
    std::string eventsPromptEventName;
    std::string eventsItemNewEvent;

    std::string eventsShouldEvent;
    std::string eventsChangeSectionBoundsToCurrent;
    std::string eventsChangeAllSectionBoundsToCurrent;

    std::string eventsControlsForEvent;
    std::string eventsSettingsForEvent;
    std::string eventsHeaderShow;
    std::string eventsHeaderHide;
    std::string eventsHeaderToggle;
    std::string eventsHeaderMove;

    std::string eventsActionKeep;
    std::string eventsActionReset;
    std::string eventsActionSet;

    std::string eventsCaseMusic;
    std::string eventsCaseBackground;
    std::string eventsCaseBounds;

    std::string eventsPhraseAllSections;

    std::string eventsPropAutostart;
    std::string eventsPropSound;
    std::string eventsPropEndGame;
    std::string eventsPropControls;

    std::string eventsHeaderTriggerEvent;

    std::string levelName;
    std::string levelStartPos;

    std::string sectionSetBounds;
    std::string sectionScroll;
    std::string sectionHorizWrap;
    std::string sectionUnderwater;
    std::string sectionNoTurnBack;
    std::string sectionOffscreenExit;

    std::string worldName;
    std::string worldIntroLevel;
    std::string worldHubWorld;
    std::string worldRetryOnFail;
    std::string worldTotalStars;
    std::string worldAllowChars;
    std::string worldCreditIndex;

    std::string selectSoundForEvent;
    std::string selectSectBlankPropBlankForEvent;
    std::string selectAllSectPropBlankForEvent;
    std::string selectSectionBlankPropBlank;
    std::string selectPathBlankUnlock;
    std::string selectWarpTransitionEffect;
    std::string selectWorldMusic;

    std::string layersHeader;

    std::string labelLayer;
    std::string layersLabelAttached;
    std::string layersAbbrevAttLayer;
    std::string layersLayerDefault;

    std::string layersLabelAttachedLayer;
    std::string layersLabelMoveLayer;

    std::string layersDeletionHeader;
    std::string layersDeletionPreserveLayerContents;
    std::string layersDeletionConfirmPreserve;
    std::string layersDeletionConfirmDelete;
    std::string layersDeletionCancel;

    std::string layersDescAtt1;
    std::string layersDescAtt2;
    std::string layersDescAtt3;
    std::string layersDescAtt4;
    std::string layersDescAtt5;

    std::string layersPromptLayerName;
    std::string layersItemNewLayer;

    std::vector<std::string> listWarpTransitNames{6};
};

extern EditorContent g_editorStrings;

void initEditorStrings();

#endif // EDITOR_STRINGS_H
