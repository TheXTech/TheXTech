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

enum NPCID
{
    NPCID_GOOMBA_SMB3 = 1,
    NPCID_REDGOOMBA = 2,
    NPCID_REDPARAGOOMBA = 3,
    NPCID_GRNTROOPA_SMB3 = 4,
    NPCID_GRNSHELL_SMB3 = 5,
    NPCID_REDTROOPA_SMB3 = 6,
    NPCID_REDSHELL_SMB3 = 7,
    NPCID_PIRHANA_SMB3 = 8,
    NPCID_SHROOM_SMB3 = 9,
    NPCID_COIN_SMB3 = 10,

    NPCID_ITEMGOAL = 11,
    NPCID_PODOBOO = 12,
    NPCID_PLAYERFIREBALL = 13,
    NPCID_FIREFLOWER_SMB3 = 14,
    NPCID_BOOMBOOM = 15,
    NPCID_GOALORB_SMB3 = 16,
    NPCID_BULLET_SMB3 = 17,
    NPCID_BULLET_SMW = 18,
    NPCID_BLUSHYGUY = 19,
    NPCID_REDSHYGUY = 20,

    NPCID_CANNONENEMY = 21,
    NPCID_CANNONITEM = 22,
    NPCID_BUZZYBEETLE = 23,
    NPCID_BUZZYSHELL = 24,
    NPCID_NINJI_SMB2 = 25,
    NPCID_SPRING = 26,
    NPCID_UNDERGOOMBA = 27,
    NPCID_GRNCHEEP = 28,
    NPCID_HAMBRO = 29,
    NPCID_ENEMYHAMMER = 30,

    NPCID_KEY = 31,
    NPCID_PSWITCH_SMW = 32,
    NPCID_COIN_SMW = 33,
    NPCID_LEAF = 34,
    NPCID_GRNBOOT = 35,
    NPCID_SPINY_SMB3 = 36,
    NPCID_THWOMP_SMB3 = 37,
    NPCID_BOO_SMB3 = 38,
    NPCID_BIRDO = 39,
    NPCID_BIRDOEGG = 40,

    NPCID_GOALORB_SMB2 = 41,
    NPCID_EERIE = 42,
    NPCID_BOO_SMW = 43,
    NPCID_BIGBOO = 44,
    NPCID_ICE_BLOCK = 45,       NPCID_THROWBLOCK = NPCID_ICE_BLOCK,
    NPCID_DONUTBLOCK_RED = 46,  NPCID_REDDONUT = NPCID_DONUTBLOCK_RED,
    NPCID_LAKITU_SMB3 = 47,
    NPCID_SPINYBALL_SMB3 = 48,
    NPCID_TOOTHYPIPE = 49,
    NPCID_TOOTHY = 50,

    NPCID_BOTTOMPIRHANA = 51,
    NPCID_SIDEPIRHANA = 52,
    NPCID_CRAB = 53,
    NPCID_FLY = 54,
    NPCID_EXTKOOPA = 55,
    NPCID_CLOWNCAR = 56,
    NPCID_CONVEYER = 57,
    NPCID_METALBARREL = 58,
    NPCID_YELSWITCHGOOMBA = 59,
    NPCID_YELBLOCKS = 60,

    NPCID_BLUSWITCHGOOMBA = 61,
    NPCID_BLUBLOCKS = 62,
    NPCID_GRNSWITCHGOOMBA = 63,
    NPCID_GRNBLOCKS = 64,
    NPCID_REDSWITCHGOOMBA = 65,
    NPCID_REDBLOCKS = 66,
    NPCID_HPIPE_SHORT = 67,
    NPCID_HPIPE_LONG = 68,
    NPCID_VPIPE_SHORT = 69,
    NPCID_VPIPE_LONG = 70,

    NPCID_BIGGOOMBA = 71,
    NPCID_BIGTROOPA = 72,
    NPCID_BIGSHELL = 73,
    NPCID_BIGPIRHANA = 74,
    NPCID_TOAD_A = 75,
    NPCID_GRNPARA_SMB3 = 76,
    NPCID_NINJI_SMW = 77,
    NPCID_TANKTREADS = 78,
    NPCID_SHORTWOOD = 79,
    NPCID_LONGWOOD = 80,

    NPCID_SLANTWOOD_L = 81,
    NPCID_SLANTWOOD_R = 82,
    NPCID_SLANTWOOD_M = 83,
    NPCID_STATUE_SMB3 = 84,
    NPCID_EXT_FIRE_B = 85,
    NPCID_BOWSER_SMB3 = 86,
    NPCID_EXT_FIRE_A = 87,
    NPCID_COIN_SMB = 88,
    NPCID_GOOMBA_SMB = 89,
    NPCID_LIFE_SMB3 = 90,

    NPCID_BURIEDPLANT = 91,
    NPCID_VEGGIE_TURNIP = 92,
    NPCID_PIRHANA_SMB = 93,
    NPCID_TOAD_B = 94,
    NPCID_YOSHI_GREEN = 95,
    NPCID_YOSHIEGG = 96,
    NPCID_STAR_SMB3 = 97,
    NPCID_YOSHI_BLUE = 98,
    NPCID_YOSHI_YELLOW = 99,
    NPCID_YOSHI_RED = 100,

    NPCID_LUIGI = 101,
    NPCID_LINK = 102,
    NPCID_RED_COIN = 103,       NPCID_REDCOIN = NPCID_RED_COIN,
    NPCID_PLATFORM_SMB3 = 104,
    NPCID_CHECKERPLATFORM = 105,
    NPCID_PLATFORM_SMB = 106,
    NPCID_PINKBOBOMB = 107,
    NPCID_YOSHIFIRE = 108,
    NPCID_GRNTROOPA_SMW = 109,
    NPCID_REDTROOPA_SMW = 110,

    NPCID_BLUTROOPA = 111,
    NPCID_YELTROOPA = 112,
    NPCID_GRNSHELL_SMW = 113,
    NPCID_REDSHELL_SMW = 114,
    NPCID_BLUSHELL = 115,
    NPCID_YELSHELL = 116,
    NPCID_GRNKOOPA = 117,
    NPCID_REDKOOPA = 118,
    NPCID_BLUKOOPA = 119,
    NPCID_YELKOOPA = 120,

    NPCID_GRNPARA_SMW = 121,
    NPCID_REDPARA_SMW = 122,
    NPCID_BLUPARA = 123,
    NPCID_YELPARA = 124,
    NPCID_WOSU = 125,       NPCID_KNIGHT = NPCID_WOSU,
    NPCID_BIT_BLUE = 126,   NPCID_SLIME_B = NPCID_BIT_BLUE,
    NPCID_BOT_CYAN = 127,   NPCID_SLIME_A = NPCID_BOT_CYAN,
    NPCID_BIT = 128,        NPCID_SLIME_C = NPCID_BIT, NPCID_BIT_RED = NPCID_BIT,
    NPCID_TWEETER = 129,
    NPCID_REDSNIFIT = 130,

    NPCID_BLUSNIFIT = 131,
    NPCID_GRYSNIFIT = 132,
    NPCID_CANNONBALL = 133,
    NPCID_BOMB = 134,
    NPCID_BOBOMB_SMB2 = 135,
    NPCID_BOBOMB_SMB3 = 136,
    NPCID_ACTIVEBOBOMB_SMB3 = 137,
    NPCID_COIN_SMB2 = 138,
    NPCID_VEGGIE_ONION = 139,
    NPCID_VEGGIE_TURNIP_2 = 140,

    NPCID_VEGGIE_SPROUT = 141,
    NPCID_VEGGIE_PUMPKIN = 142,
    NPCID_VEGGIE_RADISH_SPROUT = 143,
    NPCID_VEGGIE_TURNIP_SPROUT = 144,
    NPCID_VEGGIE_TURNIP_SPROUT_2 = 145,
    NPCID_VEGGIE_RADISH = 146,
    NPCID_VEGGIE_RANDOM = 147,
    NPCID_YOSHI_BLACK = 148,
    NPCID_YOSHI_PURPLE = 149,
    NPCID_YOSHI_PINK = 150,

    NPCID_SIGN = 151,
    NPCID_RING_MISC = 152,
    NPCID_SHROOMBLOCK_A = 154,
    NPCID_SHROOMBLOCK_B = 155,
    NPCID_SHROOMBLOCK_C = 156,
    NPCID_SHROOMBLOCK_D = 157,
    NPCID_SATURN = 158,
    NPCID_QUICKSAND = 159,
    NPCID_PSHROOM = 159,
    NPCID_ROCKETWOOD = 160,

    NPCID_REDPARA_SMB3 = 161,
    NPCID_REX = 162,
    NPCID_REX_SQUISHED = 163,
    NPCID_MEGAMOLE = 164,
    NPCID_GALOOMBA = 165,
    NPCID_GALOOMBA_STOMPED = 166,
    NPCID_PARAGALOOMBA = 167,
    NPCID_BULLY = 168,
    NPCID_TANOOKISUIT = 169,
    NPCID_HAMMERSUIT = 170,

    NPCID_PLAYERHAMMER = 171,
    NPCID_GRNSHELL_SMB = 172,
    NPCID_GRNTROOPA_SMB = 173,
    NPCID_REDSHELL_SMB = 174,
    NPCID_REDTROOPA_SMB = 175,
    NPCID_GRNPARA_SMB = 176,
    NPCID_REDPARA_SMB = 177,
    NPCID_AXE = 178,
    NPCID_SAW = 179,
    NPCID_THWOMP_SMW = 180,

    NPCID_STATUE_SMW = 181,
    NPCID_FIREFLOWER_SMB = 182,
    NPCID_FIREFLOWER_SMW = 183,
    NPCID_SHROOM_SMB = 184,
    NPCID_SHROOM_SMW = 185,
    NPCID_LIFE_SMB = 186,
    NPCID_LIFE_SMW = 187,
    NPCID_MOON = 188,
    NPCID_DRYBONES = 189,
    NPCID_SKULL = 190,

    NPCID_REDBOOT = 191,
    NPCID_CHECKPOINT = 192,
    NPCID_BLUBOOT = 193,
    NPCID_DISCOSHELL = 194,
    NPCID_FLIPPEDDISCO = 195,
    NPCID_STAR_SMW = 196,
    NPCID_GOALTAPE = 197,
    NPCID_PRINCESS = 198,
    NPCID_BLARGG = 199,
    NPCID_BOWSER_SMB = 200,

    NPCID_WART = 201,
    NPCID_WARTBUBBLE = 202,
    NPCID_METROID_RIPPER = 203,        NPCID_METROID_A = NPCID_METROID_RIPPER,
    NPCID_METROID_ROCKET_RIPPER = 204, NPCID_METROID_B = NPCID_METROID_ROCKET_RIPPER,
    NPCID_METROID_ZOOMER = 205,        NPCID_METROID_C = NPCID_METROID_ZOOMER,
    NPCID_SPARK = 206,
    NPCID_SPIKE_TOP = 207, NPCID_SPIKEBEETLE = NPCID_SPIKE_TOP,
    NPCID_BOSSGLASS = 208,
    NPCID_MOTHERBRAIN = 209,
    NPCID_RINKA = 210,

    NPCID_RINKAGEN = 211,
    NPCID_DONUTBLOCK_BROWN = 212,       NPCID_BLUDONUT = NPCID_DONUTBLOCK_BROWN,
    NPCID_GRNVINE_SMB3 = 213,
    NPCID_REDVINE_SMB3 = 214,
    NPCID_GRNVINE_SMB2 = 215,
    NPCID_YELVINE = 216,
    NPCID_BLUVINE = 217,
    NPCID_GRNVINEBOTTOM = 218,
    NPCID_YELVINEBOTTOM = 219,
    NPCID_BLUVINEBOTTOM = 220,

    NPCID_LADDER = 221,
    NPCID_GRNVINE_SMB = 222,
    NPCID_GRNVINETOP_SMB = 223,
    NPCID_GRNVINE_SMW = 224,
    NPCID_VINEHEAD_RED_SMB3 = 225, NPCID_REDVINETOP_SMB3 = NPCID_VINEHEAD_RED_SMB3,
    NPCID_VINEHEAD_GREEN_SMB3 = 226, NPCID_GRNVINETOP_SMB3 = NPCID_VINEHEAD_GREEN_SMB3,
    NPCID_VINEHEAD_SMW = 227, NPCID_GRNVINETOP_SMW = NPCID_VINEHEAD_SMW,
    NPCID_YOSHI_CYAN = 228,
    NPCID_GREENCHEEPCHEEP = 229,
    NPCID_REDCHEEPCHEEP = 230,

    NPCID_BLOOPER_SMB3 = 231,
    NPCID_GOGGLEFISH = 232,
    NPCID_REDCHEEP = 233,
    NPCID_BONEFISH = 234,
    NPCID_BLOOPER = 235,
    NPCID_SMWCHEEP = 236,
    NPCID_ICEBLOCK = 237,
    NPCID_PSWITCH_SMB3 = 238,
    NPCID_DYNAMITE_PLUNGER = 239, NPCID_TNT = NPCID_DYNAMITE_PLUNGER,
    NPCID_TIMER_SMB2 = 240,

    NPCID_POW = 241,
    NPCID_GOOMBA_MISC = 242,
    NPCID_PARAGOOMBA_MISC = 243,
    NPCID_PARAGOOMBA_SMB3 = 244,
    NPCID_FIREPIRHANA = 245,
    NPCID_EXT_FIRE_D = 246,
    NPCID_POKEY = 247,
    NPCID_TIMER_SMB3 = 248,
    NPCID_SHROOM_SMB2 = 249,
    NPCID_HEART = 250,

    NPCID_GRNRUPEE = 251,
    NPCID_BLURUPEE = 252,
    NPCID_REDRUPEE = 253,
    NPCID_FAIRY_PENDANT = 254, NPCID_AMULET = NPCID_FAIRY_PENDANT,
    NPCID_LOCKDOOR = 255,
    NPCID_LONGPIRHANA_UP = 256,
    NPCID_LONGPIRHANA_DOWN = 257,
    NPCID_BLUECOIN = 258,
    NPCID_ROTODISK = 259,
    NPCID_FIREBAR = 260,

    NPCID_NIPPER_PLANT = 261, NPCID_MUNCHER = NPCID_NIPPER_PLANT,
    NPCID_MOUSER = 262,
    NPCID_ICE_CUBE = 263, NPCID_STATIC = NPCID_ICE_CUBE,
    NPCID_ICEFLOWER_SMB3 = 264,
    NPCID_PLAYERICEBALL = 265,
    NPCID_SWORDBEAM = 266,
    NPCID_LARRY = 267,
    NPCID_LARRYSHELL = 268,
    NPCID_LARRY_MAGIC_RING = 269,
    NPCID_PIRHANAHEAD = 270,

    NPCID_SWOOPER = 271, NPCID_BAT = NPCID_SWOOPER,
    NPCID_HOOPSTER = 272,
    NPCID_Q_SHROOM = 273,
    NPCID_DRAGONCOIN = 274,
    NPCID_VOLCANO_LOTUS = 275,          NPCID_LAVALOTUS = NPCID_VOLCANO_LOTUS,
    NPCID_VOLCANO_LOTUS_FIREBALL = 276, NPCID_EXT_FIRE_E = NPCID_VOLCANO_LOTUS_FIREBALL,
    NPCID_ICEFLOWER_SMW = 277,
    NPCID_PROPELLERBLOCK = 278,
    NPCID_PROPELLERCANNON = 279,
    NPCID_LUDWIG = 280,

    NPCID_LUDWIG_SHELL = 281, NPCID_LUDWIGSHELL = NPCID_LUDWIG_SHELL,
    NPCID_LUDWIG_FIRE = 282, NPCID_EXT_FIRE_C = NPCID_LUDWIG_FIRE,
    NPCID_BUBBLE = 283,
    NPCID_LAKITU_SMW = 284,
    NPCID_SPINY_SMW = 285,
    NPCID_SPINYEGG_SMW = 286,
    NPCID_SPINYBALL_SMW = 286,
    NPCID_RANDOM_POWERUP = 287,
    NPCID_POTION = 288,
    NPCID_POTIONDOOR = 289,
    NPCID_COCKPIT = 290,

    NPCID_PEACHBOMB = 291,
    NPCID_BOOMERANG = 292,
};
