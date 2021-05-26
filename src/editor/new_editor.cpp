#include <algorithm>

#include "../globals.h"
#include "../layers.h"
#include "new_editor.h"
#include "editor.h"
#include "../graphics.h"
#include "../sound.h"
#include "../main/level_file.h"
#include "../game_main.h"
#include "write_level.h"
#include "write_world.h"

#include "text_input.h"

constexpr auto LESet_Nothing = EventSection_t::LESet_Nothing;
constexpr auto LESet_ResetDefault = EventSection_t::LESet_ResetDefault;

constexpr int e_ScreenW = 640;
constexpr int e_ScreenH = 480;

void EditorScreen::EnsureWorld()
{
    if (WorldEditor) return;
    ClearLevel();
    WorldEditor = true;
}

void EditorScreen::EnsureLevel()
{
    if (!WorldEditor) return;
    ClearWorld();
    WorldEditor = false;
}

void EditorScreen::ResetCursor()
{
    EditorCursor.Block = Block_t();
    EditorCursor.Block.Type = 1;
    EditorCursor.Background = Background_t();
    EditorCursor.Background.Type = 1;
    EditorCursor.NPC = NPC_t();
    EditorCursor.NPC.Type = 1;
    EditorCursor.NPC.Direction = -1;
    EditorCursor.Water = Water_t();
    EditorCursor.Warp = Warp_t();
    EditorCursor.Warp.WarpNPC = true;
    EditorCursor.Warp.Direction = 3;
    EditorCursor.Warp.Direction2 = 3;
    EditorCursor.Warp.MapX = -1;
    EditorCursor.Warp.MapY = -1;

    EditorCursor.Tile = Tile_t();
    EditorCursor.Tile.Type = 1;
    EditorCursor.Scene = Scene_t();
    EditorCursor.Scene.Type = 1;
    EditorCursor.WorldLevel = WorldLevel_t();
    EditorCursor.WorldLevel.Type = 1;
    EditorCursor.WorldLevel.WarpX = -1;
    EditorCursor.WorldLevel.WarpY = -1;
    EditorCursor.WorldPath = WorldPath_t();
    EditorCursor.WorldPath.Type = 1;
    EditorCursor.WorldMusic = WorldMusic_t();

    FocusNPC();
    FocusBlock();
    FocusBGO();
    FocusTile();
}

bool AllowBubble()
{
    int type;
    if (EditorCursor.NPC.Type == 91 || EditorCursor.NPC.Type == 96
        || EditorCursor.NPC.Type == 283 || EditorCursor.NPC.Type == 284)
        type = EditorCursor.NPC.Special;
    else
        type = EditorCursor.NPC.Type;
    if (type == 134) return true;
    if(NPCHeight[type] > 36 || NPCWidth[type] > 36
        || NPCWidthGFX[type] > 36 || NPCHeightGFX[type] > 36)
    {
        int W = NPCWidth[type];
        int H = NPCHeight[type];
        if (NPCWidthGFX[type] > W) W = NPCWidthGFX[type];
        if (NPCHeightGFX[type] > H) H = NPCHeightGFX[type];
        if ((W <= 32 && H <= 54) || (H <= 32 && W <= 54))
            return true;
        else
            return false;
    }
    else
        return true;
}

void SetEditorNPCType(int type)
{
    if (EditorCursor.NPC.Type == 91 || EditorCursor.NPC.Type == 96
        || EditorCursor.NPC.Type == 283 || EditorCursor.NPC.Type == 284)
        EditorCursor.NPC.Special = type;
    else
    {
        // can't have a murderous default
        if (NPCIsAParaTroopa[type] && (!NPCIsAParaTroopa[EditorCursor.NPC.Type]))
            EditorCursor.NPC.Special = 1;
        EditorCursor.NPC.Type = type;
        if (!(NPCIsCheep[type] || NPCIsAParaTroopa[type] || type == 260))
            EditorCursor.NPC.Special = 0;
        if (!(type == 260) && EditorCursor.NPC.Special > 5)
            EditorCursor.NPC.Special = 0;
    }
    if (!(type == 15 || type == 39 || type == 86))
        EditorCursor.NPC.Legacy = false;
    if (EditorCursor.NPC.Type == 283 && !AllowBubble())
    {
        EditorCursor.NPC.Type = type;
        EditorCursor.NPC.Special = 0;
    }
    if (EditorCursor.NPC.Direction == 0 && (type == 57 || type == 60 || type == 62 || type == 64 || type == 66))
        EditorCursor.NPC.Direction = -1;
    printf("%d\n",type);
    ResetNPC(type);
}

void SetEditorBlockType(int type)
{
    if (BlockIsSizable[type])
    {
        if (EditorCursor.Block.Location.Width < 64.)
            EditorCursor.Block.Location.Width = 64.;
        if (EditorCursor.Block.Location.Height < 64.)
            EditorCursor.Block.Location.Height = 64.;
    }
    else
    {
        EditorCursor.Block.Location.Width = 0.;
        EditorCursor.Block.Location.Height = 0.;
    }
    if (type == 5 || type == 88 || type == 193 || type == 224)
    {
        if (EditorCursor.Block.Special == 0)
            EditorCursor.Block.Special = 1;
    }
    else if (type != 60 && type != 188 && type != 4 && type != 226 && type != 55 && type != 159 && type != 226
        && type != 55 && type != 90 && type != 170 && type != 171 && type != 172 && type != 173
        && type != 174 && type != 175 && type != 176 && type != 177 && type != 178 && type != 179 && type != 180
        && type != 181 && type != 622 && type != 623 && type != 624 && type != 625 && type != 626 && type != 627
        && type != 628 && type != 629 && type != 631 && type != 632)
        EditorCursor.Block.Special = 0;

    // only update slipperiness if it is currently what you would expect.
    if (EditorCursor.Block.Slippy ==
        (EditorCursor.Block.Type == 189 || EditorCursor.Block.Type == 190 || EditorCursor.Block.Type == 191
            || EditorCursor.Block.Type == 270 || EditorCursor.Block.Type == 271 || EditorCursor.Block.Type == 272
            || EditorCursor.Block.Type == 620 || EditorCursor.Block.Type == 621 || EditorCursor.Block.Type == 633
            || EditorCursor.Block.Type == 634 || EditorCursor.Block.Type == 241 || EditorCursor.Block.Type == 242))
    {
        EditorCursor.Block.Slippy = (type == 189 || type == 190 || type == 191 || type == 270 || type == 271 || type == 272 || type == 620 || type == 621 || type == 633 || type == 634 || type == 241 || type == 242);
    }

    printf("%d\n",type);
    EditorCursor.Block.Type = type;
}

void EditorScreen::FocusNPC()
{
    int type;
    if (m_special_page == SPECIAL_PAGE_BLOCK_CONTENTS)
        type = EditorCursor.Block.Special - 1000;
    else if (EditorCursor.NPC.Type == 91 || EditorCursor.NPC.Type == 96
        || EditorCursor.NPC.Type == 283 || EditorCursor.NPC.Type == 284)
        type = EditorCursor.NPC.Special;
    else
        type = EditorCursor.NPC.Type;
    if (m_special_page == SPECIAL_PAGE_BLOCK_CONTENTS && (type == 10 || type == 9 || type == 90 || type == 14 || type == 264 || type == 34 || type == 169 || type == 170 || type == 226 || type == 287 || type == 33 || type == 185 || type == 187 || type == 183 || type == 188 || type == 277 || type == 95 || type == 31 || type == 227 || type == 88 || type == 184 || type == 186 || type == 182 || type == 153 || type == 138 || type == 249 || type == 134 || type == 241 || type == 240 || type == 152 || type == 250 || type == 254 || type == 251 || type == 252 || type == 253))
        m_NPC_page = 7;
    else if ((type == 28) || (type == 233) || (type == 235) || (type == 260) || (type == 177) || (type == 176) || (type == 175) || (type == 173) || (type == 153) || (type == 93) || (type == 89) || (type == 29) || (type == 27) || (type == 223) || (type == 222) || (type == 174) || (type == 172) || (type == 106) || (type == 186) || (type == 182) || (type == 184) || (type == 178) || (type == 88) || (type == 200))
        m_NPC_page = 1;
    else if ((type == 289) || (type == 288) || (type == 215) || (type == 216) || (type == 217) || (type == 218) || (type == 219) || (type == 220) || (type == 221) || (type == 147) || (type == 146) || (type == 145) || (type == 144) || (type == 143) || (type == 141) || (type == 140) || (type == 139) || (type == 142) || (type == 92) || (type == 157) || (type == 156) || (type == 155) || (type == 154) || (type == 249) || (type == 241) || (type == 240) || (type == 138) || (type == 134) || (type == 41) || (type == 262) || (type == 201) || (type == 39) || (type == 272) || (type == 247) || (type == 206) || (type == 135) || (type == 132) || (type == 131) || (type == 130) || (type == 129) || (type == 25) || (type == 19) || (type == 20))
        m_NPC_page = 2;
    else if ((type == 226) || (type == 225) || (type == 214) || (type == 213) || (type == 11) || (type == 16) || (type == 97) || (type == 212) || (type == 57) || (type == 46) || (type == 104) || (type == 198) || (type == 101) || (type == 75) || (type == 94) || (type == 261) || (type == 245) || (type == 74) || (type == 52) || (type == 8) || (type == 51) || (type == 5) || (type == 7) || (type == 24) || (type == 73) || (type == 45) || (type == 160) || (type == 84) || (type == 21) || (type == 79) || (type == 80) || (type == 83) || (type == 82) || (type == 78) || (type == 81) || (type == 70) || (type == 69) || (type == 68) || (type == 58) || (type == 67) || (type == 267) || (type == 86) || (type == 15) || (type == 287) || (type == 273) || (type == 264) || (type == 238) || (type == 248) || (type == 49) || (type == 193) || (type == 191) || (type == 170) || (type == 169) || (type == 103) || (type == 34) || (type == 90) || (type == 35) || (type == 22) || (type == 14) || (type == 10) || (type == 9) || (type == 259) || (type == 244) || (type == 161) || (type == 137) || (type == 136) || (type == 76) || (type == 72) || (type == 71) || (type == 54) || (type == 53) || (type == 48) || (type == 47) || (type == 38) || (type == 37) || (type == 36) || (type == 23) || (type == 17) || (type == 12) || (type == 6) || (type == 2) || (type == 1) || (type == 3) || (type == 4) || (type == 231) || (type == 230) || (type == 229))
        m_NPC_page = 3;
    else if ((type == 280) || (type == 227) || (type == 224) || (type == 192) || (type == 151) || (type == 228) || (type == 150) || (type == 149) || (type == 148) || (type == 95) || (type == 98) || (type == 99) || (type == 100) || (type == 190) || (type == 105) || (type == 66) || (type == 64) || (type == 62) || (type == 60) || (type == 196) || (type == 197) || (type == 31) || (type == 279) || (type == 278) || (type == 277) || (type == 274) || (type == 258) || (type == 239) || (type == 195) || (type == 188) || (type == 187) || (type == 183) || (type == 185) || (type == 96) || (type == 56) || (type == 33) || (type == 32) || (type == 26) || (type == 236) || (type == 232) || (type == 234) || (type == 286) || (type == 285) || (type == 275) || (type == 271) || (type == 270) || (type == 207) || (type == 199) || (type == 189) || (type == 179) || (type == 181) || (type == 180) || (type == 167) || (type == 166) || (type == 165) || (type == 164) || (type == 163) || (type == 162) || (type == 77) || (type == 43) || (type == 42) || (type == 44) || (type == 18) || (type == 194) || (type == 124) || (type == 123) || (type == 122) || (type == 121) || (type == 120) || (type == 119) || (type == 118) || (type == 117) || (type == 116) || (type == 115) || (type == 114) || (type == 113) || (type == 112) || (type == 111) || (type == 110) || (type == 109))
        m_NPC_page = 4;
    else if ((type == 209) || (type == 208) || (type == 65) || (type == 63) || (type == 61) || (type == 59) || (type == 255) || (type == 254) || (type == 253) || (type == 252) || (type == 251) || (type == 250) || (type == 158) || (type == 152) || (type == 257) || (type == 256) || (type == 243) || (type == 242) || (type == 211) || (type == 205) || (type == 204) || (type == 203) || (type == 168) || (type == 128) || (type == 127) || (type == 126) || (type == 125) || (type == 107) || (type == 102))
        m_NPC_page = 5;
    else if (m_special_page == SPECIAL_PAGE_BLOCK_CONTENTS)
        m_NPC_page = 7;
    else
        m_NPC_page = 0;
}

void EditorScreen::FocusBlock()
{
    int type = EditorCursor.Block.Type;
    if (type == 335 || type == 334 || type == 333 || type == 332 || type == 64 || type == 62 || type == 63 || type == 65 || type == 93 || type == 189 || type == 190 || type == 191 || type == 331 || type == 330 || type == 329 || type == 328 || type == 327 || type == 326 || type == 325 || type == 324 || type == 323 || type == 56 || type == 57 || type == 58 || type == 59 || type == 61 || type == 405 || type == 371 || type == 403 || type == 402 || type == 401 || type == 400 || type == 399 || type == 398 || type == 397 || type == 396 || type == 395 || type == 394 || type == 393 || type == 390 || type == 387 || type == 386 || type == 385 || type == 384 || type == 383 || type == 376 || type == 378 || type == 197 || type == 196 || type == 377 || type == 194 || type == 195 || type == 372 || type == 382 || type == 381 || type == 380 || type == 379 || type == 375 || type == 374 || type == 373 || type == 577 || type == 576 || type == 388 || type == 369 || type == 69 || type == 66 || type == 67 || type == 68 || type == 192 || type == 60 || type == 188 || type == 193 || type == 287)
        m_Block_page = 1;
    else if (type == 343 || type == 342 || type == 341 || type == 340 || type == 339 || type == 338 || type == 337 || type == 336 || type == 44 || type == 43 || type == 42 || type == 46 || type == 45 || type == 41 || type == 39 || type == 40 || type == 198 || type == 199 || type == 200 || type == 206 || type == 201 || type == 203 || type == 204 || type == 205 || type == 207 || type == 208 || type == 211 || type == 212 || type == 213 || type == 214 || type == 215 || type == 216 || type == 511 || type == 510 || type == 508 || type == 507 || type == 506 || type == 504 || type == 503 || type == 502 || type == 501 || type == 500 || type == 499 || type == 498 || type == 497 || type == 496 || type == 495 || type == 494 || type == 493 || type == 492 || type == 491 || type == 490 || type == 574 || type == 573 || type == 505 || type == 509 || type == 392 || type == 391 || type == 389 || type == 298 || type == 297 || type == 131 || type == 202 || type == 209 || type == 210 || type == 370 || type == 293 || type == 288 || type == 243 || type == 242 || type == 38)
        m_Block_page = 2;
    else if (type == 305 || type == 306 || type == 307 || type == 308 || type == 314 || type == 311 || type == 312 || type == 313 || type == 278 || type == 277 || type == 279 || type == 20 || type == 19 || type == 18 || type == 14 || type == 13 || type == 12 || type == 11 || type == 9 || type == 10 || type == 364 || type == 363 || type == 362 || type == 361 || type == 360 || type == 359 || type == 358 || type == 357 || type == 356 || type == 355 || type == 354 || type == 353 || type == 352 || type == 351 || type == 350 || type == 349 || type == 348 || type == 347 || type == 346 || type == 345 || type == 344 || type == 630 || type == 91 || type == 136 || type == 126 || type == 29 || type == 579 || type == 575 || type == 568 || type == 130 || type == 108 || type == 260 || type == 240 || type == 241 || type == 26 || type == 25 || type == 27 || type == 28 || type == 621 || type == 620 || type == 186 || type == 225 || type == 224 || type == 226 || type == 159 || type == 55 || type == 2 || type == 4 || type == 5 || type == 406 || type == 267 || type == 268 || type == 269 || type == 30 || type == 110 || type == 109)
        m_Block_page = 3;
    else if (type == 607 || type == 606 || type == 605 || type == 604 || type == 603 || type == 602 || type == 601 || type == 600 || type == 596 || type == 595 || type == 597 || type == 275 || type == 274 || type == 276 || type == 1 || type == 129 || type == 128 || type == 127 || type == 92 || type == 17 || type == 16 || type == 15 || type == 3 || type == 6 || type == 7 || type == 634 || type == 633 || type == 270 || type == 271 || type == 272 || type == 638 || type == 637 || type == 636 || type == 635 || type == 286 || type == 285 || type == 284 || type == 167 || type == 162 || type == 163 || type == 164 || type == 165 || type == 166 || type == 102 || type == 101 || type == 100 || type == 99 || type == 98 || type == 97 || type == 96 || type == 95 || type == 94 || type == 221 || type == 220 || type == 219 || type == 218 || type == 217 || type == 222 || type == 612 || type == 611 || type == 610 || type == 609 || type == 608 || type == 54 || type == 53 || type == 52 || type == 51 || type == 50 || type == 49 || type == 48 || type == 47)
        m_Block_page = 4;
    else if (type == 571 || type == 570 || type == 569 || type == 296 || type == 295 || type == 294 || type == 182 || type == 103 || type == 104 || type == 36 || type == 35 || type == 34 || type == 37 || type == 32 || type == 31 || type == 33 || type == 24 || type == 23 || type == 22 || type == 21 || type == 599 || type == 598 || type == 578 || type == 594 || type == 593 || type == 592 || type == 591 || type == 590 || type == 589 || type == 588 || type == 587 || type == 586 || type == 585 || type == 584 || type == 583 || type == 582 || type == 581 || type == 580 || type == 614 || type == 613 || type == 185 || type == 77 || type == 76 || type == 75 || type == 74 || type == 73 || type == 72 || type == 71 || type == 70 || type == 78 || type == 132 || type == 572 || type == 457 || type == 458 || type == 168 || type == 8 || type == 290 || type == 289 || type == 280 || type == 107 || type == 106 || type == 105 || type == 184 || type == 223 || type == 187 || type == 160 || type == 112 || type == 111)
        m_Block_page = 5;
    else if (type == 619 || type == 618 || type == 617 || type == 616 || type == 489 || type == 488 || type == 300 || type == 299 || type == 304 || type == 301 || type == 302 || type == 303 || type == 310 || type == 309 || type == 266 || type == 265 || type == 264 || type == 263 || type == 273 || type == 80 || type == 81 || type == 82 || type == 83 || type == 84 || type == 85 || type == 86 || type == 87 || type == 322 || type == 319 || type == 320 || type == 321 || type == 315 || type == 316 || type == 317 || type == 318 || type == 368 || type == 365 || type == 366 || type == 367 || type == 246 || type == 247 || type == 248 || type == 249 || type == 250 || type == 251 || type == 252 || type == 253 || type == 254 || type == 262 || type == 255 || type == 256 || type == 257 || type == 258 || type == 445 || type == 444 || type == 443 || type == 442 || type == 441 || type == 440 || type == 439 || type == 438 || type == 437 || type == 161 || type == 79 || type == 244 || type == 259 || type == 245 || type == 261 || type == 615 || type == 283 || type == 282 || type == 281 || type == 169 || type == 89 || type == 88 || type == 90)
        m_Block_page = 6;
    else if (type == 454 || type == 455 || type == 456 || type == 453 || type == 452 || type == 451 || type == 450 || type == 449 || type == 418 || type == 417 || type == 416 || type == 415 || type == 436 || type == 427 || type == 426 || type == 425 || type == 424 || type == 423 || type == 422 || type == 421 || type == 419 || type == 435 || type == 434 || type == 433 || type == 432 || type == 414 || type == 413 || type == 412 || type == 411 || type == 410 || type == 404 || type == 420 || type == 487 || type == 484 || type == 483 || type == 481 || type == 473 || type == 475 || type == 477 || type == 478 || type == 486 || type == 485 || type == 482 || type == 472 || type == 474 || type == 476 || type == 479 || type == 480 || type == 459 || type == 460 || type == 461 || type == 462 || type == 463 || type == 464 || type == 465 || type == 466 || type == 467 || type == 468 || type == 469 || type == 470 || type == 471 || type == 431 || type == 430 || type == 429 || type == 428 || type == 409 || type == 408 || type == 407 || type == 121 || type == 122 || type == 123 || type == 124 || type == 125 || type == 181 || type == 180 || type == 179 || type == 178 || type == 177 || type == 176 || type == 175 || type == 174 || type == 173 || type == 171 || type == 170 || type == 172)
        m_Block_page = 7;
    else if (type == 139 || type == 140 || type == 141 || type == 144 || type == 145 || type == 146 || type == 148 || type == 149 || type == 150 || type == 154 || type == 155 || type == 156 || type == 158 || type == 157 || type == 153 || type == 152 || type == 151 || type == 147 || type == 143 || type == 142 || type == 138 || type == 137 || type == 113 || type == 114 || type == 448 || type == 447 || type == 446 || type == 115 || type == 135 || type == 134 || type == 133 || type == 227 || type == 228 || type == 229 || type == 230 || type == 231 || type == 232 || type == 233 || type == 234 || type == 235 || type == 236 || type == 237 || type == 238 || type == 239 || type == 116 || type == 117 || type == 118 || type == 119 || type == 120)
        m_Block_page = 8;
    else if (type == 632 || type == 631 || type == 629 || type == 628 || type == 627 || type == 626 || type == 625 || type == 624 || type == 623 || type == 622 || type == 567 || type == 566 || type == 565 || type == 564 || type == 563 || type == 562 || type == 561 || type == 560 || type == 559 || type == 558 || type == 557 || type == 556 || type == 555 || type == 554 || type == 553 || type == 552 || type == 551 || type == 550 || type == 549 || type == 548 || type == 547 || type == 546 || type == 545 || type == 512 || type == 513 || type == 514 || type == 515 || type == 516 || type == 517 || type == 518 || type == 519 || type == 520 || type == 521 || type == 522 || type == 523 || type == 524 || type == 525 || type == 526 || type == 527 || type == 528 || type == 529 || type == 530 || type == 531 || type == 532 || type == 533 || type == 534 || type == 535 || type == 536 || type == 537 || type == 538 || type == 539 || type == 540 || type == 541 || type == 542 || type == 543 || type == 544 || type == 291 || type == 292)
        m_Block_page = 9;
    else
        m_Block_page = 0;
}

void EditorScreen::FocusBGO()
{
    int type = EditorCursor.Background.Type;
    if (type == 147 || type == 161 || type == 131 || type == 130 || type == 129 || type == 128 || type == 127 || type == 86 || type == 84 || type == 85 || type == 25 || type == 23 || type == 24 || type == 22 || type == 21 || type == 147 || type == 20 || type == 18 || type == 19 || type == 169 || type == 168 || type == 164 || type == 26 || type == 16 || type == 17)
        m_BGO_page = 1;
    else if (type == 159 || type == 158 || type == 148 || type == 149 || type == 150 || type == 112 || type == 111 || type == 113 || type == 110 || type == 108 || type == 109 || type == 63 || type == 62 || type == 87 || type == 88)
        m_BGO_page = 2;
    else if (type == 163 || type == 162 || type == 103 || type == 102 || type == 101 || type == 99 || type == 106 || type == 67 || type == 80 || type == 81 || type == 37 || type == 38 || type == 36 || type == 68 || type == 89 || type == 90 || type == 91 || type == 69 || type == 97 || type == 93 || type == 45 || type == 46 || type == 94 || type == 95 || type == 96 || type == 114 || type == 1 || type == 9 || type == 10 || type == 4 || type == 5 || type == 6 || type == 2 || type == 8 || type == 7 || type == 3 || type == 172 || type == 171 || type == 170 || type == 165 || type == 65 || type == 66 || type == 82 || type == 83 || type == 75 || type == 76 || type == 77 || type == 78 || type == 33 || type == 34 || type == 13 || type == 61 || type == 60 || type == 12 || type == 11 || type == 79 || type == 40 || type == 39 || type == 64 || type == 15 || type == 14 || type == 190 || type == 189 || type == 187 || type == 188 || type == 100 || type == 107)
        m_BGO_page = 3;
    else if (type == 186 || type == 185 || type == 184 || type == 183 || type == 182 || type == 181 || type == 180 || type == 179 || type == 178 || type == 177 || type == 176 || type == 175 || type == 174 || type == 173 || type == 167 || type == 166 || type == 59 || type == 52 || type == 53 || type == 54 || type == 55 || type == 56 || type == 57 || type == 58 || type == 59 || type == 52 || type == 53 || type == 54 || type == 55 || type == 56 || type == 57 || type == 58 || type == 32 || type == 31 || type == 28 || type == 27 || type == 145 || type == 144 || type == 143 || type == 142 || type == 138 || type == 137 || type == 136 || type == 135 || type == 134 || type == 146 || type == 140 || type == 139 || type == 43 || type == 44 || type == 47 || type == 48 || type == 49 || type == 50 || type == 51 || type == 133 || type == 132 || type == 29 || type == 42 || type == 41 || type == 30 || type == 35 || type == 70 || type == 71 || type == 72 || type == 73 || type == 74 || type == 141 || type == 104 || type == 105 || type == 92)
        m_BGO_page = 4;
    else if (type == 157 || type == 156 || type == 155 || type == 154 || type == 153 || type == 152 || type == 151 || type == 115 || type == 116 || type == 117 || type == 118 || type == 119 || type == 120 || type == 121 || type == 122 || type == 123 || type == 124 || type == 125 || type == 126)
        m_BGO_page = 5;
    else
        m_BGO_page = 0;
}

void EditorScreen::FocusTile()
{
    int type = EditorCursor.Tile.Type;
    if (type == 20 || type == 21 || type == 22 || type == 15 || type == 16 || type == 12 || type == 23 || type == 11 || type == 24 || type == 18 || type == 19 || type == 13 || type == 25 || type == 26 || type == 17 || type == 40 || type == 41 || type == 28 || type == 32 || type == 29 || type == 5 || type == 6 || type == 3 || type == 36 || type == 37 || type == 8 || type == 33 || type ==  1 || type == 35 || type == 7 || type == 42 || type == 7 || type == 39 || type == 38 || type == 9 || type == 30 || type == 34 || type == 31 || type == 2 || type == 6 || type == 4 || type == 48 || type == 53 || type == 49 || type == 43 || type == 45 || type == 52 || type == 10 || type == 54 || type == 44 || type == 46 || type == 50 || type == 55 || type == 51 || type == 47)
        m_tile_page = 1;
    else if (type == 79 || type == 80 || type == 81 || type == 82 || type == 83 || type == 96 || type == 94 || type == 106 || type == 105 || type == 109 || type == 101 || type == 110 || type == 72 || type == 57 || type == 89 || type == 102 || type == 61 || type == 107 || type == 108 || type == 104 || type == 103 || type == 97 || type == 98 || type == 70 || type == 74 || type == 68 || type == 76 || type == 59 || type == 95 || type == 92 || type == 93 || type == 77 || type == 63 || type == 60 || type == 78 || type == 75 || type == 67 || type == 56 || type == 73 || type == 69 || type == 66 || type == 90 || type == 91 || type == 84 || type == 85 || type == 86 || type == 87 || type == 88 || type == 65 || type == 71 || type == 62 || type == 64 || type == 58 || type == 99 || type == 100 || type == 137 || type == 144 || type == 126 || type == 125 || type == 132 || type == 139 || type == 141 || type == 27 || type == 122 || type == 114 || type == 134 || type == 119 || type == 121 || type == 131 || type == 140 || type == 14 || type == 127 || type == 112 || type == 143 || type == 135 || type == 116 || type == 123 || type == 124 || type == 240 || type == 145 || type == 138 || type == 118 || type == 133 || type == 115 || type == 136 || type == 142 || type == 120 || type == 128 || type == 117 || type == 111 || type == 113 || type == 129 || type == 130)
        m_tile_page = 2;
    else if (type == 170 || type == 171 || type == 180 || type == 172 || type == 173 || type == 255 || type == 251 || type == 256 || type == 254 || type == 253 || type == 252 || type == 244 || type == 152 || type == 155 || type == 179 || type == 197 || type == 147 || type == 250 || type == 249 || type == 248 || type == 247 || type == 246 || type == 243 || type == 245 || type == 165 || type == 158 || type == 162 || type == 167 || type == 163 || type == 238 || type == 239 || type == 235 || type == 237 || type == 236 || type == 196 || type == 182 || type == 168 || type == 149 || type == 146 || type == 169 || type == 166 || type == 161 || type == 154 || type == 157 || type == 164 || type == 159 || type == 181 || type == 183 || type == 174 || type == 175 || type == 176 || type == 177 || type == 178 || type == 156 || type == 151 || type == 148 || type == 153 || type == 160 || type == 188 || type == 194 || type == 216 || type == 233 || type == 207 || type == 221 || type == 227 || type == 229 || type == 150 || type == 185 || type == 184 || type == 241 || type == 257 || type == 261 || type == 213 || type == 214 || type == 225 || type == 204 || type == 215 || type == 220 || type == 230 || type == 192 || type == 193 || type == 242 || type == 259 || type == 260 || type == 206 || type == 210 || type == 232 || type == 224 || type == 203 || type == 219 || type == 218 || type == 189 || type == 198 || type == 195 || type == 262 || type == 258 || type == 234 || type == 223 || type == 201 || type == 226 || type == 200 || type == 228 || type == 231 || type == 191 || type == 190 || type == 205 || type == 264 || type == 208 || type == 212 || type == 202 || type == 211 || type == 209 || type == 217 || type == 222 || type == 186 || type == 187 || type == 199 || type == 263)
        m_tile_page = 3;
    else if (type == 279 || type == 285 || type == 284 || type == 291 || type == 288 || type == 318 || type == 319 || type == 323 || type == 320 || type == 322 || type == 324 || type == 321 || type == 280 || type == 267 || type == 286 || type == 302 || type == 283 || type == 272 || type == 268 || type == 298 || type == 278 || type == 271 || type == 293 || type == 309 || type == 289 || type == 287 || type == 273 || type == 275 || type == 265 || type == 270 || type == 281 || type == 290 || type == 269 || type == 266 || type == 292 || type == 294 || type == 277 || type == 307 || type == 276 || type == 282 || type == 274 || type == 306 || type == 314 || type == 313 || type == 312 || type == 317 || type == 297 || type == 303 || type == 296 || type == 300 || type == 299 || type == 308 || type == 310 || type == 304 || type == 305 || type == 301 || type == 295 || type == 311 || type == 315 || type == 316 || type == 326 || type == 325 || type == 327 || type == 328)
        m_tile_page = 4;
    else
        m_tile_page = 0;
}

bool EditorScreen::UpdateButton(int x, int y, StdPicture &im, bool sel,
    int src_x, int src_y, int src_w, int src_h)
{
    // the button is 32x32 and outlined by a 36x36 box
    bool coll = false;
    if (MenuMouseX >= x && MenuMouseX < x + 32
        && MenuMouseY >= y && MenuMouseY < y + 32)
        coll = true;
    // outline:
    if (sel)
    {
        if (coll && MenuMouseDown)
            frmMain.renderRect(x - 2, y - 2, 36, 36, 0.f, 0.5f, 0.f, 1.0f, true);
        else
            frmMain.renderRect(x - 2, y - 2, 36, 36, 0.f, 1.0f, 0.f, 1.0f, true);
    }
    else if (coll && MenuMouseDown)
        frmMain.renderRect(x - 2, y - 2, 36, 36, 0.f, 0.f, 0.f, 1.0f, true);
    // background:
    if (MenuMouseDown && coll)
        frmMain.renderRect(x, y, 32, 32, 0.2f, 0.2f, 0.2f, true);
    else
        frmMain.renderRect(x, y, 32, 32, 0.5f, 0.5f, 0.5f, true);

    // scale and center image
    int dst_x, dst_y, dst_h, dst_w;
    if (src_w > 32 && src_w >= src_h)
    {
        dst_h = (src_h * 32) / src_w;
        dst_w = 32;
    }
    else if (src_h > 32 && src_h > src_w)
    {
        dst_w = (src_w * 32) / src_h;
        dst_h = 32;
    }
    else
    {
        dst_w = src_w;
        dst_h = src_h;
    }
    dst_x = x + 16 - dst_w / 2;
    dst_y = y + 16 - dst_h / 2;

    if (dst_w > src_w)
        printf("SHOULD NEVER HAPPEN\n");

    frmMain.renderTextureScale((double)dst_x, (double)dst_y, (double)dst_w, (double)dst_h, im, src_x, src_y, src_w, src_h);
    return (MenuMouseRelease && coll);
}

void EditorScreen::UpdateNPC(int x, int y, int type)
{
    if ((type < 1) || (type >= maxNPCType))
    {
        printf("OOPS, %d, %d!\n", type, maxNPCType);
        return;
    }
    int draw_width, draw_height;
    if (NPCWidthGFX[type] == 0)
    {
        draw_width = NPCWidth[type];
        draw_height = NPCHeight[type];
    }
    else
    {
        draw_width = NPCWidthGFX[type];
        draw_height = NPCHeightGFX[type];
    }
    if (m_special_page != SPECIAL_PAGE_BLOCK_CONTENTS)
    {
        bool sel = (EditorCursor.NPC.Type == type ||
            (EditorCursor.NPC.Special == type &&
                (EditorCursor.NPC.Type == 91 || EditorCursor.NPC.Type == 96
                    || EditorCursor.NPC.Type == 283 || EditorCursor.NPC.Type == 284)));
        if (UpdateButton(x, y, GFXNPC[type], sel, 0, 0, draw_width, draw_height) && !sel)
            SetEditorNPCType(type);
    }
    else
    {
        bool sel = (EditorCursor.Block.Special == type + 1000);
        if (UpdateButton(x, y, GFXNPC[type], sel, 0, 0, draw_width, draw_height) && !sel)
            EditorCursor.Block.Special = type + 1000;
    }
}

void EditorScreen::UpdateNPCGrid(int x, int y, const int* types, int n_npcs, int n_cols)
{
    for (int i = 0; i < n_npcs; i ++)
    {
        int type = types[i];
        int row = i / n_cols;
        int col = i % n_cols;
        UpdateNPC(x + col * 40 + 4, y + row * 40 + 4, type);
    }
}

void EditorScreen::UpdateNPCScreen()
{
    // NPC GUI
    frmMain.renderRect(e_ScreenW - 240, 40, 240, e_ScreenH - 40, 0.7f, 0.7f, 0.9f, 1.0f, true);
    if (m_special_page == SPECIAL_PAGE_BLOCK_CONTENTS && UpdateButton(e_ScreenW - 40 + 4, 40 + 4, GFX.NPC_modes, false, 0, 32*8, 32, 32))
    {
        m_special_page = SPECIAL_PAGE_NONE;
    }
    // Page selector
    // SuperPrint("TAB", 3, e_ScreenW - 240, 40);
    if (UpdateButton(e_ScreenW - 240 + 4, 40 + 4, GFXNPC[89], m_NPC_page == 1, 0, 0, 32, 32))
        m_NPC_page = 1;
    if (UpdateButton(e_ScreenW - 240 + 4, 80 + 4, GFXNPC[19], m_NPC_page == 2, 0, 0, 32, 32))
        m_NPC_page = 2;
    if (UpdateButton(e_ScreenW - 240 + 4, 120 + 4, GFXNPC[1], m_NPC_page == 3, 0, 0, 32, 32))
        m_NPC_page = 3;
    if (UpdateButton(e_ScreenW - 240 + 4, 160 + 4, GFXNPC[165], m_NPC_page == 4, 0, 0, 32, 32))
        m_NPC_page = 4;
    if (UpdateButton(e_ScreenW - 240 + 4, 200 + 4, GFXNPC[128], m_NPC_page == 5, 0, 0, NPCWidthGFX[128], NPCHeightGFX[128]))
        m_NPC_page = 5;
    if (m_special_page == SPECIAL_PAGE_BLOCK_CONTENTS && UpdateButton(e_ScreenW - 240 + 4, 240 + 4, GFXNPC[10], m_NPC_page == 7, 0, 0, 32, 32))
        m_NPC_page = 7;
    if (m_special_page == SPECIAL_PAGE_BLOCK_CONTENTS)
    {
        SuperPrint("PICK BLOCK", 3, e_ScreenW - 200, 90);
        SuperPrint("CONTENTS", 3, e_ScreenW - 200, 110);
    }
    if (m_special_page != SPECIAL_PAGE_BLOCK_CONTENTS)
    {
        // Containers
        SuperPrint("IN", 3, e_ScreenW - 40, 40);
        if (UpdateButton(e_ScreenW - 40 + 4, 60 + 4, GFXNPC[91], EditorCursor.NPC.Type == 91, 0, 0, 32, 32))
        {
            if (EditorCursor.NPC.Type == 91)
            {
                EditorCursor.NPC.Type = EditorCursor.NPC.Special;
                EditorCursor.NPC.Special = 0;
            }
            else if (!(EditorCursor.NPC.Type == 91 || EditorCursor.NPC.Type == 96
                || EditorCursor.NPC.Type == 283 || EditorCursor.NPC.Type == 284))
            {
                EditorCursor.NPC.Special = EditorCursor.NPC.Type;
                EditorCursor.NPC.Type = 91;
            }
            else
            {
                EditorCursor.NPC.Type = 91;
            }
        }
        if (UpdateButton(e_ScreenW - 40 + 4, 100 + 4, GFXNPC[96], EditorCursor.NPC.Type == 96, 0, 0, 32, 32))
        {
            if (EditorCursor.NPC.Type == 96)
            {
                EditorCursor.NPC.Type = EditorCursor.NPC.Special;
                EditorCursor.NPC.Special = 0;
            }
            else if (!(EditorCursor.NPC.Type == 91 || EditorCursor.NPC.Type == 96
                || EditorCursor.NPC.Type == 283 || EditorCursor.NPC.Type == 284))
            {
                EditorCursor.NPC.Special = EditorCursor.NPC.Type;
                EditorCursor.NPC.Type = 96;
            }
            else
            {
                EditorCursor.NPC.Type = 96;
            }
        }
        if (UpdateButton(e_ScreenW - 40 + 4, 140 + 4, GFXNPC[284], EditorCursor.NPC.Type == 284, 0, 0, 32, 32))
        {
            if (EditorCursor.NPC.Type == 284)
            {
                EditorCursor.NPC.Type = EditorCursor.NPC.Special;
                EditorCursor.NPC.Special = 0;
            }
            else if (!(EditorCursor.NPC.Type == 91 || EditorCursor.NPC.Type == 96
                || EditorCursor.NPC.Type == 283 || EditorCursor.NPC.Type == 284))
            {
                EditorCursor.NPC.Special = EditorCursor.NPC.Type;
                EditorCursor.NPC.Type = 284;
            }
            else
            {
                EditorCursor.NPC.Type = 284;
            }
        }
        if (AllowBubble())
        {
            if (UpdateButton(e_ScreenW - 40 + 4, 180 + 4, GFXNPC[283], EditorCursor.NPC.Type == 283, 0, 0, 32, 32))
            {
                if (EditorCursor.NPC.Type == 283)
                {
                    EditorCursor.NPC.Type = EditorCursor.NPC.Special;
                    EditorCursor.NPC.Special = 0;
                }
                else if (!(EditorCursor.NPC.Type == 91 || EditorCursor.NPC.Type == 96
                    || EditorCursor.NPC.Type == 283 || EditorCursor.NPC.Type == 284))
                {
                    EditorCursor.NPC.Special = EditorCursor.NPC.Type;
                    EditorCursor.NPC.Type = 283;
                }
                else
                {
                    EditorCursor.NPC.Type = 283;
                }
            }
        }
        // Various properties...
        int type;
        if (EditorCursor.NPC.Type == 91 || EditorCursor.NPC.Type == 96
            || EditorCursor.NPC.Type == 283 || EditorCursor.NPC.Type == 284)
            type = EditorCursor.NPC.Special;
        else
            type = EditorCursor.NPC.Type;
        // Direction
        int dir_neg_frame = 1;
        int dir_pos_frame = 2;
        bool show_random = true;
        if (type == 57 || type == 60 || type == 62 || type == 64 || type == 66)
            show_random = false;
        if (type == 60 || type == 62 || type == 64 || type == 66 || type == 104)
        {
            SuperPrint("ACTIVE", 3, e_ScreenW - 174, 40);
            dir_neg_frame = 8;
            dir_pos_frame = 7;
        }
        else if (type == 259 || type == 260)
        {
            SuperPrint("ATTACH", 3, e_ScreenW - 174, 40);
            dir_neg_frame = 5;
            dir_pos_frame = 6;
        }
        else
        {
            SuperPrint("FACING", 3, e_ScreenW - 174, 40);
            if (type == 106 || (NPCIsAParaTroopa[type] && EditorCursor.NPC.Special == 3))
            {
                dir_neg_frame = 3;
                dir_pos_frame = 4;
            }
        }
        if (UpdateButton(e_ScreenW - 180 + 4, 60 + 4, GFX.NPC_modes, EditorCursor.NPC.Direction == -1, 0, 32*dir_neg_frame, 32, 32))
            EditorCursor.NPC.Direction = -1;
        if (show_random && UpdateButton(e_ScreenW - 140 + 4, 60 + 4, GFX.NPC_modes, EditorCursor.NPC.Direction == 0, 0, 0, 32, 32))
            EditorCursor.NPC.Direction = 0;
        if (UpdateButton(e_ScreenW - 100 + 4, 60 + 4, GFX.NPC_modes, EditorCursor.NPC.Direction == 1, 0, 32*dir_pos_frame, 32, 32))
            EditorCursor.NPC.Direction = 1;
        // Inert ("nice") and Stuck ("stop")
        // The sign (NPC ID 151) is always nice.
        if (type == 151)
            EditorCursor.NPC.Inert = true;
        else
        {
            SuperPrint("NICE", 3, e_ScreenW - 200, 100);
            if (UpdateButton(e_ScreenW - 160 + 4, 120 + 4, GFX.NPC_modes, EditorCursor.NPC.Inert, 0, 32*7, 32, 32))
                EditorCursor.NPC.Inert = !EditorCursor.NPC.Inert;
        }
        SuperPrint("STOP", 3, e_ScreenW - 110, 100);
        if (UpdateButton(e_ScreenW - 120 + 4, 120 + 4, GFX.NPC_modes, EditorCursor.NPC.Stuck, 0, 32*7, 32, 32))
            EditorCursor.NPC.Stuck = !EditorCursor.NPC.Stuck;
        // Text
        if (EditorCursor.NPC.Inert)
        {
            MessageText = EditorCursor.NPC.Text;
            SuperPrint("TEXT", 3, e_ScreenW - 200, 160);
            if (UpdateButton(e_ScreenW - 160 + 4, 180 + 4, GFX.NPC_modes, !EditorCursor.NPC.Text.empty(), 0, 32*17, 32, 32))
                EditorCursor.NPC.Text = GetTextInput("NPC text", EditorCursor.NPC.Text);
        }
        // Generator
        SuperPrint("GEN", 3, e_ScreenW - 110, 160);
        if (UpdateButton(e_ScreenW - 120 + 4, 180 + 4, GFX.ECursor[1], EditorCursor.NPC.Generator, 0, 0, 32, 32))
            m_NPC_page = 6;
        // Behavior
        if (NPCIsAParaTroopa[EditorCursor.NPC.Type])
        {
            // Describe current AI if valid
            if (EditorCursor.NPC.Special == 0)
                SuperPrint("AI: TARGET", 3, e_ScreenW - 200, 220);
            else if (EditorCursor.NPC.Special == 1)
                SuperPrint("AI: JUMP", 3, e_ScreenW - 200, 220);
            else if (EditorCursor.NPC.Special == 2)
                SuperPrint("AI: L-R", 3, e_ScreenW - 200, 220);
            else if (EditorCursor.NPC.Special == 3)
                SuperPrint("AI: U-D", 3, e_ScreenW - 200, 220);
            else
                SuperPrint("AI", 3, e_ScreenW - 200, 220);

            if (UpdateButton(e_ScreenW - 200 + 4, 240 + 4, GFX.NPC_modes, EditorCursor.NPC.Special == 1, 0, 32*11, 32, 32))
                EditorCursor.NPC.Special = 1;
            if (UpdateButton(e_ScreenW - 160 + 4, 240 + 4, GFX.NPC_modes, EditorCursor.NPC.Special == 0, 0, 32*12, 32, 32))
                EditorCursor.NPC.Special = 0;
            if (UpdateButton(e_ScreenW - 120 + 4, 240 + 4, GFX.NPC_modes, EditorCursor.NPC.Special == 2, 0, 32*9, 32, 32))
                EditorCursor.NPC.Special = 2;
            if (UpdateButton(e_ScreenW - 80 + 4, 240 + 4, GFX.NPC_modes, EditorCursor.NPC.Special == 3, 0, 32*10, 32, 32))
                EditorCursor.NPC.Special = 3;
        }
        if (NPCIsCheep[EditorCursor.NPC.Type])
        {
            // Describe current AI if valid
            if (EditorCursor.NPC.Special == 0)
                SuperPrint("AI: SWIM", 3, e_ScreenW - 200, 220);
            else if (EditorCursor.NPC.Special == 1)
                SuperPrint("AI: JUMP", 3, e_ScreenW - 200, 220);
            else if (EditorCursor.NPC.Special == 2)
                SuperPrint("AI: LEAP", 3, e_ScreenW - 200, 220);
            else if (EditorCursor.NPC.Special == 3)
                SuperPrint("AI: L-R", 3, e_ScreenW - 200, 220);
            else if (EditorCursor.NPC.Special == 4)
                SuperPrint("AI: U-D", 3, e_ScreenW - 200, 220);
            else
                SuperPrint("AI", 3, e_ScreenW - 200, 220);

            if (UpdateButton(e_ScreenW - 200 + 4, 240 + 4, GFX.NPC_modes, EditorCursor.NPC.Special == 0, 0, 32*13, 32, 32))
                EditorCursor.NPC.Special = 0;
            if (UpdateButton(e_ScreenW - 160 + 4, 240 + 4, GFX.NPC_modes, EditorCursor.NPC.Special == 1, 0, 32*11, 32, 32))
                EditorCursor.NPC.Special = 1;
            if (UpdateButton(e_ScreenW - 120 + 4, 240 + 4, GFX.NPC_modes, EditorCursor.NPC.Special == 2, 0, 32*14, 32, 32))
                EditorCursor.NPC.Special = 2;
            if (UpdateButton(e_ScreenW - 80 + 4, 240 + 4, GFX.NPC_modes, EditorCursor.NPC.Special == 3, 0, 32*9, 32, 32))
                EditorCursor.NPC.Special = 3;
            if (UpdateButton(e_ScreenW - 40 + 4, 240 + 4, GFX.NPC_modes, EditorCursor.NPC.Special == 4, 0, 32*10, 32, 32))
                EditorCursor.NPC.Special = 4;
        }
        if (type == 15 || type == 39 || type == 86)
        {
            SuperPrint("USE 1.0 AI?", 3, e_ScreenW - 200, 220);
            if (UpdateButton(e_ScreenW - 200 + 4, 240 + 4, GFX.NPC_modes, EditorCursor.NPC.Legacy, 0, 32*15, 32, 32))
                EditorCursor.NPC.Legacy = true;
            if (UpdateButton(e_ScreenW - 160 + 4, 240 + 4, GFX.NPC_modes, !EditorCursor.NPC.Legacy, 0, 32*16, 32, 32))
                EditorCursor.NPC.Legacy = false;
        }
        if (EditorCursor.NPC.Type == 260)
        {
            SuperPrint("NUMBER " + std::to_string((int)EditorCursor.NPC.Special), 3, e_ScreenW - 200, 220);
            if (EditorCursor.NPC.Special > 0 && UpdateButton(e_ScreenW - 160 + 4, 240 + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
                EditorCursor.NPC.Special --;
            if (UpdateButton(e_ScreenW - 120 + 4, 240 + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
                EditorCursor.NPC.Special ++;
        }
        if (EditorCursor.NPC.Type == 289 || EditorCursor.NPC.Type == 288
            || (EditorCursor.NPC.Type == 91 && EditorCursor.NPC.Special == 288))
        {
            SuperPrint("SECTION " + std::to_string((int)EditorCursor.NPC.Special2+1), 3, e_ScreenW - 200, 220);
            if (EditorCursor.NPC.Special2 > 0 && UpdateButton(e_ScreenW - 160 + 4, 240 + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
                EditorCursor.NPC.Special2 --;
            if (EditorCursor.NPC.Special2 < 20 && UpdateButton(e_ScreenW - 120 + 4, 240 + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
                EditorCursor.NPC.Special2 ++;
        }

        // Events
        SuperPrint("EVENTS:", 3, e_ScreenW - 200, 294);
        SuperPrint("A:" + EditorCursor.NPC.TriggerActivate, 3, e_ScreenW - 240, 320);
        SuperPrint("D:" + EditorCursor.NPC.TriggerDeath, 3, e_ScreenW - 240, 340);
        SuperPrint("T:" + EditorCursor.NPC.TriggerTalk, 3, e_ScreenW - 240, 360);
        SuperPrint("L:" + EditorCursor.NPC.TriggerLast, 3, e_ScreenW - 240, 380);
        if (UpdateButton(e_ScreenW - 80 + 4, 280 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
            m_special_page = SPECIAL_PAGE_OBJ_TRIGGERS;

        // Layers
        SuperPrint("LAYER:", 3, e_ScreenW - 200, 414);
        if (EditorCursor.NPC.Layer.empty())
            SuperPrint("DEFAULT", 3, e_ScreenW - 240, 440);
        else
            SuperPrint(EditorCursor.NPC.Layer, 3, e_ScreenW - 240, 440);
        if (!EditorCursor.NPC.AttLayer.empty())
            SuperPrint("ATT: " + EditorCursor.NPC.AttLayer, 3, e_ScreenW - 240, 460);
        if (UpdateButton(e_ScreenW - 80 + 4, 400 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
            m_special_page = SPECIAL_PAGE_OBJ_LAYER;
    }

    // SMB 1
    if (m_NPC_page == 1)
    {
        SuperPrint("ENEMIES", 3, 10, 40);
        static const int p1_enemies[] = {28, 233, 235, 260, 177, 176, 175, 173, 153, 93, 89, 29, 27};
        UpdateNPCGrid(0, 60, p1_enemies, sizeof(p1_enemies)/sizeof(int), 5);

        SuperPrint("VINE", 3, 240, 320);
        static const int p1_vines[] = {223, 222};
        UpdateNPCGrid(240, 340, p1_vines, sizeof(p1_vines)/sizeof(int), 2);

        SuperPrint("SHELL", 3, 275, 220);
        static const int p1_shells[] = {174, 172};
        UpdateNPCGrid(280, 240, p1_shells, sizeof(p1_shells)/sizeof(int), 5);

        SuperPrint("PLAT", 3, 325, 380);
        static const int p1_platforms[] = {106};
        UpdateNPCGrid(320, 400, p1_platforms, sizeof(p1_platforms)/sizeof(int), 1);

        SuperPrint("ITEMS", 3, 10, 340);
        static const int p1_items[] = {186, 182, 184, 178, 88};
        UpdateNPCGrid(0, 360, p1_items, sizeof(p1_items)/sizeof(int), 6);

        SuperPrint("BOSS", 3, 324, 40);
        static const int p1_boss[] = {200};
        UpdateNPCGrid(360, 60, p1_boss, sizeof(p1_boss)/sizeof(int), 1);
    }

    // SMB 2
    if (m_NPC_page == 2)
    {
        SuperPrint("WARP", 3, 10, 260);
        static const int p2_warps[] = {289, 288};
        UpdateNPCGrid(0, 280, p2_warps, sizeof(p2_warps)/sizeof(int), 2);

        SuperPrint("VINE", 3, 240, 300);
        static const int p2_vines[] = {215, 216, 217, 218, 219, 220, 221};
        UpdateNPCGrid(240, 320, p2_vines, sizeof(p2_vines)/sizeof(int), 2);

        SuperPrint("VEG", 3, 320, 260);
        static const int p2_veg[] = {147, 146, 145, 144, 143, 141, 140, 139, 142, 92};
        UpdateNPCGrid(320, 280, p2_veg, sizeof(p2_veg)/sizeof(int), 2);

        SuperPrint("BLOCKS", 3, 10, 180);
        static const int p2_blocks[] = {157, 156, 155, 154};
        UpdateNPCGrid(0, 200, p2_blocks, sizeof(p2_blocks)/sizeof(int), 4);

        SuperPrint("ITEMS", 3, 10, 340);
        static const int p2_items[] = {249, 241, 240, 138, 134};
        UpdateNPCGrid(0, 360, p2_items, sizeof(p2_items)/sizeof(int), 6);

        SuperPrint("EXIT", 3, 324, 180);
        static const int p2_exit[] = {41};
        UpdateNPCGrid(360, 200, p2_exit, sizeof(p2_exit)/sizeof(int), 1);

        SuperPrint("BOSS", 3, 324, 40);
        static const int p2_boss[] = {262, 201, 39};
        UpdateNPCGrid(360, 60, p2_boss, sizeof(p2_boss)/sizeof(int), 1);

        SuperPrint("ENEMIES", 3, 10, 40);
        static const int p2_enemies[] = {272, 247, 206, 135, 132, 131, 130, 129, 25, 19, 20};
        UpdateNPCGrid(0, 60, p2_enemies, sizeof(p2_enemies)/sizeof(int), 5);
    }

    // SMB 3
    if (m_NPC_page == 3)
    {
        SuperPrint("VINE", 3, 240, 320);
        static const int p3_vines[] = {226, 225, 214, 213};
        UpdateNPCGrid(240, 340, p3_vines, sizeof(p3_vines)/sizeof(int), 4);

        SuperPrint("EXIT", 3, 324, 180);
        static const int p3_exit[] = {11, 16, 97};
        UpdateNPCGrid(360, 200, p3_exit, sizeof(p3_exit)/sizeof(int), 1);

        SuperPrint("PLAT", 3, 324, 380);
        static const int p3_platforms[] = {212, 57, 46, 104};
        UpdateNPCGrid(320, 400, p3_platforms, sizeof(p3_platforms)/sizeof(int), 2);

        SuperPrint("CHAR", 3, 240, 380);
        static const int p3_people[] = {198, 101, 75, 94};
        UpdateNPCGrid(240, 400, p3_people, sizeof(p3_people)/sizeof(int), 2);

        SuperPrint("PLANT", 3, 200, 180);
        static const int p3_plants[] = {261, 245, 74, 52, 8, 51};
        UpdateNPCGrid(200, 200, p3_plants, sizeof(p3_plants)/sizeof(int), 2);

        SuperPrint("SHELL", 3, 274, 220);
        static const int p3_shells[] = {5, 7, 24, 73};
        UpdateNPCGrid(280, 240, p3_shells, sizeof(p3_shells)/sizeof(int), 2);

        SuperPrint("BLOCKS", 3, 10, 180);
        static const int p3_blocks[] = {45, 160, 84, 21, 79, 80, 83, 82, 78, 81, 70, 69, 68, 58, 67};
        UpdateNPCGrid(0, 200, p3_blocks, sizeof(p3_blocks)/sizeof(int), 5);

        SuperPrint("BOSS", 3, 324, 40);
        static const int p3_boss[] = {267, 86, 15};
        UpdateNPCGrid(360, 60, p3_boss, sizeof(p3_boss)/sizeof(int), 1);

        SuperPrint("ITEMS", 3, 10, 340);
        static const int p3_items[] = {287, 273, 264, 238, 248, 49, 193, 191, 170, 169, 103, 34, 90, 35, 22, 14, 10, 9};
        UpdateNPCGrid(0, 360, p3_items, sizeof(p3_items)/sizeof(int), 6);

        SuperPrint("ENEMIES", 3, 10, 40);
        static const int p3_enemies[] = {259, 244, 161, 137, 136, 76, 72, 71, 54, 53, 48, 47, 38, 37, 36, 23, 17, 12, 6, 2, 1, 3, 4, 231, 230, 229};
        UpdateNPCGrid(0, 60, p3_enemies, sizeof(p3_enemies)/sizeof(int), 9);
    }

    // SMW
    if (m_NPC_page == 4)
    {
        SuperPrint("BOSS", 3, 324, 40);
        static const int p4_boss[] = {280};
        UpdateNPCGrid(360, 60, p4_boss, sizeof(p4_boss)/sizeof(int), 1);

        SuperPrint("VINE", 3, 240, 300);
        static const int p4_vines[] = {227, 224};
        UpdateNPCGrid(240, 320, p4_vines, sizeof(p4_vines)/sizeof(int), 2);

        SuperPrint("CKPT", 3, 280, 80);
        UpdateNPC(284, 104, 192);

        // The sign
        SuperPrint("SIGN", 3, 324, 120);
        UpdateNPC(364, 144, 151);

        SuperPrint("YOSHI", 3, 170, 380);
        static const int p4_yoshi[] = {228, 150, 149, 148, 95, 98, 99, 100};
        UpdateNPCGrid(160, 400, p4_yoshi, sizeof(p4_yoshi)/sizeof(int), 4);

        SuperPrint("PLAT", 3, 324, 340);
        static const int p4_platforms[] = {190, 105, 66, 64, 62, 60};
        UpdateNPCGrid(320, 360, p4_platforms, sizeof(p4_platforms)/sizeof(int), 2);

        SuperPrint("EXIT", 3, 324, 180);
        static const int p4_exit[] = {196, 197, 31};
        UpdateNPCGrid(360, 200, p4_exit, sizeof(p4_exit)/sizeof(int), 1);

        SuperPrint("ITEMS", 3, 10, 300);
        static const int p4_items[] = {279, 278, 277, 274, 258, 239, 195, 188, 187, 183, 185, 96, 56, 33, 32, 26};
        UpdateNPCGrid(0, 320, p4_items, sizeof(p4_items)/sizeof(int), 4);

        SuperPrint("ENEMIES", 3, 10, 40);
        static const int p4_enemies[] = {236, 232, 234, 286, 285, 275, 271,
            270, 207, 199, 189, 179, 181, 180, 167, 166, 165, 164, 163, 162,
            77, 43, 42, 44, 18, 194, 124, 123, 122, 121, 120, 119, 118, 117,
            116, 115, 114, 113, 112, 111, 110, 109};
        UpdateNPCGrid(0, 60, p4_enemies, sizeof(p4_enemies)/sizeof(int), 7);
    }

    // MISC
    if (m_NPC_page == 5)
    {
        SuperPrint("BOSS", 3, 324, 40);
        static const int p5_boss[] = {209, 208};
        UpdateNPCGrid(360, 60, p5_boss, sizeof(p5_boss)/sizeof(int), 1);

        SuperPrint("SWITCH", 3, 10, 180);
        static const int p5_switch[] = {65, 63, 61, 59};
        UpdateNPCGrid(0, 200, p5_switch, sizeof(p5_switch)/sizeof(int), 2);

        SuperPrint("ITEMS", 3, 10, 340);
        static const int p5_items[] = {255, 254, 253, 252, 251, 250, 158, 152};
        UpdateNPCGrid(0, 360, p5_items, sizeof(p5_items)/sizeof(int), 4);

        SuperPrint("ENEMIES", 3, 10, 40);
        static const int p5_enemies[] = {257, 256, 243, 242, 211, 205, 204,
            203, 168, 128, 127, 126, 125};
        UpdateNPCGrid(0, 60, p5_enemies, sizeof(p5_enemies)/sizeof(int), 5);

        SuperPrint("CHAR", 3, 240, 380);
        static const int p5_char[] = {107, 102};
        UpdateNPCGrid(240, 400, p5_char, sizeof(p5_char)/sizeof(int), 2);
    }

    // GENERATOR SETTINGS SCREEN
    if (m_NPC_page == 6 && m_special_page != SPECIAL_PAGE_BLOCK_CONTENTS)
    {
        SuperPrint("GENERATOR SETTINGS", 3, 10, 44);
        if (UpdateButton(e_ScreenW - 240 - 40 + 4, 40 + 4, GFX.NPC_modes, false, 0, 32*8, 32, 32))
            FocusNPC();
        SuperPrint("ENABLED", 3, 10, 110);
        if (UpdateButton(220 + 4, 100 + 4, GFX.NPC_modes, EditorCursor.NPC.Generator, 0, 32*7, 32, 32))
        {
            EditorCursor.NPC.Generator = true;
            if (!EditorCursor.NPC.GeneratorDirection)
                EditorCursor.NPC.GeneratorDirection = 1;
            if (!EditorCursor.NPC.GeneratorEffect)
                EditorCursor.NPC.GeneratorEffect = 1;
            if (EditorCursor.NPC.GeneratorTimeMax < 1.f)
                EditorCursor.NPC.GeneratorTimeMax = 1.f;
        }
        if (UpdateButton(260 + 4, 100 + 4, GFX.NPC_modes, !EditorCursor.NPC.Generator, 0, 32*8, 32, 32))
            EditorCursor.NPC.Generator = false;
        if (EditorCursor.NPC.Generator)
        {
            SuperPrint("DIRECTION", 3, 10, 150);
            if (UpdateButton(220 + 4, 140 + 4, GFX.NPC_modes, EditorCursor.NPC.GeneratorDirection == 1, 0, 32*3, 32, 32))
                EditorCursor.NPC.GeneratorDirection = 1;
            if (UpdateButton(260 + 4, 140 + 4, GFX.NPC_modes, EditorCursor.NPC.GeneratorDirection == 3, 0, 32*4, 32, 32))
                EditorCursor.NPC.GeneratorDirection = 3;
            if (UpdateButton(300 + 4, 140 + 4, GFX.NPC_modes, EditorCursor.NPC.GeneratorDirection == 2, 0, 32*1, 32, 32))
                EditorCursor.NPC.GeneratorDirection = 2;
            if (UpdateButton(340 + 4, 140 + 4, GFX.NPC_modes, EditorCursor.NPC.GeneratorDirection == 4, 0, 32*2, 32, 32))
                EditorCursor.NPC.GeneratorDirection = 4;
            if (EditorCursor.NPC.GeneratorEffect == 1)
                SuperPrint("EFFECT WARP", 3, 10, 190);
            else if (EditorCursor.NPC.GeneratorEffect == 2)
                SuperPrint("EFFECT SHOOT", 3, 10, 190);
            else
                SuperPrint("EFFECT", 3, 10, 190);
            if (UpdateButton(220 + 4, 180 + 4, GFX.NPC_modes, EditorCursor.NPC.GeneratorEffect == 1, 0, 32*5, 32, 32))
                EditorCursor.NPC.GeneratorEffect = 1;
            if (UpdateButton(260 + 4, 180 + 4, GFX.NPC_modes, EditorCursor.NPC.GeneratorEffect == 2, 0, 32*3, 32, 32))
                EditorCursor.NPC.GeneratorEffect = 2;
            SuperPrint("DELAY " + std::to_string(((int)EditorCursor.NPC.GeneratorTimeMax)*100) + "MS", 3, 10, 230);
            if (EditorCursor.NPC.GeneratorTimeMax > 1.f && UpdateButton(220 + 4, 220 + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
                EditorCursor.NPC.GeneratorTimeMax --;
            if (UpdateButton(260 + 4, 220 + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
                EditorCursor.NPC.GeneratorTimeMax ++;
        }
    }

    // COMMON CONTENTS
    if (m_NPC_page == 7 && m_special_page == SPECIAL_PAGE_BLOCK_CONTENTS)
    {
        SuperPrint("COINS", 3, 10, 40);
        {
            bool currently_coins = EditorCursor.Block.Special > 0 && EditorCursor.Block.Special < 1000;
            if (UpdateButton(0 + 4, 60 + 4, GFXNPC[10], currently_coins, 0, 0, 32, 32) && !currently_coins)
            {
                EditorCursor.Block.Special = 1;
                currently_coins = true;
            }
            if (currently_coins)
            {
                SuperPrint("x" + std::to_string(EditorCursor.Block.Special), 3, 40, 74);
                if (EditorCursor.Block.Special > 0 && UpdateButton(120 + 4, 60 + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
                    EditorCursor.Block.Special --;
                if (EditorCursor.Block.Special < 99 && UpdateButton(160 + 4, 60 + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
                    EditorCursor.Block.Special ++;
            }
        }
        SuperPrint("NOTHING", 3, 260, 40);
        if (UpdateButton(320 + 4, 60 + 4, GFXBlock[2], EditorCursor.Block.Special == 0, 0, 0, 32, 32))
            EditorCursor.Block.Special = 0;

        SuperPrint("SUPER MARIO BROS. 3", 3, 10, 140);
        static const int p7_common[] = {10, 9, 90, 14, 264, 34, 169, 170, 226, 287};
        UpdateNPCGrid(0, 160, p7_common, sizeof(p7_common)/sizeof(int), 10);

        SuperPrint("SUPER MARIO WORLD", 3, 10, 200);
        static const int p7_smw[] = {33, 185, 187, 183, 188, 277, 95, 31, 227};
        UpdateNPCGrid(0, 220, p7_smw, sizeof(p7_smw)/sizeof(int), 10);

        SuperPrint("SUPER MARIO BROS.", 3, 10, 260);
        static const int p7_smb1[] = {88, 184, 186, 182, 153};
        UpdateNPCGrid(0, 280, p7_smb1, sizeof(p7_smb1)/sizeof(int), 5);

        SuperPrint("SUPER MARIO BROS. 2", 3, 10, 320);
        static const int p7_smb2[] = {138, 249, 134, 241, 240};
        UpdateNPCGrid(0, 340, p7_smb2, sizeof(p7_smb2)/sizeof(int), 5);

        SuperPrint("MISC", 3, 10, 380);
        static const int p7_misc[] = {152, 250, 254, 251, 252, 253};
        UpdateNPCGrid(0, 400, p7_misc, sizeof(p7_misc)/sizeof(int), 10);
    }
}

void EditorScreen::UpdateEventsScreen()
{
    if (m_special_page == SPECIAL_PAGE_EVENT_DELETION)
    {
        SuperPrint("DELETING EVENT " + Events[m_current_event].Name, 3, 60, 40);
        SuperPrint("ARE YOU SURE?", 3, 10, 60);
        SuperPrint("YES: DELETE EVENT", 3, 60, 110);
        if (UpdateButton(20 + 4, 100 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
        {
            DeleteEvent(Events[m_current_event].Name);
            m_special_page = SPECIAL_PAGE_EVENTS;
            m_current_event = 0;
        }
        SuperPrint("NO: DO NOT DELETE EVENT", 3, 60, 150);
        if (UpdateButton(20 + 4, 140 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
        {
            m_special_page = SPECIAL_PAGE_EVENTS;
            m_current_event = 0;
        }
        return;
    }

    // render general GUI
    SuperPrint("EVENTS", 3, 60, 40);
    int page_max = numEvents / 10;
    SuperPrint("PAGE " + std::to_string(m_events_page+1) + " OF " + std::to_string(page_max+1), 3, e_ScreenW - 330, 40);
    if (m_events_page > 0 && UpdateButton(e_ScreenW - 120 + 4, 40 + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
        m_events_page --;
    if (m_events_page < page_max && UpdateButton(e_ScreenW - 80 + 4, 40 + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
        m_events_page ++;

    // render event selector
    for (int i = 0; i < 10; i++)
    {
        int e = m_events_page*10 + i;
        if (!Events[e].Name.empty())
        {
            if (Events[e].Name.length() < 20)
                SuperPrint(Events[e].Name, 3, 10, 80 + 40*i + 10);
            else
            {
                SuperPrint(Events[e].Name.substr(0,19), 3, 10, 80 + 40*i + 2);
                SuperPrint(Events[e].Name.substr(19), 3, 10, 80 + 40*i + 20);
            }
            if (UpdateButton(360 + 4, 80 + 40*i + 4, GFX.NPC_modes, false, 0, 32*25, 32, 32))
            {
                m_special_page = SPECIAL_PAGE_EVENT_SETTINGS;
                m_current_event = e;
            }

            // rename, shift up, shift down, delete

            if (e <= 2)
                continue;

            // rename
            if (UpdateButton(400 + 4, 80 + 40*i + 4, GFX.NPC_modes, false, 0, 32*17, 32, 32))
            {
                std::string new_name = GetTextInput("New event name", Events[e].Name);
                if (!new_name.empty())
                    RenameEvent(Events[e].Name, new_name);
            }

            // shift up
            if (e > 3 && UpdateButton(440 + 4, 80 + 40*i + 4, GFX.NPC_modes, false, 0, 32*3, 32, 32))
                std::swap(Events[e-1], Events[e]);

            // shift down
            if (e < numEvents - 1 && UpdateButton(480 + 4, 80 + 40*i + 4, GFX.NPC_modes, false, 0, 32*4, 32, 32))
                std::swap(Events[e], Events[e+1]);

            // delete
            if (e < numEvents && UpdateButton(520 + 4, 80 + 40*i + 4, GFX.NPC_modes, false, 0, 32*8, 32, 32))
            {
                m_special_page = SPECIAL_PAGE_EVENT_DELETION;
                m_current_event = e;
                return;
            }
        }
        // create a new event!
        else if (e != 0 && !Events[e - 1].Name.empty())
        {
            SuperPrint("<NEW EVENT>", 3, 54, 80 + 40*i + 10);
            // rename only
            if (UpdateButton(400 + 4, 80 + 40*i + 4, GFX.NPC_modes, false, 0, 32*17, 32, 32))
            {
                std::string new_name = GetTextInput("New event name", "");
                if (!new_name.empty())
                {
                    InitializeEvent(Events[e]);
                    Events[e].Name = new_name;
                    numEvents ++;
                }
            }
        }
    }
}

void EditorScreen::UpdateEventSettingsScreen()
{
    if (m_special_page == SPECIAL_PAGE_EVENT_BOUNDS)
    {
        SuperPrint("SHOULD EVENT " + Events[m_current_event].Name, 3, 60, 40);
        if (m_special_subpage != 0)
            SuperPrint("CHANGE SECTION " + std::to_string(m_special_subpage) + " BOUNDS TO CURRENT?", 3, 10, 60);
        else
            SuperPrint("CHANGE ALL SECTION BOUNDS TO CURRENT?", 3, 10, 60);
        SuperPrint("YES", 3, 60, 110);
        if (UpdateButton(20 + 4, 100 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
        {
            if (m_special_subpage == 0)
            {
                for (int s = 0; s <= maxSections; s++)
                    Events[m_current_event].section[s].position = level[s];
            }
            else
                Events[m_current_event].section[m_special_subpage-1].position = level[m_special_subpage-1];
            m_special_page = SPECIAL_PAGE_EVENT_SETTINGS;
        }
        SuperPrint("NO", 3, 60, 150);
        if (UpdateButton(20 + 4, 140 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
        {
            m_special_page = SPECIAL_PAGE_EVENT_SETTINGS;
        }
        return;
    }

    if (m_special_page == SPECIAL_PAGE_EVENT_CONTROLS)
    {
        SuperPrint("CONTROLS FOR EVENT", 3, 60, 40);
        SuperPrint(Events[m_current_event].Name, 3, 10, 60);

        if (UpdateButton(e_ScreenW - 40 + 4, 40 + 4, GFX.NPC_modes, false, 0, 8*32, 32, 32))
        {
            m_special_page = SPECIAL_PAGE_EVENT_SETTINGS;
        }

        if (UpdateButton(10 + 4, 80 + 4, GFX.NPC_modes, Events[m_current_event].Controls.Up, 0, 7*32, 32, 32))
            Events[m_current_event].Controls.Up = !Events[m_current_event].Controls.Up;
        SuperPrint("UP", 3, 54, 90);
        if (UpdateButton(e_ScreenW/2 + 10 + 4, 80 + 4, GFX.NPC_modes, Events[m_current_event].Controls.Down, 0, 7*32, 32, 32))
            Events[m_current_event].Controls.Down = !Events[m_current_event].Controls.Down;
        SuperPrint("DOWN", 3, e_ScreenW/2 + 54, 90);

        if (UpdateButton(10 + 4, 120 + 4, GFX.NPC_modes, Events[m_current_event].Controls.Left, 0, 7*32, 32, 32))
            Events[m_current_event].Controls.Left = !Events[m_current_event].Controls.Left;
        SuperPrint("LEFT", 3, 54, 130);
        if (UpdateButton(e_ScreenW/2 + 10 + 4, 120 + 4, GFX.NPC_modes, Events[m_current_event].Controls.Right, 0, 7*32, 32, 32))
            Events[m_current_event].Controls.Right = !Events[m_current_event].Controls.Right;
        SuperPrint("RIGHT", 3, e_ScreenW/2 + 54, 130);

        if (UpdateButton(10 + 4, 160 + 4, GFX.NPC_modes, Events[m_current_event].Controls.Jump, 0, 7*32, 32, 32))
            Events[m_current_event].Controls.Jump = !Events[m_current_event].Controls.Jump;
        SuperPrint("JUMP", 3, 54, 170);
        if (UpdateButton(e_ScreenW/2 + 10 + 4, 160 + 4, GFX.NPC_modes, Events[m_current_event].Controls.Run, 0, 7*32, 32, 32))
            Events[m_current_event].Controls.Run = !Events[m_current_event].Controls.Run;
        SuperPrint("RUN", 3, e_ScreenW/2 + 54, 170);

        if (UpdateButton(10 + 4, 200 + 4, GFX.NPC_modes, Events[m_current_event].Controls.AltJump, 0, 7*32, 32, 32))
            Events[m_current_event].Controls.AltJump = !Events[m_current_event].Controls.AltJump;
        SuperPrint("ALT JUMP", 3, 54, 210);
        if (UpdateButton(e_ScreenW/2 + 10 + 4, 200 + 4, GFX.NPC_modes, Events[m_current_event].Controls.AltRun, 0, 7*32, 32, 32))
            Events[m_current_event].Controls.AltRun = !Events[m_current_event].Controls.AltRun;
        SuperPrint("ALT RUN", 3, e_ScreenW/2 + 54, 210);

        if (UpdateButton(10 + 4, 240 + 4, GFX.NPC_modes, Events[m_current_event].Controls.Start, 0, 7*32, 32, 32))
            Events[m_current_event].Controls.Start = !Events[m_current_event].Controls.Start;
        SuperPrint("START", 3, 54, 250);
        if (UpdateButton(e_ScreenW/2 + 10 + 4, 240 + 4, GFX.NPC_modes, Events[m_current_event].Controls.Drop, 0, 7*32, 32, 32))
            Events[m_current_event].Controls.Drop = !Events[m_current_event].Controls.Drop;
        SuperPrint("DROP", 3, e_ScreenW/2 + 54, 250);
        return;
    }

    SuperPrint("SETTINGS FOR EVENT", 3, 60, 40);
    SuperPrint(Events[m_current_event].Name, 3, 10, 60);

    // RIGHT PANE: layers
    frmMain.renderRect(e_ScreenW - 240, 40, 240, e_ScreenH - 40, 0.7f, 0.7f, 0.9f, 1.0f, true);
    if (UpdateButton(e_ScreenW - 40 + 4, 40 + 4, GFX.NPC_modes, false, 0, 32*8, 32, 32))
    {
        m_special_page = SPECIAL_PAGE_EVENTS;
        m_current_event = 0;
    }
    // layers
    int layer_line = 1;
    if (!Events[m_current_event].ShowLayer.empty())
    {
        layer_line ++;
        SuperPrint("SHOW:", 3, e_ScreenW-200, 40 + 20*layer_line);
        layer_line ++;
        SuperPrint(Events[m_current_event].ShowLayer[0], 3, e_ScreenW-240, 40 + 20*layer_line);
        layer_line ++;
        if (Events[m_current_event].ShowLayer.size() >= 2)
        {
            SuperPrint(Events[m_current_event].ShowLayer[1], 3, e_ScreenW-240, 40 + 20*layer_line);
            layer_line ++;
        }
        if (Events[m_current_event].ShowLayer.size() == 3)
        {
            SuperPrint(Events[m_current_event].ShowLayer[2], 3, e_ScreenW-240, 40 + 20*layer_line);
            layer_line ++;
        }
        else if (Events[m_current_event].ShowLayer.size() > 3)
        {
            SuperPrint(std::to_string(Events[m_current_event].ShowLayer.size() - 2) + " MORE", 3, e_ScreenW-220, 40 + 20*layer_line);
            layer_line ++;
        }
    }
    if (!Events[m_current_event].HideLayer.empty())
    {
        layer_line ++;
        SuperPrint("HIDE:", 3, e_ScreenW-200, 40 + 20*layer_line);
        layer_line ++;
        SuperPrint(Events[m_current_event].HideLayer[0], 3, e_ScreenW-240, 40 + 20*layer_line);
        layer_line ++;
        if (Events[m_current_event].HideLayer.size() >= 2)
        {
            SuperPrint(Events[m_current_event].HideLayer[1], 3, e_ScreenW-240, 40 + 20*layer_line);
            layer_line ++;
        }
        if (Events[m_current_event].HideLayer.size() == 3)
        {
            SuperPrint(Events[m_current_event].HideLayer[2], 3, e_ScreenW-240, 40 + 20*layer_line);
            layer_line ++;
        }
        else if (Events[m_current_event].HideLayer.size() > 3)
        {
            SuperPrint(std::to_string(Events[m_current_event].HideLayer.size() - 2) + " MORE", 3, e_ScreenW-220, 40 + 20*layer_line);
            layer_line ++;
        }
    }
    if (!Events[m_current_event].ToggleLayer.empty())
    {
        layer_line ++;
        SuperPrint("TOGGLE:", 3, e_ScreenW-200, 40 + 20*layer_line);
        layer_line ++;
        SuperPrint(Events[m_current_event].ToggleLayer[0], 3, e_ScreenW-240, 40 + 20*layer_line);
        layer_line ++;
        if (Events[m_current_event].ToggleLayer.size() >= 2)
        {
            SuperPrint(Events[m_current_event].ToggleLayer[1], 3, e_ScreenW-240, 40 + 20*layer_line);
            layer_line ++;
        }
        if (Events[m_current_event].ToggleLayer.size() == 3)
        {
            SuperPrint(Events[m_current_event].ToggleLayer[2], 3, e_ScreenW-240, 40 + 20*layer_line);
            layer_line ++;
        }
        else if (Events[m_current_event].ToggleLayer.size() > 3)
        {
            SuperPrint(std::to_string(Events[m_current_event].ToggleLayer.size() - 2) + " MORE", 3, e_ScreenW-220, 40 + 20*layer_line);
            layer_line ++;
        }
    }
    // MoveLayer is a string, not a vector
    if (!Events[m_current_event].MoveLayer.empty())
    {
        layer_line ++;
        SuperPrint("MOVE:", 3, e_ScreenW-200, 40 + 20*layer_line);
        layer_line ++;
        SuperPrint(Events[m_current_event].MoveLayer, 3, e_ScreenW-240, 40 + 20*layer_line);
        layer_line ++;
        // settings for this...
        int sy = (vb6Round)(Events[m_current_event].SpeedY*10);
        int sx = (vb6Round)(Events[m_current_event].SpeedX*10);
        if (sy < 0)
            SuperPrint("U"+std::to_string(-sy), 3, e_ScreenW-240, 40 + 20*layer_line);
        else if (sy > 0)
            SuperPrint("D"+std::to_string(sy), 3, e_ScreenW-240, 40 + 20*layer_line);
        else
            SuperPrint("-", 3, e_ScreenW-240, 40 + 20*layer_line);
        layer_line ++;
        if (sx < 0)
            SuperPrint("L"+std::to_string(-sx), 3, e_ScreenW-240, 40 + 20*layer_line);
        else if (sx > 0)
            SuperPrint("R"+std::to_string(sx), 3, e_ScreenW-240, 40 + 20*layer_line);
        else
            SuperPrint("-", 3, e_ScreenW-240, 40 + 20*layer_line);
        layer_line --;
        if (UpdateButton(e_ScreenW-160 + 4, 40 + 20*layer_line + 4, GFX.NPC_modes, false, 0, 32*4, 32, 32))
            Events[m_current_event].SpeedY = 0.1f*(sy+1);
        if (UpdateButton(e_ScreenW-120 + 4, 40 + 20*layer_line + 4, GFX.NPC_modes, false, 0, 32*3, 32, 32))
            Events[m_current_event].SpeedY = 0.1f*(sy-1);
        if (UpdateButton(e_ScreenW-80 + 4, 40 + 20*layer_line + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
            Events[m_current_event].SpeedX = 0.1f*(sx-1);
        if (UpdateButton(e_ScreenW-40 + 4, 40 + 20*layer_line + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
            Events[m_current_event].SpeedX = 0.1f*(sx+1);
    }
    if (layer_line == 1)
    {
        SuperPrint("LAYERS:", 3, e_ScreenW-200, 40);
        SuperPrint("NONE", 3, e_ScreenW-200, 40 + 20*layer_line);
    }
    else
    {
        SuperPrint("LAYERS:", 3, e_ScreenW-200, 50);
    }
    if (UpdateButton(e_ScreenW - 240 + 4, 40 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
        m_special_page = SPECIAL_PAGE_EVENT_LAYERS;

    // BOTTOM PANE: sections - background, music, autoscroll
    frmMain.renderRect(0, e_ScreenH - 180, e_ScreenW - 240, 180, 0.6f, 0.6f, 0.8f, 1.0f, true);
    if (m_special_subpage > 0 && UpdateButton(40 + 4, e_ScreenH - 180 + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
        m_special_subpage --;
    if (m_special_subpage < maxSections + 1 && UpdateButton(320 + 4, e_ScreenH - 180 + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
        m_special_subpage ++;
    // description of possibilities
    SuperPrint("KEEP", 3, 132, e_ScreenH - 140);
    SuperPrint("RESET", 3, 226, e_ScreenH - 140);
    SuperPrint("SET", 3, 340, e_ScreenH - 140);

    SuperPrint("MUSIC", 3, 4, e_ScreenH - 110);
    SuperPrint("BG", 3, 4, e_ScreenH - 70);
    SuperPrint("BOUNDS", 3, 4, e_ScreenH - 30);
    // subpage - 1 is the internal section ID; subpage 0 is all sections.
    if (m_special_subpage == 0)
    {
        SuperPrint("ALL SECTIONS", 3, 90, e_ScreenH - 174);
        // music
        bool all_keep = true;
        bool all_reset = true;
        bool all_set = true;
        int set_same = -1;
        for (int s = 0; s <= maxSections; s++)
        {
            if (Events[m_current_event].section[s].music_id != LESet_Nothing)
                all_keep = false;
            if (Events[m_current_event].section[s].music_id != LESet_ResetDefault)
                all_reset = false;
            if (Events[m_current_event].section[s].music_id < 0)
                all_set = false;
            else if (set_same == -1)
                set_same = Events[m_current_event].section[s].music_id;
            else if (set_same != Events[m_current_event].section[s].music_id)
                all_set = false;
        }
        if (UpdateButton(150 + 4, e_ScreenH - 120 + 4, GFX.NPC_modes, all_keep, 0, 32*31, 32, 32) && !all_keep)
        {
            for (int s = 0; s <= maxSections; s++)
                Events[m_current_event].section[s].music_id = LESet_Nothing;
        }
        if (UpdateButton(250 + 4, e_ScreenH - 120 + 4, GFX.NPC_modes, all_reset, 0, 32*8, 32, 32) && !all_reset)
        {
            for (int s = 0; s <= maxSections; s++)
                Events[m_current_event].section[s].music_id = LESet_ResetDefault;
        }
        if (UpdateButton(350 + 4, e_ScreenH - 120 + 4, GFX.ECursor[1], all_set, 0, 0, 32, 32))
            m_special_page = SPECIAL_PAGE_EVENT_MUSIC;
        all_keep = true;
        all_reset = true;
        all_set = true;
        set_same = -1;
        for (int s = 0; s <= maxSections; s++)
        {
            if (Events[m_current_event].section[s].background_id != LESet_Nothing)
                all_keep = false;
            if (Events[m_current_event].section[s].background_id != LESet_ResetDefault)
                all_reset = false;
            if (Events[m_current_event].section[s].background_id < 0)
                all_set = false;
            else if (set_same == -1)
                set_same = Events[m_current_event].section[s].background_id;
            else if (set_same != Events[m_current_event].section[s].background_id)
                all_set = false;
        }
        if (UpdateButton(150 + 4, e_ScreenH - 80 + 4, GFX.NPC_modes, all_keep, 0, 32*31, 32, 32) && !all_keep)
        {
            for (int s = 0; s <= maxSections; s++)
                Events[m_current_event].section[s].background_id = LESet_Nothing;
        }
        if (UpdateButton(250 + 4, e_ScreenH - 80 + 4, GFX.NPC_modes, all_reset, 0, 32*8, 32, 32) && !all_reset)
        {
            for (int s = 0; s <= maxSections; s++)
                Events[m_current_event].section[s].background_id = LESet_ResetDefault;
        }
        if (UpdateButton(350 + 4, e_ScreenH - 80 + 4, GFX.ECursor[1], all_set, 0, 0, 32, 32))
            m_special_page = SPECIAL_PAGE_EVENT_BACKGROUND;
        all_keep = true;
        all_reset = true;
        all_set = true;
        for (int s = 0; s <= maxSections; s++)
        {
            if ((int)Events[m_current_event].section[s].position.X == LESet_Nothing)
            {
                all_reset = false;
                all_set = false;
            }
            else if ((int)Events[m_current_event].section[s].position.X == LESet_ResetDefault)
            {
                all_set = false;
                all_keep = false;
            }
            else
            {
                all_keep = false;
                all_reset = false;
            }
        }
        if (UpdateButton(150 + 4, e_ScreenH - 40 + 4, GFX.NPC_modes, all_keep, 0, 32*31, 32, 32) && !all_keep)
        {
            for (int s = 0; s <= maxSections; s++)
                Events[m_current_event].section[s].position.X = -1;
        }
        if (UpdateButton(250 + 4, e_ScreenH - 40 + 4, GFX.NPC_modes, all_reset, 0, 32*8, 32, 32) && !all_reset)
        {
            for (int s = 0; s <= maxSections; s++)
                Events[m_current_event].section[s].position.X = -2;
        }
        if (UpdateButton(350 + 4, e_ScreenH - 40 + 4, GFX.ECursor[1], all_set, 0, 0, 32, 32))
            m_special_page = SPECIAL_PAGE_EVENT_BOUNDS;
    }
    else
    {
        SuperPrint("SECTION "+std::to_string(m_special_subpage), 3, 120, e_ScreenH - 174);
        if (UpdateButton(150 + 4, e_ScreenH - 120 + 4, GFX.NPC_modes, Events[m_current_event].section[m_special_subpage-1].music_id == LESet_Nothing, 0, 32*31, 32, 32))
        {
            Events[m_current_event].section[m_special_subpage-1].music_id = LESet_Nothing;
        }
        if (UpdateButton(250 + 4, e_ScreenH - 120 + 4, GFX.NPC_modes, Events[m_current_event].section[m_special_subpage-1].music_id == LESet_ResetDefault, 0, 32*8, 32, 32))
        {
            Events[m_current_event].section[m_special_subpage-1].music_id = LESet_ResetDefault;
        }
        if (UpdateButton(350 + 4, e_ScreenH - 120 + 4, GFX.ECursor[1], Events[m_current_event].section[m_special_subpage-1].music_id >= 0, 0, 0, 32, 32))
            m_special_page = SPECIAL_PAGE_EVENT_MUSIC;
        if (UpdateButton(150 + 4, e_ScreenH - 80 + 4, GFX.NPC_modes, Events[m_current_event].section[m_special_subpage-1].background_id == LESet_Nothing, 0, 32*31, 32, 32))
        {
            Events[m_current_event].section[m_special_subpage-1].background_id = LESet_Nothing;
        }
        if (UpdateButton(250 + 4, e_ScreenH - 80 + 4, GFX.NPC_modes, Events[m_current_event].section[m_special_subpage-1].background_id == LESet_ResetDefault, 0, 32*8, 32, 32))
        {
            Events[m_current_event].section[m_special_subpage-1].background_id = LESet_ResetDefault;
        }
        if (UpdateButton(350 + 4, e_ScreenH - 80 + 4, GFX.ECursor[1], Events[m_current_event].section[m_special_subpage-1].background_id >= 0, 0, 0, 32, 32))
            m_special_page = SPECIAL_PAGE_EVENT_BACKGROUND;
        if (UpdateButton(150 + 4, e_ScreenH - 40 + 4, GFX.NPC_modes, (int)Events[m_current_event].section[m_special_subpage-1].position.X == LESet_Nothing, 0, 32*31, 32, 32))
        {
            Events[m_current_event].section[m_special_subpage-1].position.X = LESet_Nothing;
        }
        if (UpdateButton(250 + 4, e_ScreenH - 40 + 4, GFX.NPC_modes, (int)Events[m_current_event].section[m_special_subpage-1].position.X == LESet_ResetDefault, 0, 32*8, 32, 32))
        {
            Events[m_current_event].section[m_special_subpage-1].position.X = LESet_ResetDefault;
        }
        if (UpdateButton(350 + 4, e_ScreenH - 40 + 4, GFX.ECursor[1], (int)Events[m_current_event].section[m_special_subpage-1].position.X != LESet_Nothing && (int)Events[m_current_event].section[m_special_subpage-1].position.X != LESet_ResetDefault, 0, 0, 32, 32))
            m_special_page = SPECIAL_PAGE_EVENT_BOUNDS;
    }

    // autostart
    SuperPrint("AUTOSTART", 3, 54, 90);
    if (UpdateButton(10 + 4, 80 + 4, GFX.NPC_modes, Events[m_current_event].AutoStart, 0, 32*7, 32, 32))
        Events[m_current_event].AutoStart = !Events[m_current_event].AutoStart;
    // sound
    SuperPrint("SOUND", 3, 254, 90);
    if (UpdateButton(210 + 4, 80 + 4, GFX.ECursor[1], Events[m_current_event].Sound != 0, 0, 0, 32, 32))
        m_special_page = SPECIAL_PAGE_EVENT_SOUND;
    // end game
    SuperPrint("END GAME", 3, 54, 130);
    if (UpdateButton(10 + 4, 120 + 4, GFX.NPC_modes, Events[m_current_event].EndGame == 1, 0, 32*7, 32, 32))
        Events[m_current_event].EndGame ^= 1;
    // control lock
    bool controls_set = (Events[m_current_event].Controls.AltJump ||
        Events[m_current_event].Controls.AltRun ||
        Events[m_current_event].Controls.Down ||
        Events[m_current_event].Controls.Drop ||
        Events[m_current_event].Controls.Jump ||
        Events[m_current_event].Controls.Left ||
        Events[m_current_event].Controls.Right ||
        Events[m_current_event].Controls.Run ||
        Events[m_current_event].Controls.Start ||
        Events[m_current_event].Controls.Up);
    SuperPrint("CONTROLS", 3, 254, 130);
    if (UpdateButton(210 + 4, 120 + 4, GFX.ECursor[1], controls_set, 0, 0, 32, 32))
        m_special_page = SPECIAL_PAGE_EVENT_CONTROLS;
    // message
    MessageText = Events[m_current_event].Text;
    SuperPrint("TEXT", 3, 54, 170);
    if (UpdateButton(10 + 4, 160 + 4, GFX.NPC_modes, !Events[m_current_event].Text.empty(), 0, 32*17, 32, 32))
        Events[m_current_event].Text = GetTextInput("Event text", Events[m_current_event].Text);

    // trigger event
    SuperPrint("TRIGGER:", 3, 54, 220);
    if (UpdateButton(10 + 4, 220 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
        m_special_page = SPECIAL_PAGE_EVENT_TRIGGER;
    if (!Events[m_current_event].TriggerEvent.empty())
    {
        SuperPrint(Events[m_current_event].TriggerEvent.substr(0,19), 3, 54, 240);
        if (Events[m_current_event].TriggerDelay > 0)
        {
            SuperPrint("AFTER", 3, 54, 260);
            SuperPrint(std::to_string((int)Events[m_current_event].TriggerDelay*100) + "MS", 3, 54, 280);
        }
        else
            SuperPrint("INSTANT", 3, 54, 260);
        if (Events[m_current_event].TriggerDelay > 0 && UpdateButton(170 + 4, 260 + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
            Events[m_current_event].TriggerDelay --;
        if (UpdateButton(210 + 4, 260 + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
            Events[m_current_event].TriggerDelay ++;
    }
    else
        SuperPrint("NONE", 3, 54, 240);
}

static const std::vector<std::string> display_music_names = {"None", "SMB3 - Overworld", "SMB3 - Sky", "SMB3 - Castle", "SMB3 - Underground", "SMB2 - Overworld", "SMB3 - Battle", "SMB - Underground", "MISC - Corneria", "SMB - Overworld", "SMW - Overworld", "SM - Brinstar", "SM - Crateria", "NSMB", "SM64 - Desert", "SMB2 - Boss", "SM RPG - Forest Maze", "SMW - Ghost House", "SMG - Beach Bowl", "SSBB - Airship Theme", "SMG - Star Reactor", "SM RPG - Bowser Battle", "TDS - Metroid Charge", "ALTTP - Lost Woods", "Custom", "SMB2 - Underground", "SM64 - Castle", "SM64 - Main Theme", "SMW - Sky", "SMW - Cave", "SM RPG - Mario's Pad", "SM RPG - Seaside Town", "SM RPG - Tadpole Pond", "SM RPG - Nimbus Land", "SM RPG - Rose Town", "SM64 - Snow", "SM64 - Boss", "PM - Shiver Mnt", "PM - Yoshi's Village", "SSBB - Hyrule Temple", "SSBB - Meta Knight", "SMW - Castle", "SMB - Castle", "SMB2 - Wart", "SM - Item Room", "SM - Mother Brain", "SMB - Water", "SMB3 - Water", "SMW - Water", "SM64 - Water", "SM64 - Cave", "SMW - Boss", "SSBB - Underground", "SSBB - Waluigi", "SMB3 - Hammer Bros", "SMG - Fleet Glide", "SMK - Mushroom Gorge"};
static const std::vector<std::string> display_backgrounds_names = {"None", "SMB3 - Blocks", "SMB3 - Hills", "SMB3 - Dungeon", "SMB3 - Pipes", "SMB2 - Trees", "SMB3 - Bonus", "SMB - Underground", "SMB - Night", "SMB - Night 2", "SMB - Overworld", "SMW - Hills", "SMW - Trees", "SMB3 - Clouds", "SMB3 - Desert", "SMB3 - Dungeon 2", "SM - Crateria", "SMB3 - Ship", "SMW - Ghost House", "SMW - Forest", "SMB3 - Forest", "SMB3 - Battle", "SMB3 - Waterfall", "SMB3 - Tanks", "SMB3 - Bowser", "SMB2 - Underground", "SMB3 - Toad's House", "SMB3 - Castle", "SMW - Bonus", "SMW - Night", "SMW - Cave", "SMW - Clouds", "SMW - Hills 2", "SMW - Hills 4", "SMW - Hills 3", "SMB3 - Snow Trees", "SMB3 - Clouds 2", "SMB3 - Snow Hills", "SMB3 - Cave", "SMB3 - Cave 2", "STH - Mystic Cave Z.", "SMB - Castle", "SMW - Castle", "SMW - Castle 2", "SMB2 - Castle", "SM - Brinstar", "SM - Transport", "SM - Mother Brain", "SMB2 - Clouds", "SMB2 - Night - Hills", "SMB - Mushrooms", "SMB - Desert", "SMB2 - Night - Desert", "SMB2 - Cliff", "SMB2 - Warehouse", "SMW - Underwater", "SMB3 - Underwater", "SMB2 - Dungeon", "SMW - Desert Night"};

// updates the bounds for Section 0 in level start event according to its autoscroll
void UpdateStartLevelEventBounds()
{
    Events[0].AutoSection = 0;
    Events[0].section[0].position = level[0];
    // not sure why 800 is also used for height in the default code, but I will stick with it.
    if (Events[0].AutoX < 0)
        Events[0].section[0].position.X = Events[0].section[0].position.Width - 800;
    else if (Events[0].AutoX > 0)
        Events[0].section[0].position.Width = Events[0].section[0].position.X + 800;
    if (Events[0].AutoY < 0)
        Events[0].section[0].position.Y = Events[0].section[0].position.Height - 800;
    else if (Events[0].AutoY > 0)
        Events[0].section[0].position.Height = Events[0].section[0].position.Y + 800;
}

void EditorScreen::UpdateSectionsScreen()
{
    // level settings
    if (UpdateButton(10 + 4, 40 + 4, GFX.NPC_modes, false, 0, 32*17, 32, 32))
        LevelName = GetTextInput("Level name", LevelName);
    SuperPrint("LEVEL NAME:", 3, 54, 42);
    if (!LevelName.empty())
        SuperPrint(LevelName, 3, 54, 60);
    else
        SuperPrint("NONE", 3, 54, 60);

    SuperPrint("SET START:", 3, 10, 110);
    if (UpdateButton(240, 100, GFXBlock[622], EditorCursor.SubMode == 4, 0, 0, 32, 32))
        EditorCursor.SubMode = 4;
    if (UpdateButton(280, 100, GFXBlock[623], EditorCursor.SubMode == 5, 0, 0, 32, 32))
        EditorCursor.SubMode = 5;

    // section settings
    SuperPrint("SECTION "+std::to_string(curSection+1), 3, 240, 166);
    if (curSection > 0 && UpdateButton(160 + 4, 160 + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
        SetSection(curSection - 1);
    if (curSection < maxSections && UpdateButton(440 + 4, 160 + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
        SetSection(curSection + 1);

    // music
    if (UpdateButton(10 + 4, 280 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
        m_special_page = SPECIAL_PAGE_SECTION_MUSIC;
    SuperPrint("MUSIC: "+display_music_names[bgMusic[curSection]], 3, 54, 286);
    if (bgMusic[curSection] == 24)
    {
        if (CustomMusic[curSection].length() < 15)
            SuperPrint(CustomMusic[curSection], 3, 374, 292);
        else
        {
            SuperPrint(CustomMusic[curSection].substr(0,14), 3, 374, 282);
            SuperPrint(CustomMusic[curSection].substr(14,14), 3, 374, 300);
        }
        if (UpdateButton(330 + 4, 280 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
            StartFileBrowser(&CustomMusic[curSection], FileNamePath, "", {".mp3", ".ogg", ".spc", ".vgm"}, BROWSER_MODE_OPEN, BROWSER_CALLBACK_CUSTOM_MUSIC);
    }

    // background
    if (UpdateButton(10 + 4, 240 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
        m_special_page = SPECIAL_PAGE_SECTION_BACKGROUND;
    SuperPrint("BG: "+display_backgrounds_names[Background2[curSection]], 3, 54, 246);

    // set bounds
    SuperPrint("SET BOUNDS:", 3, 10, 206);
    if (UpdateButton(240, 200, GFX.NPC_modes, EditorCursor.SubMode == 0, 0, 32*3, 32, 32))
        EditorCursor.SubMode = 0;
    if (UpdateButton(280, 200, GFX.NPC_modes, EditorCursor.SubMode == 3, 0, 32*4, 32, 32))
        EditorCursor.SubMode = 3;
    if (UpdateButton(320, 200, GFX.NPC_modes, EditorCursor.SubMode == 1, 0, 32*1, 32, 32))
        EditorCursor.SubMode = 1;
    if (UpdateButton(360, 200, GFX.NPC_modes, EditorCursor.SubMode == 2, 0, 32*2, 32, 32))
        EditorCursor.SubMode = 2;

    // hwrap - LevelWrap
    if (UpdateButton(10 + 4, 320 + 4, GFX.NPC_modes, LevelWrap[curSection], 0, 32*7, 32, 32))
        LevelWrap[curSection] = !LevelWrap[curSection];
    SuperPrint("HORIZ. WRAP", 3, 54, 326);
    // underwater - UnderWater
    if (UpdateButton(e_ScreenW/2 + 10 + 4, 320 + 4, GFX.NPC_modes, UnderWater[curSection], 0, 32*7, 32, 32))
        UnderWater[curSection] = !UnderWater[curSection];
    SuperPrint("UNDERWATER", 3, e_ScreenW/2 + 54, 326);
    // no turn back - NoTurnBack
    if (UpdateButton(10 + 4, 360 + 4, GFX.NPC_modes, NoTurnBack[curSection], 0, 32*7, 32, 32))
        NoTurnBack[curSection] = !NoTurnBack[curSection];
    SuperPrint("NO TURN BACK", 3, 54, 366);
    // leave to exit - OffScreenExit
    if (UpdateButton(e_ScreenW/2 + 10 + 4, 360 + 4, GFX.NPC_modes, OffScreenExit[curSection], 0, 32*7, 32, 32))
        OffScreenExit[curSection] = !OffScreenExit[curSection];
    SuperPrint("LEAVE TO EXIT", 3, e_ScreenW/2 + 54, 366);

    // moved autoscroll into level settings, and only allow section 0 / event 0 (level start).
    // this is due to an awful bug which couldn't be fixed if people had taken
    // advantage of the ridiculously incorrect original behavior.
    if (curSection == 0)
    {
        SuperPrint("SCROLL:", 3, 10, 430);
        int sy = (vb6Round)(Events[0].AutoY*10);
        int sx = (vb6Round)(Events[0].AutoX*10);
        if (sy < 0)
            SuperPrint("U"+std::to_string(-sy), 3, 180, 422);
        else if (sy > 0)
            SuperPrint("D"+std::to_string(sy), 3, 180, 422);
        else
            SuperPrint("-", 3, 180, 422);
        if (sx < 0)
            SuperPrint("L"+std::to_string(-sx), 3, 180, 440);
        else if (sx > 0)
            SuperPrint("R"+std::to_string(sx), 3, 180, 440);
        else
            SuperPrint("-", 3, 180, 440);
        if (UpdateButton(240 + 4, 420 + 4, GFX.NPC_modes, false, 0, 32*3, 32, 32))
        {
            Events[0].AutoY = 0.1f*(sy-1);
            UpdateStartLevelEventBounds();
        }
        if (UpdateButton(280 + 4, 420 + 4, GFX.NPC_modes, false, 0, 32*4, 32, 32))
        {
            Events[0].AutoY = 0.1f*(sy+1);
            UpdateStartLevelEventBounds();
        }
        if (UpdateButton(320 + 4, 420 + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
        {
            Events[0].AutoX = 0.1f*(sx-1);
            UpdateStartLevelEventBounds();
        }
        if (UpdateButton(360 + 4, 420 + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
        {
            Events[0].AutoX = 0.1f*(sx+1);
            UpdateStartLevelEventBounds();
        }
    }
}

void EditorScreen::UpdateWorldSettingsScreen()
{
    // world name
    if (UpdateButton(10 + 4, 40 + 4, GFX.NPC_modes, false, 0, 32*17, 32, 32))
        WorldName = GetTextInput("World name", WorldName);
    SuperPrint("WORLD NAME:", 3, 54, 42);
    if (!WorldName.empty())
        SuperPrint(WorldName, 3, 54, 60);
    else
        SuperPrint("NONE", 3, 54, 60);

    // auto start level
    if (UpdateButton(10 + 4, 100 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
        StartFileBrowser(&StartLevel, FileNamePath, "", {".lvl"}, BROWSER_MODE_OPEN);
    SuperPrint("AUTO START LEVEL:", 3, 54, 102);
    if (!StartLevel.empty())
        SuperPrint(StartLevel, 3, 54, 120);
    else
        SuperPrint("NONE", 3, 54, 120);

    // no world map - NoMap
    if (UpdateButton(10 + 4, 160 + 4, GFX.NPC_modes, NoMap, 0, 32*7, 32, 32))
        NoMap = !NoMap;
    SuperPrint("NO WORLD MAP", 3, 54, 170);
    // restart after death - RestartLevel
    if (UpdateButton(e_ScreenW/2 + 10 + 4, 160 + 4, GFX.NPC_modes, RestartLevel, 0, 32*7, 32, 32))
        RestartLevel = !RestartLevel;
    SuperPrint("RESTART", 3, e_ScreenW/2 + 54, 162);
    SuperPrint("ON DEATH", 3, e_ScreenW/2 + 54, 180);

    if (MaxWorldStars > 0 && UpdateButton(120 + 4, 220 + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
        MaxWorldStars --;
    SuperPrint("TOTAL STARS: " + std::to_string(MaxWorldStars), 3, 170, 230);
    if (UpdateButton(440 + 4, 220 + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
        MaxWorldStars ++;

    SuperPrint("ALLOW CHARS:", 3, 10, 290);
    if (UpdateButton(240 + 4, 280 + 4, GFXBlock[622], !blockCharacter[1], 0, 0, 32, 32))
        blockCharacter[1] = !blockCharacter[1];
    if (UpdateButton(280 + 4, 280 + 4, GFXBlock[623], !blockCharacter[2], 0, 0, 32, 32))
        blockCharacter[2] = !blockCharacter[2];
    if (UpdateButton(320 + 4, 280 + 4, GFXBlock[624], !blockCharacter[3], 0, 0, 32, 32))
        blockCharacter[3] = !blockCharacter[3];
    if (UpdateButton(360 + 4, 280 + 4, GFXBlock[625], !blockCharacter[4], 0, 0, 32, 32))
        blockCharacter[4] = !blockCharacter[4];
    if (UpdateButton(400 + 4, 280 + 4, GFXBlock[631], !blockCharacter[5], 0, 0, 32, 32))
        blockCharacter[5] = !blockCharacter[5];

    // don't allow all characters to be blocked.
    if (blockCharacter[1] && blockCharacter[2] && blockCharacter[3] && blockCharacter[4] && blockCharacter[5])
        blockCharacter[1] = false;

    // credits...
    if (m_special_subpage > 0 && UpdateButton(10 + 4, 340 + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
        m_special_subpage --;
    if (UpdateButton(50 + 4, 340 + 4, GFX.NPC_modes, false, 0, 32*17, 32, 32))
        WorldCredits[m_special_subpage+1] = GetTextInput("Credits", WorldCredits[m_special_subpage+1]);
    if (m_special_subpage < 4 && UpdateButton(90 + 4, 340 + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
        m_special_subpage ++;
    SuperPrint("WORLD CREDITS LINE "+std::to_string(m_special_subpage+1)+":", 3, 144, 342);
    SuperPrint(WorldCredits[m_special_subpage+1], 3, 144, 360);
}

static const std::vector<std::string> list_sounds_names = {"None", "Jump", "Stomp", "Block Hit", "Block Smashed", "Shrink", "Grow", "Mushroom", "Player Died", "Shell Kick", "Skid", "Drop Item", "Got Item", "Camera", "Coin", "1up", "Lava", "Warp", "Fireball", "SMB3 Exit", "Defeat Boss", "Dungeon Clear ", "Bullet Bill", "Grab", "Spring", "Hammer Toss", "Slide", "New Path", "Level Select", "Do", "Pause", "Key", "Switch", "Tail", "Racoon", "Boot", "Smash", "Thwomp", "Birdo Spit", "Birdo Hit", "SMB2 Exit", "Birdo Beat", "Big Fireball", "Fireworks", "Bowser Killed", "SMB3 Game Beat", "Door", "Message", "Yoshi", "Yoshi Hurt", "Yoshi Tongue", "Yoshi Egg", "Got Star Exit", "Zelda Kill", "Player Died 2", "Yoshi Swallow", "Sonic Ring", "Dry Bones", "SMW Checkpoint", "Dragon Coin", "SMW Exit", "Blaarg", "Wart Bubbles", "Wart Killed", "SM Block Hit", "SM Killed", "SM Hurt", "SM Glass", "SM Boss Hit", "SM Cry", "SM Explosion", "Climbing", "Swim", "Light Grab", "Saw", "SMB2 Throw", "SMB2 Hit", "Zelda Stab", "Zelda Hurt", "Zelda Heart", "Zelda Died", "Zelda Rupee", "Zelda Fire", "Zelda Item", "Zelda Key", "Zelda Shield", "Zelda Dash", "Zelda Fairy", "Zelda Grass"};
static const std::vector<std::string> list_backgrounds_names = {"None", "SMB", "Underground", "Night", "Night 2", "Overworld", "Castle", "Mushrooms", "Desert", "", "SMB 2", "Trees", "Underground", "Castle", "Clouds", "Night - Hills", "Night - Desert", "Cliff", "Warehouse", "Dungeon", "SMB 3", "Blocks", "Hills", "Dungeon", "Pipes", "Bonus", "Clouds", "Desert", "Dungeon 2", "Ship", "Forest", "Battle", "Waterfall", "Tanks", "Bowser", "Toad's House", "Castle", "Snow Trees", "Clouds 2", "Snow Hills", "Cave", "Cave 2", "Underwater", "SMW", "Trees", "Ghost House", "Forest", "Bonus", "Night", "Cave", "Clouds", "Hills", "Hills 2", "Hills 4", "Hills 3", "Castle", "Castle 2", "Underwater", "Desert Night", "", "Misc.", "Mother Brain", "Transport", "Brinstar", "Crateria", "Mystic Cave Z."};
static const std::vector<int> list_backgrounds_indices = {0, -1, 7, 8, 9, 10, 41, 50, 51, -1, -1, 5, 25, 44, 48, 49, 52, 53, 54, 57, -1, 1, 2, 3, 4, 6, 13, 14, 15, 17, 20, 21, 22, 23, 24, 26, 27, 35, 36, 37, 38, 39, 56, -1, 12, 18, 19, 28, 29, 30, 31, 11, 32, 33, 34, 42, 43, 55, 58, -1, -1, 47, 46, 45, 16, 40};
static const std::vector<std::string> list_music_names = {"None", "Custom", "SMB", "Overworld", "Underground", "Castle", "Water", "SMB 2", "Overworld", "Underground", "Boss", "Wart", "SMB 3", "Overworld", "Sky", "Underground", "Castle", "Water", "Hammer Bros", "Battle", "SMW", "Overworld", "Ghost House", "Sky", "Cave", "Castle", "Water", "Boss", "SM RPG", "Mario's Pad", "Rose Town", "Forest Maze", "Seaside Town", "Tadpole Pond", "Nimbus Land", "Bowser Battle", "SM64", "Main Theme", "Cave", "Snow", "Desert", "Water", "Castle", "Boss", "SSBB", "Meta Knight", "Underground", "Hyrule Temple", "Airship Theme", "Waluigi", "Metroid", "Brinstar", "Crateria", "Item Room", "Mother Brain", "Metroid Charge", "Misc.", "Yoshi's Village", "Shiver Mnt", "Fleet Glide", "Beach Bowl", "Star Reactor", "Mushroom Gorge", "New SMB", "Lost Woods", "Corneria"};
static const std::vector<int> list_music_indices = {0, 24, -1, 9, 7, 42, 46, -1, 5, 25, 15, 43, -1, 1, 2, 4, 3, 47, 54, 6, -1, 10, 17, 28, 29, 41, 48, 51, -1, 30, 34, 16, 31, 32, 33, 21, -1, 27, 50, 35, 14, 49, 26, 36, -1, 40, 52, 39, 19, 53, -1, 11, 12, 44, 45, 22, -1, 38, 37, 55, 18, 20, 56, 13, 23, 8};
static const std::vector<std::string> list_world_music_names = {"None", "SMB3", "SMB3 World 1", "SMB3 World 2", "SMB3 World 3", "SMB3 World 4", "SMB3 World 5", "SMB3 World 6", "SMB3 World 7", "SMB3 World 8", "SMW", "SMW Theme", "SMW Cave", "SMW Island", "SMW Forest", "SMW Bowser", "SMW Star Road", "SMW Special", "NSMB", "NSMB Theme"};
static const std::vector<int> list_world_music_indices = {0, -1, 1, 6, 8, 2, 11, 10, 3, 9, -1, 4, 16, 15, 7, 13, 14, 12, -1, 5};
static const std::vector<std::string> list_level_exit_names = {"ANY", "NONE", "SMB3 END", "SMB3 ORB", "LEAVE", "KEYHOLE", "SMB2 ORB", "WARP", "STAR", "SMW END"};

void EditorScreen::UpdateSelectListScreen()
{
    if (m_special_page != SPECIAL_PAGE_NONE && UpdateButton(e_ScreenW - 40 + 4, 40 + 4, GFX.NPC_modes, false, 0, 32*8, 32, 32))
    {
        if (m_special_page == SPECIAL_PAGE_EVENT_SOUND || m_special_page == SPECIAL_PAGE_EVENT_MUSIC || m_special_page == SPECIAL_PAGE_EVENT_BACKGROUND)
            m_special_page = SPECIAL_PAGE_EVENT_SETTINGS;
        else
            m_special_page = SPECIAL_PAGE_NONE;
    }
    int* target;
    int* current_page;
    const std::vector<std::string>* source;
    const std::vector<int>* source_indices;
    if (m_special_page == SPECIAL_PAGE_EVENT_SOUND)
    {
        SuperPrint("SOUND FOR", 3, 10, 40);
        SuperPrint(Events[m_current_event].Name, 3, 10, 60);
        target = &Events[m_current_event].Sound;
        current_page = &m_sounds_page;
        source = &list_sounds_names;
        source_indices = nullptr;
    }
    else if (m_special_page == SPECIAL_PAGE_EVENT_BACKGROUND)
    {
        if (m_special_subpage > 0)
        {
            SuperPrint("SECT " + std::to_string(m_special_subpage) + " BG FOR", 3, 10, 40);
            target = &Events[m_current_event].section[m_special_subpage-1].background_id;
        }
        else
        {
            SuperPrint("ALL SECT BG FOR", 3, 10, 40);
            target = nullptr;
        }
        SuperPrint(Events[m_current_event].Name, 3, 10, 60);
        current_page = &m_background_page;
        source = &list_backgrounds_names;
        source_indices = &list_backgrounds_indices;
    }
    else if (m_special_page == SPECIAL_PAGE_EVENT_MUSIC)
    {
        if (m_special_subpage > 0)
        {
            SuperPrint("SECT " + std::to_string(m_special_subpage) + " MUSIC FOR", 3, 10, 40);
            target = &Events[m_current_event].section[m_special_subpage-1].music_id;
        }
        else
        {
            SuperPrint("ALL SECT MUSIC FOR", 3, 10, 40);
            target = nullptr;
        }
        SuperPrint(Events[m_current_event].Name, 3, 10, 60);
        current_page = &m_music_page;
        source = &list_music_names;
        source_indices = &list_music_indices;
    }
    else if (m_special_page == SPECIAL_PAGE_SECTION_BACKGROUND)
    {
        SuperPrint("SECTION " + std::to_string(curSection + 1) + " BG", 3, 10, 40);
        target = &Background2[curSection];
        current_page = &m_background_page;
        source = &list_backgrounds_names;
        source_indices = &list_backgrounds_indices;
    }
    else if (m_special_page == SPECIAL_PAGE_SECTION_MUSIC)
    {
        SuperPrint("SECTION " + std::to_string(curSection + 1) + " MUSIC", 3, 10, 40);
        target = &bgMusic[curSection];
        current_page = &m_music_page;
        source = &list_music_names;
        source_indices = &list_music_indices;
    }
    else if (m_special_page == SPECIAL_PAGE_LEVEL_EXIT)
    {
        if (m_special_subpage == 1)
            SuperPrint("UPPER PATH UNLOCK", 3, 10, 40);
        else if (m_special_subpage == 2)
            SuperPrint("LEFT PATH UNLOCK", 3, 10, 40);
        else if (m_special_subpage == 3)
            SuperPrint("LOWER PATH UNLOCK", 3, 10, 40);
        else if (m_special_subpage == 4)
            SuperPrint("RIGHT PATH UNLOCK", 3, 10, 40);
        else
            return;
        target = &EditorCursor.WorldLevel.LevelExit[m_special_subpage];
        current_page = nullptr;
        source = &list_level_exit_names;
        source_indices = nullptr;
    }
    else if (EditorCursor.Mode == OptCursor_t::WLD_MUSIC)
    {
        SuperPrint("WORLD MUSIC", 3, 10, 40);
        target = &EditorCursor.WorldMusic.Type;
        current_page = nullptr;
        source = &list_world_music_names;
        source_indices = &list_world_music_indices;
    }
    else
    {
        return;
    }

    if (current_page != nullptr)
    {
        int page_max = (source->size() - 1) / 20;
        if (!(page_max == 0 && *current_page == 0))
        SuperPrint("PAGE " + std::to_string(*current_page+1) + " OF " + std::to_string(page_max+1), 3, e_ScreenW - 320, 40);
        if (*current_page > 0 && UpdateButton(e_ScreenW - 120 + 4, 40 + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
            *current_page = *current_page - 1;
        if (*current_page < page_max && UpdateButton(e_ScreenW - 80 + 4, 40 + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
            *current_page = *current_page + 1;
    }

    for (int i = 0; i < 20; i++)
    {
        int x = 10 + (e_ScreenW/2)*(i/10);
        int y = 80 + 40*(i%10);

        int j;
        if (current_page != nullptr)
            j = *current_page*20 + i;
        else
            j = i;
        if (j >= source->size())
            break;

        int index;
        if (source_indices != nullptr)
            index = (*source_indices)[j];
        else if (m_special_page == SPECIAL_PAGE_LEVEL_EXIT)
            index = j - 1;
        else
            index = j;

        if (index == -1 && m_special_page != SPECIAL_PAGE_LEVEL_EXIT)
            x += 40;

        SuperPrint((*source)[j], 3, x + 44, y + 10);
        if (index != -1 || m_special_page == SPECIAL_PAGE_LEVEL_EXIT)
        {
            bool sel;
            if (target != nullptr)
                sel = *target == index;
            else if (m_special_page == SPECIAL_PAGE_EVENT_BACKGROUND)
            {
                sel = true;
                for (int s = 0; s <= maxSections; s++)
                {
                    if (Events[m_current_event].section[s].background_id != index)
                        sel = false;
                }
            }
            else if (m_special_page == SPECIAL_PAGE_EVENT_MUSIC)
            {
                sel = true;
                for (int s = 0; s <= maxSections; s++)
                {
                    if (Events[m_current_event].section[s].music_id != index)
                        sel = false;
                }
            }
            else // should never happen
            {
                return;
            }

            if (UpdateButton(x + 4, y + 4, GFX.ECursor[2], sel, 0, 0, 32, 32) && !sel)
            {
                if (target != nullptr)
                    *target = index;
                else if (m_special_page == SPECIAL_PAGE_EVENT_BACKGROUND)
                {
                    for (int s = 0; s <= maxSections; s++)
                        Events[m_current_event].section[s].background_id = index;
                }
                else if (m_special_page == SPECIAL_PAGE_EVENT_MUSIC)
                {
                    for (int s = 0; s <= maxSections; s++)
                        Events[m_current_event].section[s].music_id = index;
                }
                // and do whatever necessary to preview it.
                if (m_special_page == SPECIAL_PAGE_EVENT_SOUND && index != 0)
                    PlaySound(index);
                else if (m_special_page == SPECIAL_PAGE_SECTION_MUSIC)
                    StartMusic(curSection);
            }
        }
    }
}

void EditorScreen::UpdateEventsSubScreen()
{
    // render shared GUI elements on right
    frmMain.renderRect(e_ScreenW - 240, 40, 240, e_ScreenH - 40, 0.7f, 0.7f, 0.9f, 1.0f, true);
    if (UpdateButton(e_ScreenW - 40 + 4, 40 + 4, GFX.NPC_modes, false, 0, 32*8, 32, 32))
    {
        if (m_special_page == SPECIAL_PAGE_EVENT_TRIGGER)
            m_special_page = SPECIAL_PAGE_EVENT_SETTINGS;
        else
            m_special_page = SPECIAL_PAGE_NONE;
        m_special_subpage = 0;
    }
    if (m_special_page == SPECIAL_PAGE_EVENT_TRIGGER)
    {
        if (Events[m_current_event].TriggerDelay > 0 && UpdateButton(e_ScreenW - 160 + 4, 200 + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
            Events[m_current_event].TriggerDelay --;
        if (UpdateButton(e_ScreenW - 120 + 4, 200 + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
            Events[m_current_event].TriggerDelay ++;
    }
    else if (EditorCursor.Mode == OptCursor_t::LVL_NPCS)
    {
        SuperPrint("ACTIVATE:", 3, e_ScreenW - 200, 200 + 2);
        if (!EditorCursor.NPC.TriggerActivate.empty())
            SuperPrint(EditorCursor.NPC.TriggerActivate, 3, e_ScreenW - 200, 220 + 2);
        else
            SuperPrint("NONE", 3, e_ScreenW - 200, 220 + 2);
        if (UpdateButton(e_ScreenW - 240 + 4, 200 + 4, GFX.ECursor[1], m_special_subpage == 1, 0, 0, 32, 32))
            m_special_subpage = 1;

        SuperPrint("DEATH:", 3, e_ScreenW - 200, 240 + 2);
        if (!EditorCursor.NPC.TriggerDeath.empty())
            SuperPrint(EditorCursor.NPC.TriggerDeath, 3, e_ScreenW - 200, 260 + 2);
        else
            SuperPrint("NONE", 3, e_ScreenW - 200, 260 + 2);
        if (UpdateButton(e_ScreenW - 240 + 4, 240 + 4, GFX.ECursor[1], m_special_subpage == 2, 0, 0, 32, 32))
            m_special_subpage = 2;

        SuperPrint("TALK:", 3, e_ScreenW - 200, 280 + 2);
        if (!EditorCursor.NPC.TriggerTalk.empty())
            SuperPrint(EditorCursor.NPC.TriggerTalk, 3, e_ScreenW - 200, 300 + 2);
        else
            SuperPrint("NONE", 3, e_ScreenW - 200, 300 + 2);
        if (UpdateButton(e_ScreenW - 240 + 4, 280 + 4, GFX.ECursor[1], m_special_subpage == 3, 0, 0, 32, 32))
            m_special_subpage = 3;

        SuperPrint("LAYER CLEAR:", 3, e_ScreenW - 200, 320 + 2);
        if (!EditorCursor.NPC.TriggerLast.empty())
            SuperPrint(EditorCursor.NPC.TriggerLast, 3, e_ScreenW - 200, 340 + 2);
        else
            SuperPrint("NONE", 3, e_ScreenW - 200, 340 + 2);
        if (UpdateButton(e_ScreenW - 240 + 4, 320 + 4, GFX.ECursor[1], m_special_subpage == 4, 0, 0, 32, 32))
            m_special_subpage = 4;
    }
    else if (EditorCursor.Mode == OptCursor_t::LVL_BLOCKS)
    {
        SuperPrint("HIT:", 3, e_ScreenW - 200, 200 + 2);
        if (!EditorCursor.Block.TriggerHit.empty())
            SuperPrint(EditorCursor.Block.TriggerHit, 3, e_ScreenW - 200, 220 + 2);
        else
            SuperPrint("NONE", 3, e_ScreenW - 200, 220 + 2);
        if (UpdateButton(e_ScreenW - 240 + 4, 200 + 4, GFX.ECursor[1], m_special_subpage == 1, 0, 0, 32, 32))
            m_special_subpage = 1;

        SuperPrint("DESTROY:", 3, e_ScreenW - 200, 240 + 2);
        if (!EditorCursor.Block.TriggerDeath.empty())
            SuperPrint(EditorCursor.Block.TriggerDeath, 3, e_ScreenW - 200, 260 + 2);
        else
            SuperPrint("NONE", 3, e_ScreenW - 200, 260 + 2);
        if (UpdateButton(e_ScreenW - 240 + 4, 240 + 4, GFX.ECursor[1], m_special_subpage == 2, 0, 0, 32, 32))
            m_special_subpage = 2;

        SuperPrint("LAYER CLEAR:", 3, e_ScreenW - 200, 280 + 2);
        if (!EditorCursor.Block.TriggerLast.empty())
            SuperPrint(EditorCursor.Block.TriggerLast, 3, e_ScreenW - 200, 300 + 2);
        else
            SuperPrint("NONE", 3, e_ScreenW - 200, 300 + 2);
        if (UpdateButton(e_ScreenW - 240 + 4, 280 + 4, GFX.ECursor[1], m_special_subpage == 3, 0, 0, 32, 32))
            m_special_subpage = 3;
    }
    else
        return;
    int page_max = numEvents / 10;
    SuperPrint("PAGE " + std::to_string(m_events_page+1) + " OF " + std::to_string(page_max+1), 3, e_ScreenW - 228, e_ScreenH - 60);
    if (m_events_page > 0 && UpdateButton(e_ScreenW - 160 + 4, e_ScreenH - 40 + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
        m_events_page --;
    if (m_events_page < page_max && UpdateButton(e_ScreenW - 120 + 4, e_ScreenH - 40 + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
        m_events_page ++;

    // prepare selector
    std::string event_name;
    std::string event_desc;
    std::string event_desc_2;
    std::string event_desc_3;
    std::string* event_to_set;
    if (m_special_page == SPECIAL_PAGE_EVENT_TRIGGER)
    {
        event_name = "NEXT";
        if (Events[m_current_event].TriggerDelay == 0)
            event_desc = "JUST AFTER";
        else
            event_desc = std::to_string((int)Events[m_current_event].TriggerDelay*100) + "MS AFTER";
        event_desc_2 = Events[m_current_event].Name;
        event_desc_3 = "OCCURS";
        event_to_set = &Events[m_current_event].TriggerEvent;
    }
    else if (EditorCursor.Mode == OptCursor_t::LVL_NPCS)
    {
        if (m_special_subpage == 1)
        {
            event_name = "ACTIVATE";
            event_desc = "NPC ENTERS";
            event_desc_2 = "THE SCREEN";
            event_to_set = &EditorCursor.NPC.TriggerActivate;
        }
        else if (m_special_subpage == 2)
        {
            event_name = "DEATH";
            event_desc = "NPC DIES";
            event_to_set = &EditorCursor.NPC.TriggerDeath;
        }
        else if (m_special_subpage == 3)
        {
            event_name = "TALK";
            event_desc = "PLAYER TALKS";
            event_desc_2 = "TO NPC";
            event_to_set = &EditorCursor.NPC.TriggerTalk;
        }
        else if (m_special_subpage == 4)
        {
            event_name = "LAYER CLEAR";
            event_desc = "EVERYTHING IN";
            event_desc_2 = "NPC'S LAYER";
            event_desc_3 = "HAS DIED";
            event_to_set = &EditorCursor.NPC.TriggerLast;
        }
        else
            return;
    }
    else if (EditorCursor.Mode == OptCursor_t::LVL_BLOCKS)
    {
        if (m_special_subpage == 1)
        {
            event_name = "HIT";
            event_desc = "BLOCK IS HIT";
            event_to_set = &EditorCursor.Block.TriggerHit;
        }
        else if (m_special_subpage == 2)
        {
            event_name = "DESTROY";
            event_desc = "BLOCK IS";
            event_desc_2 = "DESTROYED";
            event_to_set = &EditorCursor.Block.TriggerDeath;
        }
        else if (m_special_subpage == 3)
        {
            event_name = "LAYER CLEAR";
            event_desc = "EVERYTHING IN";
            event_desc_2 = "BLOCK LAYER";
            event_desc_3 = "HAS DIED";
            event_to_set = &EditorCursor.Block.TriggerLast;
        }
        else
            return;
    }
    else // this should not happen
    {
        return;
    }

    // render description
    SuperPrint(event_name, 3, e_ScreenW - 236, 80);
    SuperPrint("TRIGGERS WHEN", 3, e_ScreenW - 236, 100);
    SuperPrint(event_desc, 3, e_ScreenW - 236, 120);
    SuperPrint(event_desc_2, 3, e_ScreenW - 236, 140);
    SuperPrint(event_desc_3, 3, e_ScreenW - 236, 160);

    // render current event
    SuperPrint(event_name + " EVENT:", 3, 10, 40);
    if (event_to_set->empty())
        SuperPrint("NONE", 3, 10, 56);
    else
        SuperPrint(*event_to_set, 3, 10, 56);

    // render event selector
    for (int i = 0; i < 10; i++)
    {
        int e = m_events_page*10 + i - 1;
        if (e == -1)
        {
            SuperPrint("NONE", 3, 54, 80 + 40*i + 12);
            if (UpdateButton(10 + 4, 80 + 40*i + 4, GFX.ECursor[2], event_to_set->empty(), 0, 0, 32, 32))
                *event_to_set = "";
        }
        else if (!Events[e].Name.empty())
        {
            if (Events[e].Name.length() < 20)
                SuperPrint(Events[e].Name, 3, 54, 80 + 40*i + 10);
            else
            {
                SuperPrint(Events[e].Name.substr(0,19), 3, 54, 80 + 40*i + 2);
                SuperPrint(Events[e].Name.substr(19), 3, 54, 80 + 40*i + 20);
            }
            if (UpdateButton(10 + 4, 80 + 40*i + 4, GFX.ECursor[2], (*event_to_set) == Events[e].Name, 0, 0, 32, 32))
                *event_to_set = Events[e].Name;
        }
    }
}

void EditorScreen::UpdateLayersScreen()
{
    if (m_special_page == SPECIAL_PAGE_LAYER_DELETION)
    {
        SuperPrint("DELETING LAYER " + Layer[m_special_subpage].Name, 3, 60, 40);
        SuperPrint("PRESERVE LAYER CONTENTS?", 3, 10, 60);
        SuperPrint("YES: MOVE TO DEFAULT LAYER", 3, 60, 110);
        if (UpdateButton(20 + 4, 100 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
        {
            DeleteLayer(Layer[m_special_subpage].Name, false);
            m_special_subpage = 0;
            m_special_page = SPECIAL_PAGE_LAYERS;
        }
        SuperPrint("NO: *DELETE ALL CONTENTS*", 3, 60, 150);
        if (UpdateButton(20 + 4, 140 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
        {
            DeleteLayer(Layer[m_special_subpage].Name, true);
            m_special_subpage = 0;
            m_special_page = SPECIAL_PAGE_LAYERS;
        }
        SuperPrint("CANCEL: DO NOT DELETE " + Layer[m_special_subpage].Name, 3, 60, 190);
        if (UpdateButton(20 + 4, 180 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
        {
            m_special_subpage = 0;
            m_special_page = SPECIAL_PAGE_LAYERS;
        }
        return;
    }

    // render shared GUI elements on right
    if (m_special_page != SPECIAL_PAGE_LAYERS && m_special_page != SPECIAL_PAGE_EVENT_LAYERS)
        frmMain.renderRect(e_ScreenW - 240, 40, 240, e_ScreenH - 40, 0.7f, 0.7f, 0.9f, 1.0f, true);
    if (m_special_page != SPECIAL_PAGE_LAYERS && UpdateButton(e_ScreenW - 40 + 4, 40 + 4, GFX.NPC_modes, false, 0, 32*8, 32, 32))
    {
        if (m_special_page == SPECIAL_PAGE_EVENT_LAYERS)
            m_special_page = SPECIAL_PAGE_EVENT_SETTINGS;
        else
            m_special_page = SPECIAL_PAGE_NONE;
        m_special_subpage = 0;
    }
    if (m_special_page == SPECIAL_PAGE_OBJ_LAYER && EditorCursor.Mode == OptCursor_t::LVL_NPCS)
    {
        SuperPrint("LAYER:", 3, e_ScreenW - 200, 200 + 2);
        if (EditorCursor.Layer.empty())
            SuperPrint("DEFAULT", 3, e_ScreenW - 200, 220 + 2);
        else
            SuperPrint(EditorCursor.Layer, 3, e_ScreenW - 200, 220 + 2);
        if (UpdateButton(e_ScreenW - 240 + 4, 200 + 4, GFX.ECursor[1], m_special_subpage == 0, 0, 0, 32, 32))
            m_special_subpage = 0;

        SuperPrint("ATTACHED:", 3, e_ScreenW - 200, 260 + 2);
        if (EditorCursor.NPC.AttLayer.empty())
            SuperPrint("NONE", 3, e_ScreenW - 200, 280 + 2);
        else
            SuperPrint(EditorCursor.NPC.AttLayer, 3, e_ScreenW - 200, 280 + 2);
        if (UpdateButton(e_ScreenW - 240 + 4, 260 + 4, GFX.ECursor[1], m_special_subpage == 1, 0, 0, 32, 32))
            m_special_subpage = 1;
    }

    int page_max;
    // extra slot for layer creation when not in the object layer selection
    if (m_special_page == SPECIAL_PAGE_OBJ_LAYER && m_special_subpage == 0)
        page_max = (numLayers - 1) / 10;
    else
        page_max = numLayers / 10;

    // different location for layers and event layers page because they has so many options
    if (m_special_page == SPECIAL_PAGE_LAYERS || m_special_page == SPECIAL_PAGE_EVENT_LAYERS)
    {
        SuperPrint("PAGE " + std::to_string(m_layers_page+1) + " OF " + std::to_string(page_max+1), 3, e_ScreenW - 330, 40);
        if (m_layers_page > 0 && UpdateButton(e_ScreenW - 120 + 4, 40 + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
            m_layers_page --;
        if (m_layers_page < page_max && UpdateButton(e_ScreenW - 80 + 4, 40 + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
            m_layers_page ++;
    }
    else
    {
        SuperPrint("PAGE " + std::to_string(m_layers_page+1) + " OF " + std::to_string(page_max+1), 3, e_ScreenW - 228, e_ScreenH - 60);
        if (m_layers_page > 0 && UpdateButton(e_ScreenW - 160 + 4, e_ScreenH - 40 + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
            m_layers_page --;
        if (m_layers_page < page_max && UpdateButton(e_ScreenW - 120 + 4, e_ScreenH - 40 + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
            m_layers_page ++;
    }

    // prepare selector
    std::string layer_name;
    std::string* layer_to_set;
    if (m_special_page == SPECIAL_PAGE_OBJ_LAYER && m_special_subpage == 1)
    {
        layer_name = "ATTACHED LAYER:";
        layer_to_set = &EditorCursor.NPC.AttLayer;
    }
    else if (m_special_page == SPECIAL_PAGE_EVENT_LAYERS)
    {
        layer_name = "MOVE LAYER:";
        layer_to_set = &Events[m_current_event].MoveLayer;
    }
    else
    {
        layer_name = "CURRENT LAYER:";
        layer_to_set = &EditorCursor.Layer;
    }
    // extra failsafe
    if (layer_to_set == nullptr)
        return;

    // render description
    if (m_special_page == SPECIAL_PAGE_OBJ_LAYER && m_special_subpage == 1)
    {
        SuperPrint("WHENEVER THE", 3, e_ScreenW - 236, 80);
        SuperPrint("NPC MOVES,", 3, e_ScreenW - 236, 100);
        SuperPrint("THE ATTACHED", 3, e_ScreenW - 236, 120);
        SuperPrint("LAYER MOVES", 3, e_ScreenW - 236, 140);
        SuperPrint("FOLLOWING IT.", 3, e_ScreenW - 236, 160);
    }

    // render current layer
    SuperPrint(layer_name, 3, 10, 40);
    if (layer_to_set->empty())
    {
        if (m_special_subpage == 1 || m_special_page == SPECIAL_PAGE_EVENT_LAYERS)
            SuperPrint("NONE", 3, 10, 56);
        else
            SuperPrint("DEFAULT", 3, 10, 56);
    }
    else
        SuperPrint(*layer_to_set, 3, 10, 56);

    // render layer selector
    for (int i = 0; i < 10; i++)
    {
        int l;
        // separate Default and None for AttLayer
        if ((m_special_page == SPECIAL_PAGE_OBJ_LAYER && m_special_subpage == 1) || m_special_page == SPECIAL_PAGE_EVENT_LAYERS)
            l = m_layers_page*10 + i - 1;
        else
            l = m_layers_page*10 + i;
        if (l > maxLayers)
            continue;
        if (l == -1)
        {
            SuperPrint("NONE", 3, 54, 80 + 40*i + 12);
            if (m_special_page != SPECIAL_PAGE_EVENT_LAYERS && UpdateButton(10 + 4, 80 + 40*i + 4, GFX.ECursor[2], layer_to_set->empty(), 0, 0, 32, 32))
                *layer_to_set = "";
            if (m_special_page == SPECIAL_PAGE_EVENT_LAYERS && UpdateButton(10 + 4, 80 + 40*i + 4, GFX.NPC_modes, layer_to_set->empty(), 0, 32*27, 32, 32))
                *layer_to_set = "";
        }
        // default is a special case on OBJ page 1
        else if (l == 0 && (m_special_page == SPECIAL_PAGE_OBJ_LAYER && m_special_subpage == 0 || m_special_page == SPECIAL_PAGE_LAYERS))
        {
            SuperPrint("DEFAULT", 3, 54, 80 + 40*i + 12);
            if (UpdateButton(10 + 4, 80 + 40*i + 4, GFX.ECursor[2], layer_to_set->empty() || (*layer_to_set) == Layer[l].Name, 0, 0, 32, 32))
                *layer_to_set = "";
        }
        else if (!Layer[l].Name.empty())
        {
            if (Layer[l].Name.length() < 20)
                SuperPrint(Layer[l].Name, 3, 54, 80 + 40*i + 10);
            else
            {
                SuperPrint(Layer[l].Name.substr(0,19), 3, 54, 80 + 40*i + 2);
                SuperPrint(Layer[l].Name.substr(19), 3, 54, 80 + 40*i + 20);
            }
            if (m_special_page != SPECIAL_PAGE_EVENT_LAYERS && UpdateButton(10 + 4, 80 + 40*i + 4, GFX.ECursor[2], (*layer_to_set) == Layer[l].Name, 0, 0, 32, 32))
                *layer_to_set = Layer[l].Name;
            // extra buttons for layers page
            if (m_special_page == SPECIAL_PAGE_LAYERS)
            {
                // rename, hide/show, shift up, shift down, delete
                // hide/show
                if (UpdateButton(440 + 4, 80 + 40*i + 4, GFX.NPC_modes, !Layer[l].Hidden, 0, 32*18, 32, 32))
                {
                    if (Layer[l].Hidden)
                        ShowLayer(Layer[l].Name);
                    else
                        HideLayer(Layer[l].Name);
                }

                if (l <= 2)
                    continue;

                // rename
                if (UpdateButton(400 + 4, 80 + 40*i + 4, GFX.NPC_modes, false, 0, 32*17, 32, 32))
                {
                    std::string new_name = GetTextInput("New layer name", Layer[l].Name);
                    if (!new_name.empty())
                        RenameLayer(Layer[l].Name, new_name);
                }

                // shift up
                if (l > 3 && UpdateButton(480 + 4, 80 + 40*i + 4, GFX.NPC_modes, false, 0, 32*3, 32, 32))
                    std::swap(Layer[l-1], Layer[l]);

                // shift down
                if (l < numLayers - 1 && UpdateButton(520 + 4, 80 + 40*i + 4, GFX.NPC_modes, false, 0, 32*4, 32, 32))
                    std::swap(Layer[l], Layer[l+1]);

                // delete
                if (l < numLayers && UpdateButton(560 + 4, 80 + 40*i + 4, GFX.NPC_modes, false, 0, 32*8, 32, 32))
                {
                    m_special_page = SPECIAL_PAGE_LAYER_DELETION;
                    m_special_subpage = l;
                    return;
                }
            }
            // extra buttons for event layers
            else if (m_special_page == SPECIAL_PAGE_EVENT_LAYERS)
            {
                // nothing, hide, show, toggle
                std::vector<std::string>::iterator hide_it = std::find(Events[m_current_event].HideLayer.begin(), Events[m_current_event].HideLayer.end(), Layer[l].Name);
                std::vector<std::string>::iterator show_it = std::find(Events[m_current_event].ShowLayer.begin(), Events[m_current_event].ShowLayer.end(), Layer[l].Name);
                std::vector<std::string>::iterator togg_it = std::find(Events[m_current_event].ToggleLayer.begin(), Events[m_current_event].ToggleLayer.end(), Layer[l].Name);
                bool cur_hide = hide_it != Events[m_current_event].HideLayer.end();
                bool cur_show = show_it != Events[m_current_event].ShowLayer.end();
                bool cur_togg = togg_it != Events[m_current_event].ToggleLayer.end();
                // no change
                if (UpdateButton(400 + 4, 80 + 40*i + 4, GFX.NPC_modes, !cur_hide && !cur_show && !cur_togg, 0, 32*31, 32, 32))
                {
                    if (cur_hide) Events[m_current_event].HideLayer.erase(hide_it);
                    if (cur_show) Events[m_current_event].ShowLayer.erase(show_it);
                    if (cur_togg) Events[m_current_event].ToggleLayer.erase(togg_it);
                }
                // show layer
                if (UpdateButton(440 + 4, 80 + 40*i + 4, GFX.NPC_modes, cur_show, 0, 32*18, 32, 32))
                {
                    if (cur_hide) Events[m_current_event].HideLayer.erase(hide_it);
                    if (!cur_show) Events[m_current_event].ShowLayer.push_back(Layer[l].Name);
                    if (cur_togg) Events[m_current_event].ToggleLayer.erase(togg_it);
                }
                // hide layer
                if (UpdateButton(480 + 4, 80 + 40*i + 4, GFX.NPC_modes, cur_hide, 0, 32*19, 32, 32))
                {
                    if (!cur_hide) Events[m_current_event].HideLayer.push_back(Layer[l].Name);
                    if (cur_show) Events[m_current_event].ShowLayer.erase(show_it);
                    if (cur_togg) Events[m_current_event].ToggleLayer.erase(togg_it);
                }
                // togg layer
                if (UpdateButton(520 + 4, 80 + 40*i + 4, GFX.NPC_modes, cur_togg, 0, 32*20, 32, 32))
                {
                    if (cur_hide) Events[m_current_event].HideLayer.erase(hide_it);
                    if (cur_show) Events[m_current_event].ShowLayer.erase(show_it);
                    if (!cur_togg) Events[m_current_event].ToggleLayer.push_back(Layer[l].Name);
                }
                if (UpdateButton(10 + 4, 80 + 40*i + 4, GFX.NPC_modes, (*layer_to_set) == Layer[l].Name, 0, 32*27, 32, 32))
                    *layer_to_set = Layer[l].Name;
            }
        }
        // create a new layer!
        else if (m_special_page == SPECIAL_PAGE_LAYERS && l != 0 && !Layer[l - 1].Name.empty())
        {
            SuperPrint("<NEW LAYER>", 3, 54, 80 + 40*i + 10);
            // rename only
            if (UpdateButton(400 + 4, 80 + 40*i + 4, GFX.NPC_modes, false, 0, 32*17, 32, 32))
            {
                std::string new_name = GetTextInput("New layer name", "");
                if (!new_name.empty())
                {
                    Layer[l] = Layer_t();
                    Layer[l].Name = new_name;
                    numLayers ++;
                }
            }
        }
    }
}

void EditorScreen::UpdateBlock(int x, int y, int type)
{
    if ((type < 1) || (type >= maxBlockType))
        return;
    bool sel = EditorCursor.Block.Type == type;
    int draw_width, draw_height;
    if (BlockIsSizable[type])
        draw_width = 32;
    else if (BlockWidth[type] > 0)
        draw_width = BlockWidth[type];
    else
        draw_width = 32;
    if (BlockIsSizable[type])
        draw_height = 32;
    else if (BlockHeight[type] > 0)
        draw_height = BlockHeight[type];
    else
        draw_height = 32;
    if (UpdateButton(x, y, GFXBlock[type], sel, 0, BlockFrame[type] * 32, draw_width, draw_height) && !sel)
        SetEditorBlockType(type);
}

void EditorScreen::UpdateBlockGrid(int x, int y, const int* types, int n_blocks, int n_cols)
{
    for (int i = 0; i < n_blocks; i ++)
    {
        int type = types[i];
        int row = i / n_cols;
        int col = i % n_cols;
        UpdateBlock(x + col * 40 + 4, y + row * 40 + 4, type);
    }
}

void EditorScreen::UpdateBlockScreen()
{
    // Block GUI
    frmMain.renderRect(e_ScreenW - 240, 40, 240, e_ScreenH - 40, 0.7f, 0.7f, 0.9f, 1.0f, true);
    // Page selector
    if (UpdateButton(e_ScreenW - 240 + 4, 40 + 4, GFXBlock[63], m_Block_page == 1, 0, 0, 32, 32))
        m_Block_page = 1;
    if (UpdateButton(e_ScreenW - 240 + 4, 80 + 4, GFXBlock[45], m_Block_page == 2, 0, 0, 32, 32))
        m_Block_page = 2;
    if (UpdateButton(e_ScreenW - 240 + 4, 120 + 4, GFXBlock[10], m_Block_page == 3, 0, 0, 32, 32))
        m_Block_page = 3;
    if (UpdateButton(e_ScreenW - 200 + 4, 120 + 4, GFXBlock[3], m_Block_page == 4, 0, 0, 32, 32))
        m_Block_page = 4;
    if (UpdateButton(e_ScreenW - 160 + 4, 120 + 4, GFXBlock[294], m_Block_page == 5, 0, 0, 32, 32))
        m_Block_page = 5;
    if (UpdateButton(e_ScreenW - 240 + 4, 160 + 4, GFXBlock[81], m_Block_page == 6, 0, 0, 32, 32))
        m_Block_page = 6;
    if (UpdateButton(e_ScreenW - 200 + 4, 160 + 4, GFXBlock[424], m_Block_page == 7, 0, 0, 32, 32))
        m_Block_page = 7;
    if (UpdateButton(e_ScreenW - 160 + 4, 160 + 4, GFXBlock[448], m_Block_page == 8, 0, 0, 32, 32))
        m_Block_page = 8;
    if (UpdateButton(e_ScreenW - 240 + 4, 200 + 4, GFXBlock[557], m_Block_page == 9, 0, 0, 32, 32))
        m_Block_page = 9;

    // Resizing
    if (BlockIsSizable[EditorCursor.Block.Type])
    {
        int H = ((int)EditorCursor.Block.Location.Height)/32;
        int W = ((int)EditorCursor.Block.Location.Width)/32;
        SuperPrint("W " + std::to_string(W), 3, e_ScreenW - 72, 40);
        if (W > 2 && UpdateButton(e_ScreenW - 80 + 4, 60 + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
            EditorCursor.Block.Location.Width = 32 * (W - 1);
        if (UpdateButton(e_ScreenW - 40 + 4, 60 + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
            EditorCursor.Block.Location.Width = 32 * (W + 1);
        SuperPrint("H " + std::to_string(H), 3, e_ScreenW - 72, 100);
        if (H > 2 && UpdateButton(e_ScreenW - 80 + 4, 120 + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
            EditorCursor.Block.Location.Height = 32 * (H - 1);
        if (UpdateButton(e_ScreenW - 40 + 4, 120 + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
            EditorCursor.Block.Location.Height = 32 * (H + 1);
    }

    // Slippy ("SLICK") and Invis
    SuperPrint("SLICK:", 3, e_ScreenW - 150, 214);
    if (UpdateButton(e_ScreenW - 40 + 4, 200 + 4, GFX.NPC_modes, EditorCursor.Block.Slippy, 0, 32*7, 32, 32))
        EditorCursor.Block.Slippy = !EditorCursor.Block.Slippy;
    SuperPrint("INVIS:", 3, e_ScreenW - 150, 254);
    if (UpdateButton(e_ScreenW - 40 + 4, 240 + 4, GFX.NPC_modes, EditorCursor.Block.Invis, 0, 32*7, 32, 32))
        EditorCursor.Block.Invis = !EditorCursor.Block.Invis;

    // Contents
    SuperPrint("CONTENT:", 3, e_ScreenW - 240 + 10, 294);
    StdPicture* image;
    if (EditorCursor.Block.Special > 0 && EditorCursor.Block.Special <= 1000)
    {
        image = &GFXNPC[10];
        SuperPrint("x" + std::to_string(EditorCursor.Block.Special), 3, e_ScreenW-40, 294);
    }
    else if (EditorCursor.Block.Special != 0)
        image = &GFXNPC[EditorCursor.Block.Special - 1000];
    else
        image = &GFX.ECursor[1];
    if (UpdateButton(e_ScreenW - 80 + 4, 280 + 4, *image, EditorCursor.Block.Special != 0, 0, 0, 32, 32))
    {
        m_special_page = SPECIAL_PAGE_BLOCK_CONTENTS;
        FocusNPC();
    }

    // Events
    SuperPrint("EVENTS:", 3, e_ScreenW - 200, 334);
    SuperPrint("H:" + EditorCursor.Block.TriggerHit, 3, e_ScreenW - 240, 360);
    SuperPrint("D:" + EditorCursor.Block.TriggerDeath, 3, e_ScreenW - 240, 380);
    SuperPrint("L:" + EditorCursor.Block.TriggerLast, 3, e_ScreenW - 240, 400);
    if (UpdateButton(e_ScreenW - 80 + 4, 320 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
        m_special_page = SPECIAL_PAGE_OBJ_TRIGGERS;

    // Layers
    SuperPrint("LAYER:", 3, e_ScreenW - 200, 434);
    if (EditorCursor.Block.Layer.empty())
        SuperPrint("DEFAULT", 3, e_ScreenW - 240, 460);
    else
        SuperPrint(EditorCursor.Block.Layer, 3, e_ScreenW - 240, 460);
    if (UpdateButton(e_ScreenW - 80 + 4, 420 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
        m_special_page = SPECIAL_PAGE_OBJ_LAYER;

    // SMB 1: 63
    if (m_Block_page == 1)
    {
        SuperPrint("OVERWORLD", 3, 10, 40);
        static const int p1_overworld[] = {62, 63, 64, 65, 332, 333, 189, 190, 191, 93, 334, 335};
        UpdateBlockGrid(0, 60, p1_overworld, sizeof(p1_overworld)/sizeof(int), 6);

        SuperPrint("UNDERGROUND", 3, 10, 140);
        static const int p1_underground[] = {56, 57, 58, 324, 325, 326, 327, 331, 323, 330, 59, 61, 328, 329};
        UpdateBlockGrid(0, 160, p1_underground, sizeof(p1_underground)/sizeof(int), 7);

        SuperPrint("DUNGEON", 3, 10, 240);
        static const int p1_dungeon[] = {396, 387, 402, 385, 403, 371, 401, 395, 394, 393, 384, 383, 405, 397, 386, 0, 390, 399, 398, 400};
        UpdateBlockGrid(0, 260, p1_dungeon, sizeof(p1_dungeon)/sizeof(int), 7);

        SuperPrint("PIPES", 3, 210, 380);
        static const int p1_pipes[] = {194, 196, 376, 377, 195, 197, 378};
        UpdateBlockGrid(200, 400, p1_pipes, sizeof(p1_pipes)/sizeof(int), 4);

        SuperPrint("CLOUDS", 3, 10, 380);
        static const int p1_clouds[] = {372, 373, 374, 375, 379, 380, 381, 382};
        UpdateBlockGrid(0, 400, p1_clouds, sizeof(p1_clouds)/sizeof(int), 4);

        SuperPrint("SPECIAL", 3, 274, 40);
        static const int p1_special[] = {192, 60, 188, 193};
        UpdateBlockGrid(320, 60, p1_special, sizeof(p1_special)/sizeof(int), 2);

        SuperPrint("MISC", 3, 320, 140);
        static const int p1_misc[] = {66, 67, 68, 69, 369, 576, 388, 577};
        UpdateBlockGrid(320, 160, p1_misc, sizeof(p1_misc)/sizeof(int), 2);

        SuperPrint("SIZED", 3, 300, 320);
        static const int p1_sizable[] = {287};
        UpdateBlockGrid(320, 340, p1_sizable, sizeof(p1_sizable)/sizeof(int), 1);
    }
    // SMB 2: 45
    if (m_Block_page == 2)
    {
        SuperPrint("OVERWORLD", 3, 10, 40);
        static const int p2_overworld[] = {39, 40, 41, 340, 341, 342, 343, 42, 43, 44, 198, 200, 199, 336, 337, 338, 339, 45, 46};
        UpdateBlockGrid(0, 60, p2_overworld, sizeof(p2_overworld)/sizeof(int), 10);

        SuperPrint("UNDERGROUND", 3, 10, 140);
        static const int p2_underground[] = {207, 201, 208, 211, 212, 203, 205, 204, 216, 206, 214, 213, 215};
        UpdateBlockGrid(0, 160, p2_underground, sizeof(p2_underground)/sizeof(int), 5);

        SuperPrint("CASTLE", 3, 10, 280);
        static const int p2_dungeon[] = {491, 498, 490, 495, 496, 497, 500, 502, 493, 494, 492, 499, 501, 503, 504, 506, 507, 508, 510, 511};
        UpdateBlockGrid(0, 300, p2_dungeon, sizeof(p2_dungeon)/sizeof(int), 7);

        SuperPrint("SPECIAL", 3, 180, 420);
        static const int p2_special[] = {370, 293};
        UpdateBlockGrid(200, 440, p2_special, sizeof(p2_special)/sizeof(int), 2);

        SuperPrint("MISC", 3, 320, 140);
        static const int p2_misc[] = {573, 131, 574, 505, 209, 297, 298, 202, 509, 210, 391, 392, 389};
        UpdateBlockGrid(320, 160, p2_misc, sizeof(p2_misc)/sizeof(int), 2);

        SuperPrint("SIZED", 3, 10, 420);
        static const int p2_sizable[] = {288, 243, 242, 38};
        UpdateBlockGrid(0, 440, p2_sizable, sizeof(p2_sizable)/sizeof(int), 4);
    }
    // SMB 3 - Page 1: 10
    if (m_Block_page == 3)
    {
        SuperPrint("GRASS", 3, 10, 40);
        static const int p3_overworld[] = {9, 10, 11, 305, 306, 308, 307, 18, 19, 20, 311, 312, 314, 313, 279, 278, 277, 12, 13, 14};
        UpdateBlockGrid(0, 60, p3_overworld, sizeof(p3_overworld)/sizeof(int), 7);

        SuperPrint("CAVE", 3, 10, 180);
        static const int p3_cave[] = {344, 345, 346, 358, 357, 360, 359, 347, 348, 349, 362, 361, 364, 363, 350, 351, 352, 353, 354, 355, 356};
        UpdateBlockGrid(0, 200, p3_cave, sizeof(p3_cave)/sizeof(int), 7);

        SuperPrint("DUNGEON", 3, 10, 320);
        static const int p3_dungeon[] = {630, 91, 136, 126, 29};
        UpdateBlockGrid(0, 340, p3_dungeon, sizeof(p3_dungeon)/sizeof(int), 5);

        SuperPrint("SPECIAL", 3, 280, 300);
        static const int p3_special[] = {2, 4, 5, 55, 159, 186, 224, 225, 226, 620, 621};
        UpdateBlockGrid(280, 320, p3_special, sizeof(p3_special)/sizeof(int), 3);

        SuperPrint("HURTS", 3, 310, 40);
        static const int p3_hurts[] = {406, 267, 268, 269, 30, 110, 109};
        UpdateBlockGrid(320, 60, p3_hurts, sizeof(p3_hurts)/sizeof(int), 2);

        SuperPrint("SIZED", 3, 10, 380);
        static const int p3_sizable[] = {579, 575, 568, 130, 108, 260, 240, 241, 26, 25, 27, 28};
        UpdateBlockGrid(0, 400, p3_sizable, sizeof(p3_sizable)/sizeof(int), 6);
    }
    // SMB 3 - Page 2: 3
    if (m_Block_page == 4)
    {
        SuperPrint("WOOD AND SNOW", 3, 10, 40);
        static const int p4_wood[] = {7, 3, 6, 600, 604, 605, 601, 270, 272, 271, 15, 16, 17, 602, 606, 607, 603, 128, 127, 129, 274, 276, 275, 1, 92, 633, 634, 595, 596, 597};
        UpdateBlockGrid(0, 60, p4_wood, sizeof(p4_wood)/sizeof(int), 10);

        SuperPrint("DESERT", 3, 10, 180);
        static const int p4_desert[] = {94, 95, 96, 162, 163, 164, 97, 98, 99, 165, 166, 167, 100, 101, 102, 286, 285, 284, 635, 636, 638, 637};
        UpdateBlockGrid(0, 200, p4_desert, sizeof(p4_desert)/sizeof(int), 6);

        SuperPrint("HOUSE", 3, 10, 360);
        static const int p4_house[] = {217, 219, 220, 218, 221, 222};
        UpdateBlockGrid(0, 380, p4_house, sizeof(p4_house)/sizeof(int), 3);

        SuperPrint("BONUS", 3, 290, 180);
        static const int p4_bonus[] = {49, 52, 610, 50, 54, 47, 608, 48, 609, 611, 612, 0, 51, 53};
        UpdateBlockGrid(280, 200, p4_bonus, sizeof(p4_bonus)/sizeof(int), 3);
    }
    // SMB 3 - Page 3: 294
    if (m_Block_page == 5)
    {
        SuperPrint("PIPES", 3, 10, 40);
        static const int p5_pipes[] = {21, 34, 36, 103, 31, 32, 33, 294, 295, 296, 22, 35, 37, 104, 23, 24, 569, 570, 182, 571};
        UpdateBlockGrid(0, 60, p5_pipes, sizeof(p5_pipes)/sizeof(int), 10);

        SuperPrint("WATER", 3, 10, 140);
        static const int p5_water[] = {586, 587, 588, 580, 581, 582, 589, 590, 591, 583, 584, 585, 592, 593, 594, 578, 599, 598};
        UpdateBlockGrid(0, 160, p5_water, sizeof(p5_water)/sizeof(int), 6);

        SuperPrint("MISC", 3, 10, 280);
        static const int p5_misc[] = {132, 572, 457, 458, 168, 8, 290, 289, 280, 107, 106, 105, 184, 223, 187, 160, 112, 111};
        UpdateBlockGrid(0, 300, p5_misc, sizeof(p5_misc)/sizeof(int), 6);

        SuperPrint("AIRSHIP", 3, 270, 140);
        static const int p5_airship[] = {70, 71, 72, 73, 74, 75, 76, 77, 78, 185, 613, 614};
        UpdateBlockGrid(280, 160, p5_airship, sizeof(p5_airship)/sizeof(int), 3);
    }
    // SMW - Page 1: 81
    if (m_Block_page == 6)
    {
        SuperPrint("GRASS", 3, 10, 40);
        static const int p6_grass[] = {80, 81, 82, 299, 616, 302, 301, 617, 300, 83, 87, 84, 86, 618, 304, 303, 619, 85, 265, 264, 266, 309, 263, 273, 310, 488, 489};
        UpdateBlockGrid(0, 60, p6_grass, sizeof(p6_grass)/sizeof(int), 9);

        SuperPrint("CAVE", 3, 10, 180);
        static const int p6_cave[] = {246, 250, 247, 316, 365, 321, 319, 366, 315,
            252, 251, 253, 258, 255, 322, 320, 256, 257,
            248, 254, 249, 317, 368, 367, 318, 262};
        UpdateBlockGrid(0, 200, p6_cave, sizeof(p6_cave)/sizeof(int), 9);

        SuperPrint("SIZED", 3, 10, 320);
        static const int p6_sizable[] = {79, 161, 244, 245, 259, 261, 437, 438, 439, 440, 441, 442, 443, 444, 445};
        UpdateBlockGrid(0, 340, p6_sizable, sizeof(p6_sizable)/sizeof(int), 5);

        SuperPrint("SPECIAL", 3, 250, 320);
        static const int p6_special[] = {88, 89, 90, 169, 281, 282, 283, 615};
        UpdateBlockGrid(240, 340, p6_special, sizeof(p6_special)/sizeof(int), 4);
    }
    // SMW - Page 2: 424
    if (m_Block_page == 7)
    {
        SuperPrint("CASTLE", 3, 10, 40);
        static const int p7_castle[] = {435, 414, 434, 425, 424, 426, 416, 415, 418, 417, 412, 413, 411, 423, 422, 421, 453, 450, 452, 454, 433, 410, 432, 427, 419, 436, 456, 451, 449, 455};
        UpdateBlockGrid(0, 60, p7_castle, sizeof(p7_castle)/sizeof(int), 10);

        SuperPrint("LAVA AND HURTS", 3, 10, 180);
        static const int p7_hurts[] = {408, 466, 459, 460, 469, 470, 480, 472, 474, 482, 409, 463, 467, 461, 468, 471, 481, 473, 475, 483, 407, 465, 462, 464, 404, -1, 487, 477, 478, 484, 429, 428, 430, 431, 420, -1, 486, 476, 479, 485};
        UpdateBlockGrid(0, 200, p7_hurts, sizeof(p7_hurts)/sizeof(int), 10);

        SuperPrint("GHOST", 3, 10, 360);
        static const int p7_ghost[] = {121, 122, 123, 124, 125};
        UpdateBlockGrid(0, 380, p7_ghost, sizeof(p7_ghost)/sizeof(int), 3);

        SuperPrint("SWITCH", 3, 170, 360);
        static const int p7_switch[] = {170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181};
        UpdateBlockGrid(160, 380, p7_switch, sizeof(p7_switch)/sizeof(int), 6);
    }
    // SMW - Page 3: 447
    if (m_Block_page == 8)
    {
        SuperPrint("PIPES", 3, 10, 40);
        static const int p8_pipes[] = {139, 140, 141, 138, 137, 113, 144, 145, 146, 143, 142, 114, 148, 154, 149, 155, 150, 156, 147, 153, 151, 157, 152, 158};
        UpdateBlockGrid(0, 60, p8_pipes, sizeof(p8_pipes)/sizeof(int), 6);

        SuperPrint("BONUS", 3, 10, 220);
        static const int p8_bonus[] = {227, 228, 229, 238, 239,230, 231, 232, 237, 236, 233, 234, 235};
        UpdateBlockGrid(0, 240, p8_bonus, sizeof(p8_bonus)/sizeof(int), 5);

        SuperPrint("MISC", 3, 270, 40);
        static const int p8_misc[] = {133, 134, 135, 115, 446, 447, 448};
        UpdateBlockGrid(280, 60, p8_misc, sizeof(p8_misc)/sizeof(int), 3);

        SuperPrint("WOODS", 3, 10, 360);
        static const int p8_woods[] = {116, 119, 117, 120, 0, 118};
        UpdateBlockGrid(0, 380, p8_woods, sizeof(p8_woods)/sizeof(int), 3);
    }
    // Misc: 557
    if (m_Block_page == 9)
    {
        SuperPrint("METROID (A)", 3, 10, 40);
        static const int p9_metroid_a[] = {515, 513, 533, 517, 514, 518, 512, 516, 528, 519, 520, 521, 522, 523, 532, 524, 525, 526, 530, 527, 529, 531, 534, 535, 536, 537, 540, 540, 539, 538, 542, 543, 544};
        UpdateBlockGrid(0, 60, p9_metroid_a, sizeof(p9_metroid_a)/sizeof(int), 7);

        SuperPrint("METROID (B)", 3, 10, 260);
        static const int p9_metroid_b[] = {564, 557, 550, 556, 545, 546, 555, 558, 553, 554, 547, 548, 552, 566, 565, 567, 549, 562, 559, 551, 560, 563, 561};
        UpdateBlockGrid(0, 280, p9_metroid_b, sizeof(p9_metroid_b)/sizeof(int), 6);

        SuperPrint("PLAYER", 3, 284, 40);
        static const int p9_player[] = {622, 626, 623, 627, 624, 628, 625, 629, 631, 632};
        UpdateBlockGrid(320, 60, p9_player, sizeof(p9_player)/sizeof(int), 2);

        SuperPrint("ZELDA 2", 3, 270, 260);
        static const int p9_zelda[] = {291, 292};
        UpdateBlockGrid(320, 280, p9_zelda, sizeof(p9_zelda)/sizeof(int), 2);
    }
}

void EditorScreen::UpdateBGO(int x, int y, int type)
{
    if ((type < 1) || (type >= maxBackgroundType))
        return;
    bool sel = EditorCursor.Background.Type == type;
    if (UpdateButton(x, y, GFXBackgroundBMP[type], sel, 0, BackgroundFrame[type] * BackgroundHeight[type], GFXBackgroundWidth[type], BackgroundHeight[type]) && !sel)
    {
        printf("%d\n", type);
        EditorCursor.Background.Type = type;
    }
}

void EditorScreen::UpdateBGOGrid(int x, int y, const int* types, int n_bgos, int n_cols)
{
    for (int i = 0; i < n_bgos; i ++)
    {
        int type = types[i];
        int row = i / n_cols;
        int col = i % n_cols;
        UpdateBGO(x + col * 40 + 4, y + row * 40 + 4, type);
    }
}

void EditorScreen::UpdateBGOScreen()
{
    // Block GUI
    frmMain.renderRect(e_ScreenW - 240, 40, 240, e_ScreenH - 40, 0.7f, 0.7f, 0.9f, 1.0f, true);
    // Page selector
    if (UpdateButton(e_ScreenW - 240 + 4, 40 + 4, GFXBlock[63], m_BGO_page == 1, 0, 0, 32, 32))
        m_BGO_page = 1;
    if (UpdateButton(e_ScreenW - 240 + 4, 80 + 4, GFXBlock[45], m_BGO_page == 2, 0, 0, 32, 32))
        m_BGO_page = 2;
    if (UpdateButton(e_ScreenW - 240 + 4, 120 + 4, GFXBlock[10], m_BGO_page == 3, 0, 0, 32, 32))
        m_BGO_page = 3;
    if (UpdateButton(e_ScreenW - 240 + 4, 160 + 4, GFXBlock[81], m_BGO_page == 4, 0, 0, 32, 32))
        m_BGO_page = 4;
    if (UpdateButton(e_ScreenW - 240 + 4, 200 + 4, GFXBlock[557], m_BGO_page == 5, 0, 0, 32, 32))
        m_BGO_page = 5;

    // Layers
    SuperPrint("LAYER:", 3, e_ScreenW - 200, 414);
    if (EditorCursor.Background.Layer.empty())
        SuperPrint("DEFAULT", 3, e_ScreenW - 240, 440);
    else
        SuperPrint(EditorCursor.Background.Layer, 3, e_ScreenW - 240, 440);
    if (UpdateButton(e_ScreenW - 80 + 4, 400 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
        m_special_page = SPECIAL_PAGE_OBJ_LAYER;

    // SMB 1: 63
    if (m_BGO_page == 1)
    {
        SuperPrint("FENCES", 3, 10, 40);
        static const int p1_fences[] = {23, 24, 25, 86, 129, 130, 131, 147};
        UpdateBGOGrid(0, 60, p1_fences, sizeof(p1_fences)/sizeof(int), 4);

        SuperPrint("BUILDINGS", 3, 10, 360);
        static const int p1_buildings[] = {16, 17};
        UpdateBGOGrid(0, 380, p1_buildings, sizeof(p1_buildings)/sizeof(int), 2);

        SuperPrint("PLANTS", 3, 10, 220);
        static const int p1_plants[] = {21, 127, 128, 20, 18, 22, 84, 85, 19,};
        UpdateBGOGrid(0, 240, p1_plants, sizeof(p1_plants)/sizeof(int), 5);

        SuperPrint("WATER", 3, 210, 380);
        static const int p1_water[] = {168, 26, 169, 164};
        UpdateBGOGrid(200, 400, p1_water, sizeof(p1_water)/sizeof(int), 2);

        SuperPrint("CLOUDS", 3, 210, 280);
        static const int p1_clouds[] = {161};
        UpdateBGOGrid(200, 300, p1_clouds, sizeof(p1_clouds)/sizeof(int), 2);
    }
    // SMB 2: 45
    if (m_BGO_page == 2)
    {
        SuperPrint("STRUCTURES", 3, 10, 40);
        static const int p2_structures[] = {148, 149, 150};
        UpdateBGOGrid(0, 60, p2_structures, sizeof(p2_structures)/sizeof(int), 10);

        SuperPrint("DOORS", 3, 10, 140);
        static const int p2_doors[] = {87, 88};
        UpdateBGOGrid(0, 160, p2_doors, sizeof(p2_doors)/sizeof(int), 7);

        SuperPrint("PLANTS", 3, 10, 220);
        static const int p2_plants[] = {111, 112, 113, 110, 62, 108, 109, 63};
        UpdateBGOGrid(0, 240, p2_plants, sizeof(p2_plants)/sizeof(int), 4);

        SuperPrint("WATER", 3, 210, 380);
        static const int p2_water[] = {159, 158};
        UpdateBGOGrid(200, 400, p2_water, sizeof(p2_water)/sizeof(int), 1);
    }
    // SMB 3 - Page 1: 10
    if (m_BGO_page == 3)
    {
        SuperPrint("STRUCTURES", 3, 10, 40);
        static const int p3_fences[] = {89, 94, 80, 67, 81, 46, 91, 95, 106, 101, 102, 45, 90, 96, 36, 68, 69, 93, 97, 99, 162, 163, 103, 107};
        UpdateBGOGrid(0, 60, p3_fences, sizeof(p3_fences)/sizeof(int), 6);

        SuperPrint("PLATFORM", 3, 250, 180);
        static const int p3_platform[] = {100};
        UpdateBGOGrid(360, 200, p3_platform, sizeof(p3_platform)/sizeof(int), 3);

        SuperPrint("EXIT", 3, 290, 40);
        static const int p3_exit[] = {33, 34, 13, 61, 60, 12, 11};
        UpdateBGOGrid(280, 60, p3_exit, sizeof(p3_exit)/sizeof(int), 3);

        SuperPrint("TILES", 3, 10, 380);
        static const int p3_tiles[] = {75, 76, 77, 78, 79, 40, 39, 64, 15, 14};
        UpdateBGOGrid(0, 400, p3_tiles, sizeof(p3_tiles)/sizeof(int), 5);

        SuperPrint("PLANTS", 3, 10, 240);
        static const int p3_plants[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 114, 37, 38};
        UpdateBGOGrid(0, 260, p3_plants, sizeof(p3_plants)/sizeof(int), 7);

        SuperPrint("WATER", 3, 250, 380);
        static const int p3_water[] = {172, 170, 65, 82, 66, 171, 165, 83};
        UpdateBGOGrid(240, 400, p3_water, sizeof(p3_water)/sizeof(int), 4);

        SuperPrint("SAND", 3, 320, 260);
        SuperPrint("BG/FG", 3, 310, 280);
        static const int p3_sand[] = {189, 188, 190, 187};
        UpdateBGOGrid(320, 300, p3_sand, sizeof(p3_sand)/sizeof(int), 2);
    }
    // SMB 3 - Page 2: 3
    if (m_BGO_page == 4)
    {
        SuperPrint("FENCES", 3, 10, 40);
        static const int p4_fences[] = {178, 179, 180, 174, 175, 181, 182, 183, 176, 177, 184, 185, 186};
        UpdateBGOGrid(0, 60, p4_fences, sizeof(p4_fences)/sizeof(int), 5);

        SuperPrint("BARS", 3, 250, 40);
        static const int p4_bars[] = {136, 138, 142, 143, 135, 137, 144, 145, 134};
        UpdateBGOGrid(240, 60, p4_bars, sizeof(p4_bars)/sizeof(int), 4);

        SuperPrint("PLANTS", 3, 10, 180);
        static const int p4_plants[] = {59, 52, 53, 54, 55, 56, 57, 58, 32, 31, 28, 27};
        UpdateBGOGrid(0, 200, p4_plants, sizeof(p4_plants)/sizeof(int), 6);

        SuperPrint("GHOST", 3, 10, 280);
        static const int p4_ghost[] = {146, 140, 139, 43, 44, 47, 48, 49, 50, 51};
        UpdateBGOGrid(0, 300, p4_ghost, sizeof(p4_ghost)/sizeof(int), 5);

        SuperPrint("WATER", 3, 290, 380);
        static const int p4_water[] = {166, 173, 167};
        UpdateBGOGrid(280, 400, p4_water, sizeof(p4_water)/sizeof(int), 2);

        SuperPrint("MISC", 3, 10, 380);
        static const int p4_misc[] = {133, 132, 29, 42, 41, 30};
        UpdateBGOGrid(0, 400, p4_misc, sizeof(p4_misc)/sizeof(int), 3);

        SuperPrint("PLATFORM", 3, 250, 180);
        static const int p4_platform[] = {71, 72, 70, 73, 74};
        UpdateBGOGrid(280, 200, p4_platform, sizeof(p4_platform)/sizeof(int), 3);

        SuperPrint("DOORS", 3, 170, 380);
        static const int p4_doors[] = {141, 104, 105, 92};
        UpdateBGOGrid(160, 400, p4_doors, sizeof(p4_doors)/sizeof(int), 2);

        SuperPrint("KEY", 3, 290, 280);
        static const int p4_key[] = {35};
        UpdateBGOGrid(280, 300, p4_key, sizeof(p4_key)/sizeof(int), 1);
    }
    // SMB 3 - Page 3: 294
    if (m_BGO_page == 5)
    {
        SuperPrint("METROID", 3, 10, 40);
        static const int p5_metroid[] = {153, 155, 154, 151, 156, 157, 152};
        UpdateBGOGrid(0, 60, p5_metroid, sizeof(p5_metroid)/sizeof(int), 3);

        SuperPrint("ZELDA 2", 3, 10, 240);
        static const int p5_zelda[] = {122, 121, 115, 116, 117, 123, 119, 118, 126, 125, 124, 120};
        UpdateBGOGrid(0, 260, p5_zelda, sizeof(p5_zelda)/sizeof(int), 5);
    }
}

void EditorScreen::UpdateWaterScreen()
{
    SuperPrint("WATER SETTINGS", 3, 200, 50);
    int H = ((int)EditorCursor.Water.Location.Height)/32;
    int W = ((int)EditorCursor.Water.Location.Width)/32;
    if (H == 0)
    {
        H = 1;
        EditorCursor.Water.Location.Height = 32;
    }
    if (W == 0)
    {
        W = 1;
        EditorCursor.Water.Location.Width = 32;
    }
    if (W >= 10)
        SuperPrint("WIDTH " + std::to_string(W), 3, 180, 90);
    else
        SuperPrint("WIDTH  " + std::to_string(W), 3, 180, 90);
    if (W > 1 && UpdateButton(340 + 4, 80 + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
        EditorCursor.Water.Location.Width = 32 * (W - 1);
    if (UpdateButton(380 + 4, 80 + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
        EditorCursor.Water.Location.Width = 32 * (W + 1);
    if (H >= 10)
        SuperPrint("HEIGHT " + std::to_string(H), 3, 162, 130);
    else
        SuperPrint("HEIGHT  " + std::to_string(H), 3, 162, 130);
    if (H > 1 && UpdateButton(340 + 4, 120 + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
        EditorCursor.Water.Location.Height = 32 * (H - 1);
    if (UpdateButton(380 + 4, 120 + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
        EditorCursor.Water.Location.Height = 32 * (H + 1);
    SuperPrint("MODE", 3, 246, 170);
    if (UpdateButton(340 + 4, 160 + 4, GFXBackgroundBMP[26], !EditorCursor.Water.Quicksand, 0, 0, 32, 32))
        EditorCursor.Water.Quicksand = false;
    if (UpdateButton(380 + 4, 160 + 4, GFXBackgroundBMP[188], EditorCursor.Water.Quicksand, 0, 0, 32, 32))
        EditorCursor.Water.Quicksand = true;
    // layers
    SuperPrint("LAYER:", 3, 246, 234);
    if (EditorCursor.Layer.empty())
        SuperPrint("DEFAULT", 3, 206, 260);
    else
        SuperPrint(EditorCursor.Layer, 3, 206, 260);
    if (UpdateButton(380 + 4, 220 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
        m_special_page = SPECIAL_PAGE_OBJ_LAYER;
}

void EditorScreen::UpdateWarpScreen()
{
    SuperPrint("WARP SETTINGS", 3, 200, 50);
    // placement selection
    if (EditorCursor.Warp.level == "" && !EditorCursor.Warp.LevelEnt && !EditorCursor.Warp.MapWarp)
    {
        if (EditorCursor.SubMode != 1 && EditorCursor.SubMode != 2)
            EditorCursor.SubMode = 1;
        SuperPrint("PLACING IN", 3, 118, 90);
        if (UpdateButton(300 + 4, 80 + 4, GFX.ECursor[1], EditorCursor.SubMode == 1, 0, 0, 32, 32))
            EditorCursor.SubMode = 1;
        SuperPrint("OUT", 3, 344, 90);
        if (UpdateButton(400 + 4, 80 + 4, GFX.ECursor[1], EditorCursor.SubMode == 2, 0, 0, 32, 32))
            EditorCursor.SubMode = 2;
    }
    // directions
    if (!EditorCursor.Warp.LevelEnt)
    {
        SuperPrint("IN DIRECTION", 3, 28, 130);
        if (UpdateButton(260 + 4, 120 + 4, GFX.NPC_modes, EditorCursor.Warp.Direction == 1, 0, 32*3, 32, 32))
            EditorCursor.Warp.Direction = 1;
        if (UpdateButton(300 + 4, 120 + 4, GFX.NPC_modes, EditorCursor.Warp.Direction == 3, 0, 32*4, 32, 32))
            EditorCursor.Warp.Direction = 3;
        if (UpdateButton(340 + 4, 120 + 4, GFX.NPC_modes, EditorCursor.Warp.Direction == 2, 0, 32*1, 32, 32))
            EditorCursor.Warp.Direction = 2;
        if (UpdateButton(380 + 4, 120 + 4, GFX.NPC_modes, EditorCursor.Warp.Direction == 4, 0, 32*2, 32, 32))
            EditorCursor.Warp.Direction = 4;
    }
    if (EditorCursor.Warp.level == "" && !EditorCursor.Warp.MapWarp)
    {
        SuperPrint("OUT DIRECTION", 3, 10, 170);
        if (UpdateButton(260 + 4, 160 + 4, GFX.NPC_modes, EditorCursor.Warp.Direction2 == 1, 0, 32*4, 32, 32))
            EditorCursor.Warp.Direction2 = 1;
        if (UpdateButton(300 + 4, 160 + 4, GFX.NPC_modes, EditorCursor.Warp.Direction2 == 3, 0, 32*3, 32, 32))
            EditorCursor.Warp.Direction2 = 3;
        if (UpdateButton(340 + 4, 160 + 4, GFX.NPC_modes, EditorCursor.Warp.Direction2 == 2, 0, 32*2, 32, 32))
            EditorCursor.Warp.Direction2 = 2;
        if (UpdateButton(380 + 4, 160 + 4, GFX.NPC_modes, EditorCursor.Warp.Direction2 == 4, 0, 32*1, 32, 32))
            EditorCursor.Warp.Direction2 = 4;
    }
    // warp effect
    if (EditorCursor.Warp.Effect == 1)
        SuperPrint("EFFECT PIPE", 3, 46, 210);
    else if (EditorCursor.Warp.Effect == 2)
        SuperPrint("EFFECT DOOR", 3, 46, 210);
    else if (EditorCursor.Warp.Effect == 3)
        SuperPrint("EFFECT BLIP", 3, 46, 210);
    else
    {
        EditorCursor.Warp.Effect = 1;
        SuperPrint("EFFECT PIPE", 3, 46, 210);
    }
    if (UpdateButton(260 + 4, 200, GFXBlock[294], EditorCursor.Warp.Effect == 1, 0, 0, 32, 32))
        EditorCursor.Warp.Effect = 1;
    if (UpdateButton(300 + 4, 200, GFXBackgroundBMP[88], EditorCursor.Warp.Effect == 2, 0, 0, 32, 32))
    {
        EditorCursor.Warp.Effect = 2;
        EditorCursor.Warp.Direction = 1;
        EditorCursor.Warp.Direction2 = 1;
    }
    if (UpdateButton(340 + 4, 200, GFXBackgroundBMP[61], EditorCursor.Warp.Effect == 3, 0, 0, 32, 32))
        EditorCursor.Warp.Effect = 3;
    // stars required / locked
    if (EditorCursor.Warp.Stars >= 10)
        SuperPrint("REQ STARS " + std::to_string(EditorCursor.Warp.Stars), 3, 26, 250);
    else
        SuperPrint("REQ STARS  " + std::to_string(EditorCursor.Warp.Stars), 3, 26, 250);
    if (EditorCursor.Warp.Stars > 0 && UpdateButton(260 + 4, 240 + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
        EditorCursor.Warp.Stars --;
    if (UpdateButton(300 + 4, 240 + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
        EditorCursor.Warp.Stars ++;
    SuperPrint("LOCKED", 3, 386, 250);
    if (UpdateButton(500 + 4, 240 + 4, GFX.NPC_modes, EditorCursor.Warp.Locked, 0, 32*7, 32, 32))
        EditorCursor.Warp.Locked = !EditorCursor.Warp.Locked;
    // allow / forbid
    SuperPrint("ALLOW MOUNT", 3, 46, 290);
    if (UpdateButton(260 + 4, 280 + 4, GFX.NPC_modes, !EditorCursor.Warp.NoYoshi, 0, 32*7, 32, 32))
        EditorCursor.Warp.NoYoshi = !EditorCursor.Warp.NoYoshi;
    SuperPrint("ITEM", 3, 324, 290);
    if (UpdateButton(420 + 4, 280 + 4, GFX.NPC_modes, EditorCursor.Warp.WarpNPC, 0, 32*7, 32, 32))
        EditorCursor.Warp.WarpNPC = !EditorCursor.Warp.WarpNPC;
    // map/level warps
    SuperPrint("SPECIAL OPTIONS", 3, 164, 350);
    SuperPrint("TO MAP", 3, 10, 390);
    if (UpdateButton(120 + 4, 380 + 4, GFX.NPC_modes, EditorCursor.Warp.MapWarp, 0, 32*7, 32, 32))
    {
        EditorCursor.Warp.MapWarp = !EditorCursor.Warp.MapWarp;
        if (EditorCursor.Warp.MapWarp)
        {
            EditorCursor.Warp.level = "";
            EditorCursor.Warp.LevelEnt = false;
        }
    }
    SuperPrint("LVL WARP ENTER", 3, 210, 390);
    if (UpdateButton(466 + 4, 380 + 4, GFX.NPC_modes, !EditorCursor.Warp.level.empty(), 0, 32*7, 32, 32))
    {
        if (EditorCursor.Warp.level.empty())
        {
            EditorCursor.Warp.level = "...";
            EditorCursor.Warp.MapWarp = false;
            EditorCursor.Warp.LevelEnt = false;
        }
        else
            EditorCursor.Warp.level = "";
    }
    SuperPrint("EXIT", 3, 516, 390);
    if (UpdateButton(590 + 4, 380 + 4, GFX.NPC_modes, EditorCursor.Warp.LevelEnt, 0, 32*7, 32, 32))
    {
        EditorCursor.Warp.LevelEnt = !EditorCursor.Warp.LevelEnt;
        if (EditorCursor.Warp.LevelEnt)
        {
            EditorCursor.Warp.level = "";
            EditorCursor.Warp.MapWarp = false;
        }
    }
    // special options for lvl warp entrance
    if (!EditorCursor.Warp.level.empty())
    {
        if (EditorCursor.Warp.level.length() < 10)
            SuperPrint("TARGET: " + EditorCursor.Warp.level, 3, 10, 430);
        else
        {
            SuperPrint("TARGET: " + EditorCursor.Warp.level.substr(0,10), 3, 10, 420);
            SuperPrint(EditorCursor.Warp.level.substr(10), 3, 28, 440);
        }
        if (UpdateButton(330 + 4, 420 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
            StartFileBrowser(&EditorCursor.Warp.level, FileNamePath, "", {".lvl"}, BROWSER_MODE_OPEN);
        if (EditorCursor.Warp.LevelWarp == 0)
            SuperPrint("LVL START", 3, 384, 430);
        else
            SuperPrint("SECTION "+std::to_string(EditorCursor.Warp.LevelWarp), 3, 384, 430);
        if (EditorCursor.Warp.LevelWarp > 0 && UpdateButton(560 + 4, 420 + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
            EditorCursor.Warp.LevelWarp --;
        if (UpdateButton(600 + 4, 420 + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
            EditorCursor.Warp.LevelWarp ++;
    }
    // special options for map warp
    if (EditorCursor.Warp.MapWarp)
    {
        SuperPrint("MAP WARP:", 3, 50, 424);
        if ((int)EditorCursor.Warp.MapX != -1 || (int)EditorCursor.Warp.MapY != -1.)
        {
            SuperPrint("X: "+std::to_string((int)EditorCursor.Warp.MapX), 3, 10, 442);
            SuperPrint("Y: "+std::to_string((int)EditorCursor.Warp.MapY), 3, 10, 460);
            if (UpdateButton(160 + 160 + 4, 440 + 4, GFX.NPC_modes, false, 0, 32*8, 32, 32))
            {
                EditorCursor.Warp.MapX = -1.;
                EditorCursor.Warp.MapY = -1.;
            }
        }
        else
        {
            SuperPrint("NONE", 3, 10, 450);
        }
        if (UpdateButton(160 + 4, 440 + 4, GFX.NPC_modes, false, 0, 32*3, 32, 32))
        {
            EditorCursor.Warp.MapY = 32*((int)EditorCursor.Warp.MapY/32 - 1);
            if ((int)EditorCursor.Warp.MapX == -1)
                EditorCursor.Warp.MapX = 0.;
        }
        if (UpdateButton(160 + 40 + 4, 440 + 4, GFX.NPC_modes, false, 0, 32*4, 32, 32))
        {
            EditorCursor.Warp.MapY = 32*((int)EditorCursor.Warp.MapY/32 + 1);
            if ((int)EditorCursor.Warp.MapX == -1)
                EditorCursor.Warp.MapX = 0.;
        }
        if (UpdateButton(160 + 80 + 4, 440 + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
        {
            EditorCursor.Warp.MapX = 32*((int)EditorCursor.Warp.MapX/32 - 1);
            if ((int)EditorCursor.Warp.MapY == -1)
                EditorCursor.Warp.MapY = 0.;
        }
        if (UpdateButton(160 + 120 + 4, 440 + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
        {
            EditorCursor.Warp.MapX = 32*((int)EditorCursor.Warp.MapX/32 + 1);
            if ((int)EditorCursor.Warp.MapY == -1)
                EditorCursor.Warp.MapY = 0.;
        }
    }
}

void EditorScreen::UpdateTile(int x, int y, int type)
{
    if ((type < 1) || (type >= maxTileType))
        return;
    bool sel = EditorCursor.Tile.Type == type;
    if (UpdateButton(x, y, GFXTileBMP[type], sel, 0, TileHeight[type] * TileFrame[type], TileWidth[type], TileHeight[type]) && !sel)
    {
        printf("%d\n", type);
        EditorCursor.Tile.Type = type;
    }
}

void EditorScreen::UpdateTileGrid(int x, int y, const int* types, int n_tiles, int n_cols)
{
    for (int i = 0; i < n_tiles; i ++)
    {
        int type = types[i];
        int row = i / n_cols;
        int col = i % n_cols;
        UpdateTile(x + col * 40 + 4, y + row * 40 + 4, type);
    }
}

void EditorScreen::UpdateTileScreen()
{
    // Block GUI
    frmMain.renderRect(e_ScreenW - 120, 40, 120, e_ScreenH - 40, 0.7f, 0.7f, 0.9f, 1.0f, true);
    // Page selector
    if (UpdateButton(e_ScreenW - 120 + 4, 40 + 4, GFXTileBMP[11], m_tile_page == 1, 0, 0, 32, 32))
        m_tile_page = 1;
    if (UpdateButton(e_ScreenW - 120 + 4, 80 + 4, GFXTileBMP[14], m_tile_page == 2, 0, 0, 32, 32))
        m_tile_page = 2;
    if (UpdateButton(e_ScreenW - 120 + 4, 120 + 4, GFXTileBMP[241], m_tile_page == 3, 0, 0, 32, 32))
        m_tile_page = 3;
    if (UpdateButton(e_ScreenW - 120 + 4, 160 + 4, GFXTileBMP[326], m_tile_page == 4, 0, 0, 32, 32))
        m_tile_page = 4;

    // SMB 3
    if (m_tile_page == 1)
    {
        SuperPrint("GRASS", 3, 10, 40);
        static const int p1_grass[] = {20, 21, 22, 15, 16, 12,
            23, 11, 24, 18, 19, 13,
            25, 26, 17, 40, 41,};
        UpdateTileGrid(0, 60, p1_grass, sizeof(p1_grass)/sizeof(int), 6);

        SuperPrint("DESERT", 3, 10, 180);
        static const int p1_desert[] = {28, 32, 29, 5, 6, 3, 36, 37, 8,
            33,  1, 35, 7, 42, 7, 39, 38, 9,
            30, 34, 31, 2, 6, 4,};
        UpdateTileGrid(0, 200, p1_desert, sizeof(p1_desert)/sizeof(int), 9);

        SuperPrint("SNOW", 3, 10, 320);
        static const int p1_snow[] = {48, 53, 49, 43, 45,
            52, 10, 54, 44, 46,
            50, 55, 51, 47};
        UpdateTileGrid(0, 340, p1_snow, sizeof(p1_snow)/sizeof(int), 5);
    }

    // SMW A
    if (m_tile_page == 2)
    {
        SuperPrint("SMW A", 3, 10, 40);
        static const int p2_smw_a1[] =
            {79, 80, 81, 82, 83, 96, 94, 106, 105, 109, 101, 110,
                72, 57, 89, 102, 61, 107, 108, 104, 103, -1, 97, 98,
                70, 74, 68, 76, 59, -1, -1, 95, -1, -1, 92, 93,
                77, 63, 60, 78, 75, 67, 56, 73, 69, 66, 90, 91,
                84, 85, 86, 87, 88, 65, 71, 62, 64, 58, 99, 100};
        UpdateTileGrid(0, 60, p2_smw_a1, sizeof(p2_smw_a1)/sizeof(int), 12);

        static const int p2_smw_a2[] =
        {137, 144, 126, 125, 132, 139, 141, 27,
            122, 114, 134, 119, 121, 131, 140, 14,
            127, 112, 143, 135, 116, 123, 124, 240,
            145, 138, 118, 133, 115, 136, 142, -1,
            120, 128, 117, 111, 113, 129, 130};
        UpdateTileGrid(0, 260, p2_smw_a2, sizeof(p2_smw_a2)/sizeof(int), 8);
    }

    // SMW B1
    if (m_tile_page == 3)
    {
        SuperPrint("SMW B", 3, 10, 40);
        static const int p3_smw_b[] = {170, 171, 180, 172, 173, 255, 251, 256, 254, 253, 252, 244, 152, 155, 179, 197, 147, 250, 249, 248, 247, 246, 243, 245, 165, 158, 162, 167, 163, 238, 239, 235, 237, 236, 196, 182, 168, 149, 146, 169, 166, 161, 154, 157, 164, 159, 181, 183, 174, 175, 176, 177, 178, 156, 151, 148, 153, 160, 188, 194, 216, 233, 207, 221, 227, 229, 150, 185, 184, 241, 257, 261, 213, 214, 225, 204, 215, 220, 230, 192, 193, 242, 259, 260, 206, 210, 232, 224, 203, 219, 218, 189, 198, 195, 262, 258, 234, 223, 201, 226, 200, 228, 231, 191, 190, 205, 264, -1, 208, 212, 202, 211, 209, 217, 222, 186, 187, 199, 263};
        UpdateTileGrid(0, 60, p3_smw_b, sizeof(p3_smw_b)/sizeof(int), 12);
    }

    // SMW C
    if (m_tile_page == 4)
    {
        SuperPrint("SMW C", 3, 10, 40);
        static const int p3_smw_c[] = {279, 285, 284, 291, 288, 318, 319, 323, 320, 322, 324, 321,
            280, 267, 286, 302, 283, 272, 268, 298, 278, 271, 293, 309,
            289, 287, 273, 275, 265, 270, 281, 290, 269, 266, 292, 294,
            277, 307, 276, 282, 274, 306, 314, 313, 312, 317, 297, 303,
            296, 300, 299, 308, 310, 304, 305, 301, 295, 311, 315, 316,
            326, 325, 327, 328,};
        UpdateTileGrid(0, 60, p3_smw_c, sizeof(p3_smw_c)/sizeof(int), 12);
    }
}

void EditorScreen::UpdateScene(int x, int y, int type)
{
    if ((type < 1) || (type >= maxSceneType))
        return;
    bool sel = EditorCursor.Scene.Type == type;
    if (UpdateButton(x, y, GFXSceneBMP[type], sel, 0, SceneHeight[type] * SceneFrame[type], SceneWidth[type], SceneHeight[type]) && !sel)
    {
        printf("%d\n", type);
        EditorCursor.Scene.Type = type;
    }
}

void EditorScreen::UpdateSceneGrid(int x, int y, const int* types, int n_scenes, int n_cols)
{
    for (int i = 0; i < n_scenes; i ++)
    {
        int type = types[i];
        int row = i / n_cols;
        int col = i % n_cols;
        UpdateScene(x + col * 40 + 4, y + row * 40 + 4, type);
    }
}

void EditorScreen::UpdateSceneScreen()
{
    // Scene GUI (none)

    SuperPrint("SCENERY", 3, 10, 40);
    static const int scenes[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 48, 49, 42, 47, 46, 43, 45, 64, 65, 44, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63};
    UpdateSceneGrid(0, 60, scenes, sizeof(scenes)/sizeof(int), 10);
}

void EditorScreen::UpdateLevel(int x, int y, int type)
{
    if ((type < 1) || (type >= maxLevelType))
        return;
    bool sel = EditorCursor.WorldLevel.Type == type;
    int draw_height;
    if (GFXLevelBig[type])
        draw_height = GFXLevelHeight[type];
    else
        draw_height = 32;
    if (UpdateButton(x, y, GFXLevelBMP[type], sel, 0, 32 * LevelFrame[type], GFXLevelWidth[type], draw_height) && !sel)
    {
        printf("%d\n", type);
        EditorCursor.WorldLevel.Type = type;
    }
}

void EditorScreen::UpdateLevelGrid(int x, int y, const int* types, int n_levels, int n_cols)
{
    for (int i = 0; i < n_levels; i ++)
    {
        int type = types[i];
        int row = i / n_cols;
        int col = i % n_cols;
        UpdateLevel(x + col * 40 + 4, y + row * 40 + 4, type);
    }
}

void EditorScreen::UpdateLevelScreen()
{
    // World Level GUI
    frmMain.renderRect(e_ScreenW - 240, 40, 240, e_ScreenH - 40, 0.7f, 0.7f, 0.9f, 1.0f, true);

    SuperPrint("LEVEL GRAPHIC", 3, 10, 40);
    static const int levels[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32};
    UpdateLevelGrid(0, 60, levels, sizeof(levels)/sizeof(int), 8);

    // path bg - Path
    if (UpdateButton(e_ScreenW - 240 + 4, 80+4, GFX.NPC_modes, EditorCursor.WorldLevel.Path, 0, 32*7, 32, 32))
    {
        EditorCursor.WorldLevel.Path = !EditorCursor.WorldLevel.Path;
        if (EditorCursor.WorldLevel.Path)
            EditorCursor.WorldLevel.Path2 = false;
    }
    SuperPrint("PATH BG", 3, e_ScreenW - 240 + 44, 90);
    // big bg - Path2
    if (UpdateButton(e_ScreenW - 240 + 4, 120+4, GFX.NPC_modes, EditorCursor.WorldLevel.Path2, 0, 32*7, 32, 32))
    {
        EditorCursor.WorldLevel.Path2 = !EditorCursor.WorldLevel.Path2;
        if (EditorCursor.WorldLevel.Path2)
            EditorCursor.WorldLevel.Path = false;
    }
    SuperPrint("BIG BG", 3, e_ScreenW - 240 + 44, 130);
    // game start - Start
    if (UpdateButton(e_ScreenW - 240 + 4, 160 + 4, GFX.NPC_modes, EditorCursor.WorldLevel.Start, 0, 32*7, 32, 32))
        EditorCursor.WorldLevel.Start = !EditorCursor.WorldLevel.Start;
    SuperPrint("GAME START", 3, e_ScreenW - 240 + 44, 170);
    // always visible - Visible
    if (UpdateButton(e_ScreenW - 240 + 4, 200 + 4, GFX.NPC_modes, EditorCursor.WorldLevel.Visible, 0, 32*7, 32, 32))
        EditorCursor.WorldLevel.Visible = !EditorCursor.WorldLevel.Visible;
    SuperPrint("ALWAYS VIS", 3, e_ScreenW - 240 + 44, 210);

    // map warp!
    SuperPrint("MAP WARP:", 3, e_ScreenW - 240 + 44, 302);
    if ((int)EditorCursor.WorldLevel.WarpX != -1 || (int)EditorCursor.WorldLevel.WarpY != -1.)
    {
        SuperPrint("X: "+std::to_string((int)EditorCursor.WorldLevel.WarpX), 3, e_ScreenW - 240 + 4, 320);
        SuperPrint("Y: "+std::to_string((int)EditorCursor.WorldLevel.WarpY), 3, e_ScreenW - 240 + 4, 340);
        if (UpdateButton(e_ScreenW - 240 + 160 + 4, 320 + 4, GFX.NPC_modes, false, 0, 32*8, 32, 32))
        {
            EditorCursor.WorldLevel.WarpX = -1.;
            EditorCursor.WorldLevel.WarpY = -1.;
        }
    }
    else
    {
        SuperPrint("NONE", 3, e_ScreenW - 240 + 4, 330);
    }
    if (UpdateButton(e_ScreenW - 240 + 4, 360 + 4, GFX.NPC_modes, false, 0, 32*3, 32, 32))
    {
        EditorCursor.WorldLevel.WarpY = 32*((int)EditorCursor.WorldLevel.WarpY/32 - 1);
        if ((int)EditorCursor.WorldLevel.WarpX == -1)
            EditorCursor.WorldLevel.WarpX = 0.;
    }
    if (UpdateButton(e_ScreenW - 240 + 40 + 4, 360 + 4, GFX.NPC_modes, false, 0, 32*4, 32, 32))
    {
        EditorCursor.WorldLevel.WarpY = 32*((int)EditorCursor.WorldLevel.WarpY/32 + 1);
        if ((int)EditorCursor.WorldLevel.WarpX == -1)
            EditorCursor.WorldLevel.WarpX = 0.;
    }
    if (UpdateButton(e_ScreenW - 240 + 80 + 4, 360 + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
    {
        EditorCursor.WorldLevel.WarpX = 32*((int)EditorCursor.WorldLevel.WarpX/32 - 1);
        if ((int)EditorCursor.WorldLevel.WarpY == -1)
            EditorCursor.WorldLevel.WarpY = 0.;
    }
    if (UpdateButton(e_ScreenW - 240 + 120 + 4, 360 + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
    {
        EditorCursor.WorldLevel.WarpX = 32*((int)EditorCursor.WorldLevel.WarpX/32 + 1);
        if ((int)EditorCursor.WorldLevel.WarpY == -1)
            EditorCursor.WorldLevel.WarpY = 0.;
    }

    // bottom pane: level filename, entrance and exits
    frmMain.renderRect(0, e_ScreenH - 240, e_ScreenW - 240, 240, 0.6f, 0.6f, 0.8f, 1.0f, true);

    // level name - LevelName
    SuperPrint("LEVEL NAME:", 3, 10 + 44, e_ScreenH - 240 + 2);
    SuperPrint(EditorCursor.WorldLevel.LevelName.substr(0, 19), 3, 10 + 44, e_ScreenH - 240 + 20);
    if (EditorCursor.WorldLevel.LevelName.length() > 19)
        SuperPrint(EditorCursor.WorldLevel.LevelName.substr(19), 3, 10 + 44 + 18, e_ScreenH - 240 + 38);
    if (UpdateButton(10 + 4, e_ScreenH - 240 + 4, GFX.NPC_modes, false, 0, 32*17, 32, 32))
        EditorCursor.WorldLevel.LevelName = GetTextInput("Level name", EditorCursor.WorldLevel.LevelName);

    // level filename - FileName
    SuperPrint("FILENAME:", 3, 10 + 44, e_ScreenH - 180 + 2);
    if (!EditorCursor.WorldLevel.FileName.empty())
        SuperPrint(EditorCursor.WorldLevel.FileName, 3, 10 + 44, e_ScreenH - 180 + 20);
    else
        SuperPrint("<NONE>", 3, 10 + 44, e_ScreenH - 180 + 20);
    if (UpdateButton(10 + 4, e_ScreenH - 180 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
        StartFileBrowser(&EditorCursor.WorldLevel.FileName, FileNamePath, "", {".lvl"}, BROWSER_MODE_OPEN);

    if (!EditorCursor.WorldLevel.FileName.empty())
    {
        // entrance warp - StartWarp
        if (EditorCursor.WorldLevel.StartWarp > 0 && UpdateButton(50 + 4, e_ScreenH - 180 + 40 + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
            EditorCursor.WorldLevel.StartWarp -= 1;
        if (EditorCursor.WorldLevel.StartWarp < maxWarps && UpdateButton(280 + 4, e_ScreenH - 180 + 40 + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
            EditorCursor.WorldLevel.StartWarp += 1;
        SuperPrint("ENTRANCE:", 3, 50 + 44, e_ScreenH - 180 + 40 + 2);
        if (EditorCursor.WorldLevel.StartWarp != 0)
            SuperPrint("WARP " + std::to_string(EditorCursor.WorldLevel.StartWarp), 3, 50 + 44, e_ScreenH - 180 + 40 + 20);
        else
            SuperPrint("NORMAL", 3, 50 + 44, e_ScreenH - 180 + 40 + 20);

        // exits
        SuperPrint("PATH UNLOCKS", 3, 10 + 44, e_ScreenH - 180 + 80 + 2);

        if (UpdateButton(10 + 4, e_ScreenH - 180 + 100 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
        {
            m_special_page = SPECIAL_PAGE_LEVEL_EXIT;
            m_special_subpage = 1;
        }
        SuperPrint("UP", 3, 10 + 44, e_ScreenH - 180 + 102);
        SuperPrint(list_level_exit_names[EditorCursor.WorldLevel.LevelExit[1]+1], 3, 10 + 44, e_ScreenH - 180 + 120);

        if (UpdateButton((e_ScreenW - 240)/2 + 10 + 4, e_ScreenH - 180 + 100 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
        {
            m_special_page = SPECIAL_PAGE_LEVEL_EXIT;
            m_special_subpage = 3;
        }
        SuperPrint("DOWN", 3, (e_ScreenW - 240)/2 + 10 + 44, e_ScreenH - 180 + 102);
        SuperPrint(list_level_exit_names[EditorCursor.WorldLevel.LevelExit[3]+1], 3, (e_ScreenW - 240)/2 + 10 + 44, e_ScreenH - 180 + 120);

        if (UpdateButton(10 + 4, e_ScreenH - 180 + 140 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
        {
            m_special_page = SPECIAL_PAGE_LEVEL_EXIT;
            m_special_subpage = 2;
        }
        SuperPrint("LEFT", 3, 10 + 44, e_ScreenH - 180 + 142);
        SuperPrint(list_level_exit_names[EditorCursor.WorldLevel.LevelExit[2]+1], 3, 10 + 44, e_ScreenH - 180 + 160);

        if (UpdateButton((e_ScreenW - 240)/2 + 10 + 4, e_ScreenH - 180 + 140 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
        {
            m_special_page = SPECIAL_PAGE_LEVEL_EXIT;
            m_special_subpage = 4;
        }
        SuperPrint("RIGHT", 3, (e_ScreenW - 240)/2 + 10 + 44, e_ScreenH - 180 + 142);
        SuperPrint(list_level_exit_names[EditorCursor.WorldLevel.LevelExit[4]+1], 3, (e_ScreenW - 240)/2 + 10 + 44, e_ScreenH - 180 + 160);
    }

}

void EditorScreen::UpdatePath(int x, int y, int type)
{
    if ((type < 1) || (type >= maxPathType))
        return;
    bool sel = EditorCursor.WorldPath.Type == type;
    if (UpdateButton(x, y, GFXPathBMP[type], sel, 0, 0, 32, 32) && !sel)
    {
        printf("%d\n", type);
        EditorCursor.WorldPath.Type = type;
    }
}

void EditorScreen::UpdatePathGrid(int x, int y, const int* types, int n_paths, int n_cols)
{
    for (int i = 0; i < n_paths; i ++)
    {
        int type = types[i];
        int row = i / n_cols;
        int col = i % n_cols;
        UpdatePath(x + col * 40 + 4, y + row * 40 + 4, type);
    }
}

void EditorScreen::UpdatePathScreen()
{
    // Path GUI (none)

    SuperPrint("PATH GRAPHIC", 3, 10, 40);
    static const int paths[] = { 2, 1, 18, 19, 31, 22, 32,
        11, 6, 10, 30, 25, 27, 23,
         9, 5,  7, 21,  3, 20, 26,
        13, 8, 12, 29, 24, 28,  4};
    UpdatePathGrid(0, 60, paths, sizeof(paths)/sizeof(int), 7);
}

void EditorScreen::UpdateFileScreen()
{
    if (m_special_page == SPECIAL_PAGE_FILE_CONFIRM)
    {
        if (m_special_subpage == 0)
        {
            m_special_page = SPECIAL_PAGE_FILE;
            m_special_subpage = 0;
            return;
        }

        bool can_save;
        std::string action;
        if (m_special_subpage == 1)
        {
            can_save = true;
            action = "CLEAR LEVEL";
        }
        else if (m_special_subpage == 11)
        {
            can_save = true;
            action = "CLEAR WORLD";
        }
        else if (m_special_subpage == 2 || m_special_subpage == 12)
        {
            can_save = true;
            action = "OPEN";
        }
        else if (m_special_subpage == 3 || m_special_subpage == 13)
        {
            can_save = false;
            action = "REVERT";
        }
        else if (m_special_subpage == 4)
        {
            can_save = true;
            action = "EXIT";
        }
        else // should never happen
        {
            return;
        }

        if (can_save)
            SuperPrint("SAVE BEFORE YOU " + action + "?", 3, 20, 40);
        else
            SuperPrint("ARE YOU SURE YOU WANT TO " + action + "?", 3, 20, 40);

        bool confirmed = false;
        if (can_save)
        {
            SuperPrint("YES: SAVE THEN " + action, 3, 60, 110);
            if (UpdateButton(20 + 4, 100 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
            {
                if (!WorldEditor)
                    SaveLevel(FullFileName);
                else
                    SaveWorld(FullFileName);
                confirmed = true;
            }
        }

        if (can_save)
            SuperPrint("NO: " + action + " WITHOUT SAVING", 3, 60, 150);
        else
            SuperPrint(action + " WITHOUT SAVING", 3, 60, 150);
        if (UpdateButton(20 + 4, 140 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
        {
            confirmed = true;
        }

        SuperPrint("CANCEL: DO NOT " + action, 3, 60, 190);
        if (UpdateButton(20 + 4, 180 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
        {
            m_special_page = SPECIAL_PAGE_FILE;
            m_special_subpage = 0;
            return;
        }
        if (confirmed)
        {
            if (m_special_subpage == 1)
            {
                StartFileBrowser(&FullFileName, "", FileNamePath, {".lvl"}, BROWSER_MODE_SAVE_NEW, BROWSER_CALLBACK_NEW_LEVEL);
            }
            else if (m_special_subpage == 2)
            {
                StartFileBrowser(&FullFileName, "", FileNamePath, {".lvl"}, BROWSER_MODE_OPEN, BROWSER_CALLBACK_OPEN_LEVEL);
            }
            else if (m_special_subpage == 3)
            {
                OpenLevel(FullFileName);
                m_special_page = SPECIAL_PAGE_FILE;
                m_special_subpage = 0;
            }
            else if (m_special_subpage == 11)
            {
                StartFileBrowser(&FullFileName, "", FileNamePath, {".wld"}, BROWSER_MODE_SAVE_NEW, BROWSER_CALLBACK_NEW_WORLD);
            }
            else if (m_special_subpage == 12)
            {
                StartFileBrowser(&FullFileName, "", FileNamePath, {".wld"}, BROWSER_MODE_OPEN, BROWSER_CALLBACK_OPEN_WORLD);
            }
            else if (m_special_subpage == 13)
            {
                OpenWorld(FullFileName);
                m_special_page = SPECIAL_PAGE_FILE;
                m_special_subpage = 0;
            }
            else if (m_special_subpage == 4)
            {
                ClearLevel();
                ClearWorld();
                GameMenu = true;
                MenuMode = 0;
                LevelEditor = false;
                WorldEditor = false;
                TestLevel = false;
                m_special_page = SPECIAL_PAGE_NONE;
                m_special_subpage = 0;
            }
        }
        return;
    }

    SuperPrint("CURRENT FILE: " + FileName, 3, 10, 40);

    SuperPrint("LEVEL", 3, 110, 80);

    SuperPrint("NEW", 3, 54, 110);
    if (UpdateButton(10 + 4, 100 + 4, GFX.NPC_modes, false, 0, 32*24, 32, 32))
    {
        m_special_page = SPECIAL_PAGE_FILE_CONFIRM;
        m_special_subpage = 1;
    }
    SuperPrint("OPEN...", 3, 54, 150);
    if (UpdateButton(10 + 4, 140 + 4, GFX.NPC_modes, false, 0, 32*23, 32, 32))
    {
        m_special_page = SPECIAL_PAGE_FILE_CONFIRM;
        m_special_subpage = 2;
    }
    if (!WorldEditor)
    {
        SuperPrint("SAVE", 3, 54, 190);
        if (UpdateButton(10 + 4, 180 + 4, GFX.NPC_modes, false, 0, 32*22, 32, 32))
        {
            SaveLevel(FullFileName);
        }
        SuperPrint("SAVE AS...", 3, 54, 230);
        if (UpdateButton(10 + 4, 220 + 4, GFX.NPC_modes, false, 0, 32*22, 32, 32))
        {
            StartFileBrowser(&FullFileName, "", FileNamePath, {".lvl"}, BROWSER_MODE_SAVE, BROWSER_CALLBACK_SAVE_LEVEL);
        }
        SuperPrint("REVERT", 3, 54, 270);
        if (UpdateButton(10 + 4, 260 + 4, GFX.NPC_modes, false, 0, 32*11, 32, 32))
        {
            m_special_page = SPECIAL_PAGE_FILE_CONFIRM;
            m_special_subpage = 3;
        }
    }

    SuperPrint("WORLD", 3, e_ScreenW/2 + 110, 80);

    SuperPrint("NEW", 3, e_ScreenW/2 + 54, 110);
    if (UpdateButton(e_ScreenW/2 + 10 + 4, 100 + 4, GFX.NPC_modes, false, 0, 32*24, 32, 32))
    {
        m_special_page = SPECIAL_PAGE_FILE_CONFIRM;
        m_special_subpage = 11;
    }
    SuperPrint("OPEN...", 3, e_ScreenW/2 + 54, 150);
    if (UpdateButton(e_ScreenW/2 + 10 + 4, 140 + 4, GFX.NPC_modes, false, 0, 32*23, 32, 32))
    {
        m_special_page = SPECIAL_PAGE_FILE_CONFIRM;
        m_special_subpage = 12;
    }
    if (WorldEditor)
    {
        SuperPrint("SAVE", 3, e_ScreenW/2 + 54, 190);
        if (UpdateButton(e_ScreenW/2 + 10 + 4, 180 + 4, GFX.NPC_modes, false, 0, 32*22, 32, 32))
        {
            SaveWorld(FullFileName);
        }
        SuperPrint("SAVE AS...", 3, e_ScreenW/2 + 54, 230);
        if (UpdateButton(e_ScreenW/2 + 10 + 4, 220 + 4, GFX.NPC_modes, false, 0, 32*22, 32, 32))
        {
            StartFileBrowser(&FullFileName, "", FileNamePath, {".wld"}, BROWSER_MODE_SAVE, BROWSER_CALLBACK_SAVE_WORLD);
        }
        SuperPrint("REVERT", 3, e_ScreenW/2 + 54, 270);
        if (UpdateButton(e_ScreenW/2 + 10 + 4, 260 + 4, GFX.NPC_modes, false, 0, 32*11, 32, 32))
        {
            m_special_page = SPECIAL_PAGE_FILE_CONFIRM;
            m_special_subpage = 13;
        }
    }

    SuperPrint("EXIT", 3, 54, 350);
    if (UpdateButton(10 + 4, 340 + 4, GFX.NPC_modes, false, 0, 32*8, 32, 32))
    {
        m_special_page = SPECIAL_PAGE_FILE_CONFIRM;
        m_special_subpage = 4;
    }
}

void EditorScreen::StartFileBrowser(std::string* file_target, const std::string root_path, const std::string current_path, const std::vector<std::string> target_exts, BrowserMode_t browser_mode, BrowserCallback_t browser_callback)
{
    m_file_target = file_target;
    m_root_path = root_path;
    m_target_exts = target_exts;
    m_browser_mode = browser_mode;
    m_browser_callback = browser_callback;
    m_cur_path = current_path;
    m_path_synced = false;
    m_last_special_page = m_special_page;
    m_special_page = SPECIAL_PAGE_BROWSER;
    m_special_subpage = 0;
}

void EditorScreen::FileBrowserSuccess()
{
    if (m_file_target) *m_file_target = m_cur_file;
    if (m_browser_callback == BROWSER_CALLBACK_OPEN_LEVEL)
    {
        EnsureLevel();
        OpenLevel(FullFileName);
        ResetCursor();
    }
    else if (m_browser_callback == BROWSER_CALLBACK_SAVE_LEVEL)
    {
        SaveLevel(FullFileName);
        // this will resync custom assets
        OpenLevel(FullFileName);
    }
    else if (m_browser_callback == BROWSER_CALLBACK_NEW_LEVEL)
    {
        EnsureLevel();
        ClearLevel();
        SaveLevel(FullFileName);
        // this will resync custom assets
        OpenLevel(FullFileName);
        ResetCursor();
    }
    else if (m_browser_callback == BROWSER_CALLBACK_OPEN_WORLD)
    {
        EnsureWorld();
        OpenWorld(FullFileName);
        ResetCursor();
    }
    else if (m_browser_callback == BROWSER_CALLBACK_SAVE_WORLD)
    {
        SaveWorld(FullFileName);
        // resync custom assets
        OpenWorld(FullFileName);
    }
    else if (m_browser_callback == BROWSER_CALLBACK_NEW_WORLD)
    {
        EnsureWorld();
        ClearWorld();
        SaveWorld(FullFileName);
        // resync custom assets
        OpenWorld(FullFileName);
        ResetCursor();
    }
    else if (m_browser_callback == BROWSER_CALLBACK_CUSTOM_MUSIC)
    {
        StartMusic(curSection);
    }
    FileBrowserCleanup();
}

void EditorScreen::FileBrowserFailure()
{
    // if (m_file_target && m_file_target != &FullFileName) m_file_target->clear();
    FileBrowserCleanup();
}

void EditorScreen::FileBrowserCleanup()
{
    m_cur_file.clear();
    m_file_target = nullptr;
    m_special_page = m_last_special_page;
    m_last_special_page = SPECIAL_PAGE_NONE;
    m_special_subpage = 0;
    m_browser_mode = BROWSER_MODE_NONE;
    m_browser_callback = BROWSER_CALLBACK_NONE;
}

void EditorScreen::SyncPath()
{
    if (m_path_synced) return;
    m_dirman.setPath(m_root_path+m_cur_path);
    m_dirman.getListOfFiles(m_cur_path_files, m_target_exts);
    m_dirman.getListOfFolders(m_cur_path_dirs);
    std::sort(m_cur_path_files.begin(), m_cur_path_files.end());
    std::sort(m_cur_path_dirs.begin(), m_cur_path_dirs.end());
    m_path_synced = true;
}

void EditorScreen::GoToSuper()
{
    if (m_cur_path.empty())
    {
        m_cur_path += "../";
    }
    else
    {
        size_t last_slash = m_cur_path.rfind('/', m_cur_path.length()-2);
        size_t last_chunk_start;
        if (last_slash == std::string::npos)
            last_chunk_start = 0;
        else
            last_chunk_start = last_slash + 1;
        if (m_cur_path.substr(last_chunk_start) == "../")
            m_cur_path += "../";
        else
            m_cur_path = m_cur_path.substr(0, last_chunk_start);
    }
    m_path_synced = false;
}

void EditorScreen::ValidateExt(std::string& cur_file)
{
    if (m_target_exts.empty()) return;

    for (const std::string& ext : m_target_exts)
    {
        if (cur_file.size() >= ext.size())
        {
            std::string possible_ext;
            possible_ext.reserve(ext.size());
            for(const char &c : cur_file.substr(cur_file.size() - ext.size()))
                possible_ext.push_back(std::tolower(c));

            if (possible_ext.compare(0, ext.size(), ext) == 0)
                return;
        }
    }

    cur_file += m_target_exts[0];
}

bool EditorScreen::FileExists(const std::string& cur_file)
{
    return (std::find(m_cur_path_files.begin(), m_cur_path_files.end(), cur_file) != m_cur_path_files.end());
}

void EditorScreen::UpdateBrowserScreen()
{
    constexpr bool IGNORE_DIRS = true;
    // render shared GUI elements on right
    if (UpdateButton(e_ScreenW - 40 + 4, 40 + 4, GFX.NPC_modes, false, 0, 32*8, 32, 32))
    {
        FileBrowserFailure();
        return;
    }

    if (m_special_page == SPECIAL_PAGE_BROWSER_CONFIRM)
    {
        SuperPrint("CONFIRM", 3, 60, 40);
        SuperPrint("OVERWRITE " + m_cur_file + "?", 3, 10, 60);
        SuperPrint("YES", 3, 60, 110);
        if (UpdateButton(10 + 4, 100 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
        {
            FileBrowserSuccess();
            return;
        }
        SuperPrint("NO", 3, 60, 150);
        if (UpdateButton(10 + 4, 140 + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
        {
            m_special_page = SPECIAL_PAGE_BROWSER;
            m_cur_file.clear();
        }
        return;
    }

    if (!m_path_synced)
        SyncPath();

    int dir_length = m_cur_path_dirs.size() + 1; // ".."
    int file_length = m_cur_path_files.size();
    if (m_browser_mode == BROWSER_MODE_SAVE || m_browser_mode == BROWSER_MODE_SAVE_NEW)
    {
        if (m_browser_mode == BROWSER_MODE_SAVE_NEW)
            SuperPrint("NEW FILE", 3, 60, 40);
        else
            SuperPrint("SAVE FILE", 3, 60, 40);
        dir_length ++; // "new", last folder
        file_length ++; // "new", first file
    }
    else
    {
        SuperPrint("OPEN FILE", 3, 60, 40);
    }
    // ignore directories
    if (IGNORE_DIRS)
        dir_length = 0;
    int page_max = (dir_length + file_length - 1) / 20;
    if (!(page_max == 0 && m_special_subpage == 0))
        SuperPrint("PAGE " + std::to_string(m_special_subpage+1) + " OF " + std::to_string(page_max+1), 3, e_ScreenW - 320, 40);
    if (m_special_subpage > 0 && UpdateButton(e_ScreenW - 120 + 4, 40 + 4, GFX.NPC_modes, false, 0, 32*1, 32, 32))
        m_special_subpage --;
    if (m_special_subpage < page_max && UpdateButton(e_ScreenW - 80 + 4, 40 + 4, GFX.NPC_modes, false, 0, 32*2, 32, 32))
        m_special_subpage ++;

    if (!m_cur_path.empty())
        SuperPrint("IN " + m_cur_path, 3, 10, 60);
    // render file selector
    for (int i = 0; i < 20; i++)
    {
        int x = 10 + (e_ScreenW/2)*(i/10);
        int y = 80 + 40*(i%10);

        int l = m_special_subpage*20 + i;
        if (!IGNORE_DIRS && l == 0)
        {
            SuperPrint("..", 3, x + 44, y + 12);
            if (UpdateButton(x + 4, y + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
            {
                GoToSuper();
                m_special_subpage = 0;
            }
        }
        else if ((m_browser_mode == BROWSER_MODE_SAVE || m_browser_mode == BROWSER_MODE_SAVE_NEW) && l == dir_length - 1)
        {
            SuperPrint("<NEW FOLDER>", 3, x + 44, y + 12);
            if (UpdateButton(x + 4, y + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
            {
                std::string folder_name = GetTextInput("New folder name", "");
                if (!folder_name.empty() && !m_dirman.exists(folder_name))
                {
                    m_dirman.mkdir(folder_name);
                    m_path_synced = false;
                }
            }
        }
        else if (l < dir_length)
        {
            l -= 1;
            if (m_cur_path_dirs[l].length() < 15)
                SuperPrint(m_cur_path_dirs[l], 3, x + 44, y + 10);
            else
            {
                SuperPrint(m_cur_path_dirs[l].substr(0, 14), 3, x + 44, y + 2);
                SuperPrint(m_cur_path_dirs[l].substr(14, 14), 3, x + 44, y + 20);
            }
            if (UpdateButton(x + 4, y + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
            {
                m_cur_path += m_cur_path_dirs[l] + "/";
                m_path_synced = false;
                m_special_subpage = 0;
            }
        }
        else if ((m_browser_mode == BROWSER_MODE_SAVE || m_browser_mode == BROWSER_MODE_SAVE_NEW) && l == dir_length)
        {
            SuperPrint("<NEW FILE>", 3, x + 44, y + 12);
            if (UpdateButton(x + 4, y + 4, GFX.ECursor[1], false, 0, 0, 32, 32))
            {
                std::string file_name = GetTextInput("Save as", "");
                if (!file_name.empty())
                {
                    // validate: append the file extension if it doesn't already appear.
                    ValidateExt(file_name);
                    m_cur_file = m_cur_path + file_name;
                    if (FileExists(file_name))
                        m_special_page = SPECIAL_PAGE_BROWSER_CONFIRM;
                    else
                    {
                        FileBrowserSuccess();
                        return;
                    }
                }
            }
        }
        else if (l < dir_length + file_length)
        {
            if (m_browser_mode == BROWSER_MODE_SAVE || m_browser_mode == BROWSER_MODE_SAVE_NEW)
                l -= dir_length + 1;
            else
                l -= dir_length;
            if (m_cur_path_files[l].length() < 15)
                SuperPrint(m_cur_path_files[l], 3, x + 44, y + 10);
            else
            {
                SuperPrint(m_cur_path_files[l].substr(0, 14), 3, x + 44, y + 2);
                SuperPrint(m_cur_path_files[l].substr(14, 14), 3, x + 44, y + 20);
            }
            if (UpdateButton(x + 4, y + 4, GFX.ECursor[2], false, 0, 0, 32, 32))
            {
                m_cur_file = m_cur_path + m_cur_path_files[l];
                if (m_browser_mode == BROWSER_MODE_OPEN)
                {
                    FileBrowserSuccess();
                    return;
                }
                else
                {
                    // overwrite confirmation
                    m_special_page = SPECIAL_PAGE_BROWSER_CONFIRM;
                }
            }
        }
    }
}

inline void swap_screens()
{
    editorScreen.active = !editorScreen.active;
    HasCursor = false;
    MouseRelease = false;
    MenuMouseRelease = false;
}

void EditorScreen::UpdateSelectorBar(bool level_screen)
{
    // limited and temporary editor screen functionality for the level screen
    if (level_screen && active)
        return;

    int sx;
    if (level_screen)
    {
        sx = 100;
    }
    else
        sx = 0;
    frmMain.renderRect(sx+0, 0, e_ScreenW, 40, 0.8f, 0.8f, 0.8f, 1.0f, true);
    bool in_layers = (m_special_page == SPECIAL_PAGE_LAYERS || m_special_page == SPECIAL_PAGE_LAYER_DELETION);
    bool in_events = (m_special_page == SPECIAL_PAGE_EVENTS || m_special_page == SPECIAL_PAGE_EVENT_LAYERS
        || m_special_page == SPECIAL_PAGE_EVENT_TRIGGER || m_special_page == SPECIAL_PAGE_EVENT_SETTINGS
        || m_special_page == SPECIAL_PAGE_EVENT_BOUNDS || m_special_page == SPECIAL_PAGE_EVENT_DELETION
        || m_special_page == SPECIAL_PAGE_EVENT_MUSIC || m_special_page == SPECIAL_PAGE_EVENT_BACKGROUND
        || m_special_page == SPECIAL_PAGE_EVENT_CONTROLS || m_special_page == SPECIAL_PAGE_EVENT_SOUND);
    bool in_file = (m_special_page == SPECIAL_PAGE_FILE || m_special_page == SPECIAL_PAGE_FILE_CONFIRM);
    bool in_world_settings = (m_special_page == SPECIAL_PAGE_WORLD_SETTINGS);
    bool in_excl_special = in_layers || in_events || in_world_settings || in_file;
    bool exit_special = false;

    bool currently_in;
    currently_in = !in_excl_special && EditorCursor.Mode == OptCursor_t::LVL_SELECT;
    if (UpdateButton(sx+0*40+4, 4, GFX.ECursor[2], currently_in, 0, 0, 32, 32))
    {
        if (editorScreen.active)
            swap_screens();
        EditorCursor.Mode = OptCursor_t::LVL_SELECT;
        if (in_excl_special)
            exit_special = true;
    }
    currently_in = !in_excl_special && EditorCursor.Mode == OptCursor_t::LVL_ERASER0;
    if (UpdateButton(sx+1*40+4, 4, GFX.ECursor[3], currently_in, 0, 0, 22, 30))
    {
        if (editorScreen.active)
            swap_screens();
        EditorCursor.Mode = OptCursor_t::LVL_ERASER0;
        if (in_excl_special)
            exit_special = true;
    }

    // level editor tabs
    if (!WorldEditor)
    {
        currently_in = !in_excl_special && EditorCursor.Mode == OptCursor_t::LVL_BLOCKS;
        if (UpdateButton(sx+3*40+4, 4, GFXBlock[1], currently_in, 0, 0, 32, 32))
        {
            if (currently_in)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::LVL_BLOCKS;
            if (in_excl_special)
                exit_special = true;
        }
        currently_in = !in_excl_special && EditorCursor.Mode == OptCursor_t::LVL_BGOS;
        if (UpdateButton(sx+4*40+4, 4, GFXBackgroundBMP[1], currently_in, 0, 0, 32, 32))
        {
            if (currently_in)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::LVL_BGOS;
            if (in_excl_special)
                exit_special = true;
        }
        currently_in = !in_excl_special && EditorCursor.Mode == OptCursor_t::LVL_NPCS;
        if (UpdateButton(sx+5*40+4, 4, GFXNPC[1], currently_in, 0, 0, 32, 32))
        {
            if (currently_in)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::LVL_NPCS;
            if (in_excl_special)
                exit_special = true;
        }
        currently_in = !in_excl_special && EditorCursor.Mode == OptCursor_t::LVL_WATER;
        if (UpdateButton(sx+6*40+4, 4, GFXBackgroundBMP[26], currently_in, 0, 0, 32, 32))
        {
            if (currently_in)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::LVL_WATER;
            if (in_excl_special)
                exit_special = true;
        }
        currently_in = !in_excl_special && EditorCursor.Mode == OptCursor_t::LVL_WARPS;
        if (UpdateButton(sx+7*40+4, 4, GFXBlock[294], currently_in, 0, 0, 32, 32))
        {
            if (currently_in)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::LVL_WARPS;
            if (in_excl_special)
                exit_special = true;
        }
        currently_in = !in_excl_special && EditorCursor.Mode == OptCursor_t::LVL_SETTINGS;
        if (UpdateButton(sx+9*40+4, 4, GFXBlock[60], currently_in, 0, 0, 32, 32))
        {
            if (currently_in)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::LVL_SETTINGS;
            if (in_excl_special)
                exit_special = true;
        }
        if (UpdateButton(sx+10*40+4, 4, GFXBlock[447], in_layers, 0, 0, 32, 32))
        {
            if (!editorScreen.active)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::LVL_SELECT;
            optCursor.current = OptCursor_t::LVL_SELECT;
            m_last_mode = OptCursor_t::LVL_SELECT;
            if (!in_layers)
                m_special_page = SPECIAL_PAGE_LAYERS;
        }
        if (UpdateButton(sx+11*40+4, 4, GFXBlock[169], in_events, 0, 0, 32, 32))
        {
            if (!editorScreen.active)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::LVL_SELECT;
            optCursor.current = OptCursor_t::LVL_SELECT;
            m_last_mode = OptCursor_t::LVL_SELECT;
            if (!in_events)
                m_special_page = SPECIAL_PAGE_EVENTS;
        }
    }

    // world editor tabs
    if (WorldEditor)
    {
        currently_in = !in_excl_special && EditorCursor.Mode == OptCursor_t::WLD_TILES;
        if (UpdateButton(sx+3*40+4, 4, GFXTileBMP[1], currently_in, 0, 0, 32, 32))
        {
            if (currently_in)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::WLD_TILES;
            if (in_excl_special)
                exit_special = true;
        }
        currently_in = !in_excl_special && EditorCursor.Mode == OptCursor_t::WLD_SCENES;
        if (UpdateButton(sx+4*40+4, 4, GFXSceneBMP[1], currently_in, 0, 0, 32, 32))
        {
            if (currently_in)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::WLD_SCENES;
            if (in_excl_special)
                exit_special = true;
        }
        currently_in = !in_excl_special && EditorCursor.Mode == OptCursor_t::WLD_LEVELS;
        if (UpdateButton(sx+5*40+4, 4, GFXLevelBMP[2], currently_in, 0, 0, 32, 32))
        {
            if (currently_in)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::WLD_LEVELS;
            if (in_excl_special)
                exit_special = true;
        }
        currently_in = !in_excl_special && EditorCursor.Mode == OptCursor_t::WLD_PATHS;
        if (UpdateButton(sx+6*40+4, 4, GFXPathBMP[4], currently_in, 0, 0, 32, 32))
        {
            if (currently_in)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::WLD_PATHS;
            if (in_excl_special)
                exit_special = true;
        }
        currently_in = !in_excl_special && EditorCursor.Mode == OptCursor_t::WLD_MUSIC;
        if (UpdateButton(sx+7*40+4, 4, GFX.NPC_modes, currently_in, 0, 32*26, 32, 32))
        {
            if (currently_in)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::WLD_MUSIC;
            if (in_excl_special)
                exit_special = true;
        }
        if (UpdateButton(sx+9*40+4, 4, GFXLevelBMP[15], in_world_settings, 0, 0, 32, 32))
        {
            if (!editorScreen.active)
                swap_screens();
            EditorCursor.Mode = OptCursor_t::LVL_SELECT;
            optCursor.current = OptCursor_t::LVL_SELECT;
            m_last_mode = OptCursor_t::LVL_SELECT;
            if (!in_world_settings)
                m_special_page = SPECIAL_PAGE_WORLD_SETTINGS;
        }
    }

    if (UpdateButton(sx+13*40+4, 4, GFX.NPC_modes, in_file, 0, 32*25, 32, 32))
    {
        if (!editorScreen.active)
            swap_screens();
        EditorCursor.Mode = OptCursor_t::LVL_SELECT;
        optCursor.current = OptCursor_t::LVL_SELECT;
        m_last_mode = OptCursor_t::LVL_SELECT;
        if (!in_file)
            m_special_page = SPECIAL_PAGE_FILE;
    }

    if (!WorldEditor && UpdateButton(sx+14*40 + 4, 4, GFX.NPC_modes, false, 0, 32*21, 32, 32))
    {
        // turn this into a routine...?!
        Backup_FullFileName = FullFileName;
        // how does this interact with cross-level warps?
        FullFileName = FullFileName + "tst";
        SaveLevel(FullFileName);
        HasCursor = false;
        zTestLevel();
    }

    if (UpdateButton(sx+15*40 + 4, 4, GFX.NPC_modes, false, 0, 32*31, 32, 32))
        swap_screens();

    if (level_screen)
    {
        MenuMouseRelease = false;
        MenuMouseDown = false;
    }

    // if mode has been updated for any reason, close any special dialogue boxes
    // and sync everything up
    if (exit_special || EditorCursor.Mode != m_last_mode)
    {
        if (m_browser_mode != BROWSER_MODE_NONE)
            FileBrowserCleanup();
        m_special_page = SPECIAL_PAGE_NONE;
        m_special_subpage = 0;
        optCursor.current = EditorCursor.Mode;
        m_last_mode = EditorCursor.Mode;
    }
}

void EditorScreen::UpdateEditorScreen()
{
    MessageText.clear();
#ifdef __3DS__
    frmMain.initDraw(1);
#else
    frmMain.setViewport(ScreenW/2-e_ScreenW/2, 0, e_ScreenW, e_ScreenH);
#endif
    frmMain.renderRect(0, 0, e_ScreenW, e_ScreenH, 0.4f, 0.4f, 0.8f, 1.0f, true);
    UpdateSelectorBar();
    if (m_special_page == SPECIAL_PAGE_BROWSER || m_special_page == SPECIAL_PAGE_BROWSER_CONFIRM)
        UpdateBrowserScreen();
    else if (m_special_page == SPECIAL_PAGE_FILE || m_special_page == SPECIAL_PAGE_FILE_CONFIRM)
        UpdateFileScreen();
    else if (m_special_page == SPECIAL_PAGE_LAYERS || m_special_page == SPECIAL_PAGE_LAYER_DELETION
        || m_special_page == SPECIAL_PAGE_OBJ_LAYER || m_special_page == SPECIAL_PAGE_EVENT_LAYERS)
        UpdateLayersScreen();
    else if (m_special_page == SPECIAL_PAGE_EVENTS || m_special_page == SPECIAL_PAGE_EVENT_DELETION)
        UpdateEventsScreen();
    else if (m_special_page == SPECIAL_PAGE_EVENT_SETTINGS || m_special_page == SPECIAL_PAGE_EVENT_BOUNDS || m_special_page == SPECIAL_PAGE_EVENT_CONTROLS)
        UpdateEventSettingsScreen();
    else if (m_special_page == SPECIAL_PAGE_OBJ_TRIGGERS || m_special_page == SPECIAL_PAGE_EVENT_TRIGGER)
        UpdateEventsSubScreen();
    else if (m_special_page == SPECIAL_PAGE_BLOCK_CONTENTS)
        UpdateNPCScreen();
    else if (m_special_page == SPECIAL_PAGE_WORLD_SETTINGS)
        UpdateWorldSettingsScreen();
    else if (m_special_page == SPECIAL_PAGE_EVENT_MUSIC || m_special_page == SPECIAL_PAGE_EVENT_BACKGROUND
        || m_special_page == SPECIAL_PAGE_EVENT_SOUND || m_special_page == SPECIAL_PAGE_SECTION_BACKGROUND
        || m_special_page == SPECIAL_PAGE_SECTION_MUSIC || m_special_page == SPECIAL_PAGE_LEVEL_EXIT
        || EditorCursor.Mode == OptCursor_t::WLD_MUSIC)
        UpdateSelectListScreen();
    else if (EditorCursor.Mode == OptCursor_t::LVL_BLOCKS)
        UpdateBlockScreen();
    else if (EditorCursor.Mode == OptCursor_t::LVL_BGOS)
        UpdateBGOScreen();
    else if (EditorCursor.Mode == OptCursor_t::LVL_NPCS)
        UpdateNPCScreen();
    else if (EditorCursor.Mode == OptCursor_t::LVL_WATER)
        UpdateWaterScreen();
    else if (EditorCursor.Mode == OptCursor_t::LVL_WARPS)
        UpdateWarpScreen();
    else if (EditorCursor.Mode == OptCursor_t::LVL_SETTINGS)
        UpdateSectionsScreen();
    else if (EditorCursor.Mode == OptCursor_t::WLD_TILES)
        UpdateTileScreen();
    else if (EditorCursor.Mode == OptCursor_t::WLD_SCENES)
        UpdateSceneScreen();
    else if (EditorCursor.Mode == OptCursor_t::WLD_LEVELS)
        UpdateLevelScreen();
    else if (EditorCursor.Mode == OptCursor_t::WLD_PATHS)
        UpdatePathScreen();
    else if (WorldEditor)
    {
        SuperPrint("THE CURSOR IS ON TILE...", 3, 40, 40);
        SuperPrint("X: "+std::to_string(32*(int)std::floor(EditorCursor.Location.X/32)), 3, 10, 80);
        SuperPrint("Y: "+std::to_string(32*(int)std::floor(EditorCursor.Location.Y/32)), 3, 10, 100);
        SuperPrint("IN WORLD COORDINATES.", 3, 40, 140);
    }
    // frmMain.renderTexture(MenuMouseX, MenuMouseY, GFX.ECursor[2]);
    MenuMouseRelease = false;
}

EditorScreen editorScreen;
