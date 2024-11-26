/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <nds.h>
#include <fat.h>
#include <stdio.h>
#include <calico/nds/env.h>

extern int _entry_point(int argc, char** argv);

extern void* __main_bss_end;
extern void* __heap_start_ntr;
extern void* __heap_start_twl;
extern void *fake_heap_start, *fake_heap_end;

int main(int argc, char** argv)
{
    int fail_code = 0;
    if(!fatInitDefault())
    {
        fail_code = 1;
        goto fail_exit;
    }
    if(!ovlInit())
    {
        fail_code = 2;
        goto fail_exit;
    }
    if(!ovlLoadAndActivate(0))
    {
        fail_code = 3;
        goto fail_exit;
    }

    int ret = _entry_point(argc, argv);

    ovlDeactivate(0);

    return ret;

fail_exit:
    consoleDemoInit();
    iprintf("DSi bootstrap failed. Press start to exit.\n");
    iprintf("OVL SIZE %x\n", (unsigned)g_envAppNdsHeader->arm9_ovl_size);
    iprintf("BSS END %p, HEAP START %p %p\n", __main_bss_end, __heap_start_ntr, __heap_start_twl);
    iprintf("HEAP START %p END %p\n", fake_heap_start, fake_heap_end);

    if(fail_code == 1)
        iprintf("Couldn't init FAT.\n");
    if(fail_code == 2)
        iprintf("Couldn't init OVL.\n");
    if(fail_code == 3)
        iprintf("Couldn't load OVL.\n");

    while (pmMainLoop()) {
        threadWaitForVBlank();
        scanKeys();

        unsigned pressed = keysDown();
        if (pressed & KEY_START) {
            break;
        }
    }

    return 0;
}
