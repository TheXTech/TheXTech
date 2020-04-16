// C++ wrapper for LuaJIT header files.

extern "C" {
#ifdef USE_LUA_JIT
#include "lua_jit.h"
#include "lualib_jit.h"
#include "lauxlib_jit.h"
#include "luajit.h"
#else
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#endif
}

