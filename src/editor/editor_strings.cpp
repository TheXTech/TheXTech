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

    g_editorStrings.pickBlockContents = "Pick block contents";

    g_editorStrings.blockLetterWidth  = "W ";
    g_editorStrings.blockLetterHeight = "H ";
    g_editorStrings.blockCanBreak = "Can Break";
    g_editorStrings.blockTooltipCanBreak = "Legacy: breaks when hit";
    g_editorStrings.blockSlick = "Slick";
    g_editorStrings.blockInvis = "Invis";
    g_editorStrings.blockInside = "Inside:";

    g_editorStrings.warpTitle = "Warp Settings";
    g_editorStrings.warpPlacing = "Placing:";
    g_editorStrings.warpIn = "In";
    g_editorStrings.warpOut = "Out";
    g_editorStrings.warpDir = "Dir.";
    g_editorStrings.warpTwoWay = "Two-Way";
    g_editorStrings.warpStyle = "Style: ";
    g_editorStrings.warpStylePipe = "Pipe";
    g_editorStrings.warpStyleDoor = "Door";
    g_editorStrings.warpStyleBlipInstant = "Blip";
    g_editorStrings.warpStylePortal = "Port";
    g_editorStrings.warpEffect = "Effect:";
    g_editorStrings.warpAllow = "Allow:";
    g_editorStrings.warpItem = "Item";
    g_editorStrings.warpRide = "Ride";
    g_editorStrings.warpCannonExit = "Cannon Exit";
    g_editorStrings.warpSpeed = "Speed ";
    g_editorStrings.warpNeedStarCount = "Need {0} {1}";
    g_editorStrings.warpNeedKey = "Need Key";
    g_editorStrings.warpNeedFloor = "Need Floor";
    g_editorStrings.warpStarLockMessage = "Star lock msg";
    g_editorStrings.warpToMap = "To Map";
    g_editorStrings.warpLvlWarp = "Lvl Warp";
    g_editorStrings.warpTarget = "Target: ";
    g_editorStrings.warpTo = "To: {0}";
    g_editorStrings.warpShowStartScene = "Show Start Scene";
    g_editorStrings.warpShowStarCount = "Show Star Count";

    g_editorStrings.waterTitle = "Water Settings";

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
    g_editorStrings.phraseTypeLabelEvent = "{0} Event:";

    g_editorStrings.wordCoins = "Coins";

    g_editorStrings.wordEnabled = "Enabled";
    g_editorStrings.wordText = "Text";
    g_editorStrings.wordInstant = "Instant";
    g_editorStrings.wordMode = "Mode";
    g_editorStrings.wordHeight = "Height";
    g_editorStrings.wordWidth = "Width";

    g_editorStrings.phraseTextOf = "{0} Text";
    g_editorStrings.phraseSectionIndex = "Section {0}";
    g_editorStrings.phraseRadiusIndex = "Radius {0}";
    g_editorStrings.phraseWarpIndex = "Warp {0}";
    g_editorStrings.phraseGenericIndex = "Number {0}";
    g_editorStrings.phraseCountMore = "{0} More";
    g_editorStrings.phraseDelayIsMs = "Delay: {0} ms";
    g_editorStrings.mapPos = "Map Pos:";

    g_editorStrings.letterUp = "U";
    g_editorStrings.letterDown = "D";
    g_editorStrings.letterLeft = "L";
    g_editorStrings.letterRight = "R";
    g_editorStrings.letterCoordX = "X";
    g_editorStrings.letterCoordY = "Y";

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
    g_editorStrings.eventsLetterHit = "H:";
    g_editorStrings.eventsLetterDestroy = "D:";
    g_editorStrings.eventsLetterEnter = "E:";

    g_editorStrings.eventsLabelNext = "Next";
    g_editorStrings.eventsLabelActivate = "Activate";
    g_editorStrings.eventsLabelDeath = "Death";
    g_editorStrings.eventsLabelTalk = "Talk";
    g_editorStrings.eventsLabelLayerClear = "Layer Clear";
    g_editorStrings.eventsLabelHit = "Hit";
    g_editorStrings.eventsLabelDestroy = "Destroy";
    g_editorStrings.eventsLabelEnter = "Enter";

    g_editorStrings.eventsDescActivate = "NPC enters the screen";
    g_editorStrings.eventsDescDeath = "NPC dies";
    g_editorStrings.eventsDescTalk = "player talks to NPC";
    g_editorStrings.eventsDescLayerClear = "everything in object layer is gone";
    g_editorStrings.eventsDescHit = "block is hit";
    g_editorStrings.eventsDescDestroy = "block is destroyed";
    g_editorStrings.eventsDescEnter = "warp is entered";

    g_editorStrings.eventsDescPhraseTriggersWhenTemplate = "{0} triggers when {1}";
    g_editorStrings.eventsDescPhraseTriggersAfterTemplate = "{0} triggers {1} ms after event {2} occurs";

    g_editorStrings.eventsDeletingEvent = "Deleting event {0}";
    g_editorStrings.eventsDeletionConfirm = "Yes: delete event";
    g_editorStrings.eventsDeletionCancel = "No: do not delete event";

    g_editorStrings.eventsPromptEventText = "Event text";
    g_editorStrings.eventsPromptEventName = "Event name";
    g_editorStrings.eventsItemNewEvent = "<New Event>";

    g_editorStrings.eventsShouldEvent = "Should Event {0}";
    g_editorStrings.eventsChangeSectionBoundsToCurrent = "Change sect {0} bounds to current?";
    g_editorStrings.eventsChangeAllSectionBoundsToCurrent = "Change all sect bounds to current?";

    g_editorStrings.eventsControlsForEvent = "Controls for event {0}";
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
    g_editorStrings.eventsPropLayerSmoke = "Smoke";

    g_editorStrings.eventsHeaderTriggerEvent = "Trigger:";

    g_editorStrings.levelName = "Level Name";
    g_editorStrings.levelStartPos = "Start Pos";
    g_editorStrings.levelPathBG = "Path BG";
    g_editorStrings.levelBigBG = "Big BG";
    g_editorStrings.levelGameStart = "Game Start";
    g_editorStrings.levelAlwaysVis = "Always Vis";
    g_editorStrings.levelPathUnlocks = "Path Unlocks";

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

    g_editorStrings.selectSoundForEvent = "Sound for Event {0}";
    g_editorStrings.selectSectBlankPropBlankForEvent = "Sect {0} {1} for {2}";
    g_editorStrings.selectAllSectPropBlankForEvent = "All Sect {0} for {1}";
    g_editorStrings.selectSectionBlankPropBlank = "Section {0} {1}";
    g_editorStrings.selectPathBlankUnlock = "Path {0} Unlock By";
    g_editorStrings.selectWarpTransitionEffect = "Warp Transition Effect";
    g_editorStrings.selectWorldMusic = "World Music";

    g_editorStrings.layersHeader = "Layers:";

    g_editorStrings.labelLayer = "Layer:";
    g_editorStrings.layersLabelAttached = "Attached:";
    g_editorStrings.layersAbbrevAttLayer = "Att: ";
    g_editorStrings.layersLayerDefault = "Default";

    g_editorStrings.layersLabelAttachedLayer = "Attached Layer:";
    g_editorStrings.layersLabelMoveLayer = "Move Layer:";

    g_editorStrings.layersDeletionHeader = "Deleting layer {0}";
    g_editorStrings.layersDeletionPreserveLayerContents = "Preserve layer contents?";
    g_editorStrings.layersDeletionConfirmPreserve = "Yes: move to default layer";
    g_editorStrings.layersDeletionConfirmDelete = "No: *DELETE ALL CONTENTS*";
    g_editorStrings.layersDeletionCancel = "Cancel: do not delete layer";

    g_editorStrings.layersDescAtt = "Whenever the NPC moves, the attached layer moves following it";

    g_editorStrings.layersPromptLayerName = "Layer name";
    g_editorStrings.layersItemNewLayer = "<New Layer>";

    g_editorStrings.fileActionClearLevel = "Clear Level";
    g_editorStrings.fileActionClearWorld = "Clear World";
    g_editorStrings.fileActionOpen = "Open";
    g_editorStrings.fileActionRevert = "Revert";
    g_editorStrings.fileActionExit = "Exit";
    g_editorStrings.fileConfirmationSaveBeforeAction = "Save before you {0}?";
    g_editorStrings.fileConfirmationConfirmAction = "Are you sure you want to {0}?";
    g_editorStrings.fileConfirmationConvertFormatTo = "Convert format to {0}?";
    g_editorStrings.fileOptionYesSaveThenAction = "Yes: save then {0}";
    g_editorStrings.fileOptionActionWithoutSave = "{0} without saving";
    g_editorStrings.fileOptionCancelAction = "Cancel: do not {0}";
    g_editorStrings.fileOptionProceedWithConversion = "Proceed with conversion";
    g_editorStrings.fileOptionCancelConversion = "Cancel conversion";

    g_editorStrings.fileLabelCurrentFile = "Current file: ";
    g_editorStrings.fileLabelFormat = "Format:";

    g_editorStrings.fileFormatModern = "Modern";
    g_editorStrings.fileFormatLegacy = "Legacy";

    g_editorStrings.fileSectionLevel = "Level";
    g_editorStrings.fileSectionWorld = "World";
    g_editorStrings.fileCommandNew = "New";
    g_editorStrings.fileCommandOpen = "Open...";
    g_editorStrings.fileCommandSave = "Save";
    g_editorStrings.fileCommandSaveAs = "Save as...";

    g_editorStrings.fileConvertDesc = "The file extension will change but the old file will NOT be deleted.";
    g_editorStrings.fileConvertNoIssues = "There are no compatibility issues.";
    g_editorStrings.fileConvertFeaturesWillBeLost = "The features below will be LOST:";

    g_editorStrings.fileConvert38aUnsupported = "The SMBX38-A format is not supported for conversion.";
    g_editorStrings.fileConvertFormatUnknown = "Requested format is unknown.";

    g_editorStrings.fileConvertFeatureWarpTransit = "A warp uses new transition effect.";
    g_editorStrings.fileConvertFeatureWarpNeedsStand = "A warp requires player to stand.";
    g_editorStrings.fileConvertFeatureWarpCannonExit = "A warp has the cannon exit effect.";
    g_editorStrings.fileConvertFeatureWarpEnterEvent = "A warp triggers an event on entry.";
    g_editorStrings.fileConvertFeatureWarpCustomStarsMsg = "A warp has a custom stars message.";
    g_editorStrings.fileConvertFeatureWarpNoPrintStars = "A warp hides target level stars.";
    g_editorStrings.fileConvertFeatureWarpNoStartScene = "A level warp skips the start scene.";
    g_editorStrings.fileConvertFeatureWarpPortal = "A warp uses the portal effect.";

    g_editorStrings.fileConvertFeatureEventCustomMusic = "An event sets music to a file.";
    g_editorStrings.fileConvertFeatureEventAutoscroll = "An event uses modern autoscroll.";

    g_editorStrings.fileConvertFeatureNPCVariant = "An NPC uses a behavior variant.";
    g_editorStrings.fileConvertFeatureBlockForceSmashable = "A spin block uses legacy smash.";

    g_editorStrings.fileConvertFeatureCustomWorldMusic = "Uses custom world music file.";
    g_editorStrings.fileConvertFeatureWorldStarDisplay = "Uses world setting for star display.";
    g_editorStrings.fileConvertFeatureLevelStarDisplay = "A level uses setting for star display.";

    g_editorStrings.browserNewFile = "New file";
    g_editorStrings.browserSaveFile = "Save file";
    g_editorStrings.browserOpenFile = "Open file";

    g_editorStrings.browserItemNewFile = "<New File>";
    g_editorStrings.browserItemNewFolder = "<New Folder>";

    g_editorStrings.browserAskOverwriteFile = "Overwrite {0}?";

    g_editorStrings.tooltipSelect = "Select";
    g_editorStrings.tooltipErase = "Erase";
    g_editorStrings.tooltipEraseAll = "Erase All";
    g_editorStrings.tooltipBlocks = "Blocks";
    g_editorStrings.tooltipBGOs = "BGOs";
    g_editorStrings.tooltipNPCs = "NPCs";
    g_editorStrings.tooltipWarps = "Warps";
    g_editorStrings.tooltipWater = "Water";
    g_editorStrings.tooltipSettings = "Settings";
    g_editorStrings.tooltipLayers = "Layers";
    g_editorStrings.tooltipEvents = "Events";
    g_editorStrings.tooltipTiles = "Tiles";
    g_editorStrings.tooltipScenes = "Scenes";
    g_editorStrings.tooltipLevels = "Levels";
    g_editorStrings.tooltipPaths = "Paths";
    g_editorStrings.tooltipMusic = "Music";
    g_editorStrings.tooltipFile = "File";
    g_editorStrings.tooltipShow = "Show";

    g_editorStrings.listWarpTransitNames = {"NONE", "SCROLL", "FADE", "CIRCLE", "FLIP (H)", "FLIP (V)"};
}
