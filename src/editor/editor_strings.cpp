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
    g_editorStrings.phraseAreYouSure = "Are you sure?";
    g_editorStrings.pageBlankOfBlank = "Page {0} of {1}";

    g_editorStrings.pickBlockContents1 = "Pick block";
    g_editorStrings.pickBlockContents2 = "contents";

    g_editorStrings.npcInContainer = "In";
    g_editorStrings.npcInertNice = "Nice";
    g_editorStrings.npcStuckStop = "Stop";
    g_editorStrings.npcAbbrevGen = "Gen";
    g_editorStrings.npcPropertyActive = "Active";
    g_editorStrings.npcPropertyAttachSurface = "Attach";
    g_editorStrings.npcPropertyFacing = "Facing";

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

    g_editorStrings.wordNPC = "NPC";
    g_editorStrings.wordNPCGenitive = "NPC";

    g_editorStrings.wordEvent = "Event";
    g_editorStrings.wordEventGenitive = "Event";

    g_editorStrings.wordCoins = "Coins";

    g_editorStrings.wordEnabled = "Enabled";
    g_editorStrings.wordText = "Text";
    g_editorStrings.wordInstant = "Instant";
    g_editorStrings.phraseTextOf = "{0} Text";
    g_editorStrings.phraseSectionIndex = "Section {0}";
    g_editorStrings.phraseRadiusIndex = "Radius {0}";
    g_editorStrings.phraseCountMore = "{0} More";
    g_editorStrings.phraseDelayIsMs = "Delay: {0} ms";

    g_editorStrings.letterUp = "U";
    g_editorStrings.letterDown = "D";
    g_editorStrings.letterLeft = "L";
    g_editorStrings.letterRight = "R";

    g_editorStrings.toggleMagicBlock = "Magic Block";

    g_editorStrings.testMagicHand = "Magic Hand";
    g_editorStrings.testChar = "Char";
    g_editorStrings.testPower = "Power";
    g_editorStrings.testBoot = "Boot";
    g_editorStrings.testPet = "Pet";

    g_editorStrings.eventsHeader = "Events:";
    g_editorStrings.eventsLetterActivate = "A:";
    g_editorStrings.eventsLetterDeath = "D:";
    g_editorStrings.eventsLetterTalk = "T:";
    g_editorStrings.eventsLetterLayerClear = "L:";

    g_editorStrings.eventsDeletingEvent = "Deleting event {0}";
    g_editorStrings.eventsDeletionConfirm = "Yes: delete event";
    g_editorStrings.eventsDeletionCancel = "No: do not delete event";

    g_editorStrings.eventsPromptEventText = "Event text";
    g_editorStrings.eventsPromptEventName = "Event name";
    g_editorStrings.eventsItemNewEvent = "<New Event>";

    g_editorStrings.eventsShouldEvent = "Should Event {0}";
    g_editorStrings.eventsChangeSectionBoundsToCurrent = "Change sect {0} bounds to current?";
    g_editorStrings.eventsChangeAllSectionBoundsToCurrent = "Change all sect bounds to current?";

    g_editorStrings.eventsControlsForEvent = "Controls for event";
    g_editorStrings.eventsSettingsForEvent = "Settings for event";
    g_editorStrings.eventsHeaderShow = "Show:";
    g_editorStrings.eventsHeaderHide = "Hide:";
    g_editorStrings.eventsHeaderToggle = "Toggle:";
    g_editorStrings.eventsHeaderMove = "Move:";

    g_editorStrings.eventsActionKeep = "Keep";
    g_editorStrings.eventsActionReset = "Reset";
    g_editorStrings.eventsActionSet = "Set";

    g_editorStrings.eventsCaseMusic = "Music";
    g_editorStrings.eventsCaseBackground = "BG";
    g_editorStrings.eventsCaseBounds = "Bounds";

    g_editorStrings.eventsPhraseAllSections = "All Sections";

    g_editorStrings.eventsPropAutostart = "Autostart";
    g_editorStrings.eventsPropSound = "Sound";
    g_editorStrings.eventsPropEndGame = "End Game";
    g_editorStrings.eventsPropControls = "Controls";

    g_editorStrings.eventsHeaderTriggerEvent = "Trigger:";

    g_editorStrings.levelName = "Level Name";
    g_editorStrings.levelStartPos = "Start Pos";

    g_editorStrings.sectionSetBounds = "Set Bounds";
    g_editorStrings.sectionScroll = "Scroll";
    g_editorStrings.sectionHorizWrap = "Horiz. Wrap";
    g_editorStrings.sectionUnderwater = "Underwater";
    g_editorStrings.sectionNoTurnBack = "No Turn Back";
    g_editorStrings.sectionOffscreenExit = "Offscreen Exit";

    g_editorStrings.worldName = "World Name";
    g_editorStrings.worldIntroLevel = "Intro Level";
    g_editorStrings.worldHubWorld = "Hub World";
    g_editorStrings.worldRetryOnFail = "Retry On Fail";
    g_editorStrings.worldTotalStars = "Total Stars: ";
    g_editorStrings.worldAllowChars = "Allow Chars";
    g_editorStrings.worldCreditIndex = "World Credit Line {0}:";

    g_editorStrings.selectSoundForEvent = "Sound for Event";
    g_editorStrings.selectSectBlankPropBlankForEvent = "Sect {0} {1} for";
    g_editorStrings.selectAllSectPropBlankForEvent = "All Sect {0} for";
    g_editorStrings.selectSectionBlankPropBlank = "Section {0} {1}";
    g_editorStrings.selectPathBlankUnlock = "Path {0} Unlock By";
    g_editorStrings.selectWarpTransitionEffect = "Warp Transition Effect";
    g_editorStrings.selectWorldMusic = "World Music";

    g_editorStrings.layersHeader = "Layers:";

    g_editorStrings.labelLayer = "Layer:";
    g_editorStrings.labelAbbrevAttLayer = "Att: ";
    g_editorStrings.layerDefault = "Default";

    g_editorStrings.listWarpTransitNames = {"NONE", "SCROLL", "FADE", "CIRCLE", "FLIP (H)", "FLIP (V)"};
}
