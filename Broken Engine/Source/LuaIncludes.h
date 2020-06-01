#ifndef __LUAINCLUDES_H__
#define __LUAINCLUDES_H__

extern "C"
{
#include "lua-535/include/lua.h"
#include "lua-535/include/lauxlib.h"
#include "lua-535/include/lualib.h"
}

//LUA lib
#ifndef _WIN64
#   pragma comment(lib, "lua-535/libx86/lua53.lib")
#else
#   pragma comment(lib, "lua-535/libx64/lua53.lib")
#endif

//LUABRIDGE MUST BE INCLUDED AFTER LUA
#include "LuaBridge-241/include/luabridge.h"
#include "LuaBridge-241/include/Vector.h"

#endif // __LUAINCLUDES_H__

