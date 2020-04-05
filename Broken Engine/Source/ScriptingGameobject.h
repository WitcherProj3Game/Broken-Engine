#ifndef __SCRIPTINGGAMEOBJECT_H__
#define __SCRIPTINGGAMEOBJECT_H__

#include "BrokenCore.h"
class lua_State;

namespace luabridge
{
	class LuaRef;
}

BE_BEGIN_NAMESPACE
class BROKEN_API ScriptingGameobject {
public:
	ScriptingGameobject();
	~ScriptingGameobject();

public:

	// GameObject-related functions
	uint FindGameObject(const char* go_name);
	uint GetMyUID();
	uint GetScriptGOParent();
	uint GetGOParentFromUID(uint gameobject_UUID);
	uint GetComponentFromGO(const char* component_name, uint gameobject_UUID);
	void SetActiveGameObject(bool active, uint gameobject_UUID);
	void DestroyGOFromScript(uint gameobject_UUID);

	int GetMyLayer();
	int GetLayerByID(uint UID);

	// Current Camera
	int GetPosInFrustum(float x, float y, float z, float fovratio1, float fovratio2);
	luabridge::LuaRef GetFrustumPlanesIntersection(float x, float y, float z, float fovratio, lua_State* L);

	luabridge::LuaRef WorldToScreen(float x, float y, float z, lua_State* L);
	luabridge::LuaRef ScreenToWorld(float x, float y, float distance, lua_State* L);

	// Get a script file
	luabridge::LuaRef GetScript(uint gameobject_UUID, lua_State* L);
};
BE_END_NAMESPACE
#endif // __SCRIPTINGGAMEOBJECT_H__
