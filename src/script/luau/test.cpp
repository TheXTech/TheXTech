/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <cstdlib>
#include <Utils/files.h>
#include <Logger/logger.h>

#include <lua.h>
#include <luacode.h>
#include <lualib.h>

#include "global_dirs.h"

#include "script/luau/test.h"

lua_State* L = nullptr;
bool working = false;

static void *l_alloc (void *ud, void *ptr, size_t osize,
                                           size_t nsize) {
  (void)ud;  (void)osize;  /* not used */
  if (nsize == 0) {
    free(ptr);
    return NULL;
  }
  else
    return realloc(ptr, nsize);
}

void load_test_script()
{
    working = false;
    if(L)
        lua_close(L);

    Files::Data d = Files::load_file(g_dirEpisode.resolveFileCaseExistsAbs("test.luau"));

    if(d.empty())
        return;

    size_t bytecodeSize = 0;
    char* bytecode = luau_compile(d.c_str(), d.size(), NULL, &bytecodeSize);
    if(!bytecode)
        return;

    L = lua_newstate(l_alloc, nullptr);
    luaopen_base(L);

    if(!L)
    {
        free(bytecode);
        return;
    }

    int result = luau_load(L, "chunk", bytecode, bytecodeSize, 0);
    if(result == 0)
    {
        if(lua_pcall(L, 0, 0, 0) == 0)
            working = true;
        else
        {
            pLogWarning("Error when loading code: %s", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }
    else
    {
        pLogWarning("Error when compiling code: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }

    luaL_sandbox(L);

    run_test_script();
    run_test_script();
}

void run_test_script()
{
    if(working)
    {
        lua_getfield(L, LUA_GLOBALSINDEX, "main");
        if(lua_pcall(L, 0, 0, 0) != 0)
        {
            pLogWarning("Error when calling main: %s", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }
}
